/* 
 *    mantissa.c
 *
 *	Aaron Holtzman - May 1999
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "decode.h"
#include "bitstream.h"
#include "mantissa.h"

//Lookup tables of 0.16 two's complement quantization values
uint_16 q_1[3] = {( -2 << 15)/3, 0           ,(  2 << 15)/3 };

uint_16 q_2[5] = {( -4 << 15)/5,( -2 << 15)/5,   0         ,
	                (  2 << 15)/5,(  4 << 15)/5};

uint_16 q_3[7] = {( -6 << 15)/7,( -4 << 15)/7,( -2 << 15)/7,
	                   0         ,(  2 << 15)/7,(  4 << 15)/7,
									(  6 << 15)/7};

uint_16 q_4[11] = {(-10 << 15)/11,(-8 << 15)/11,(-6 << 15)/11,
	                ( -4 << 15)/11,(-2 << 15)/11,  0          ,
									(  2 << 15)/11,( 4 << 15)/11,( 6 << 15)/11,
									(  8 << 15)/11,(10 << 15)/11};

uint_16 q_5[15] = {(-14 << 15)/15,(-12 << 15)/15,(-10 << 15)/15,
                   ( -8 << 15)/15,( -6 << 15)/15,( -4 << 15)/15,
									 ( -2 << 15)/15,   0          ,(  2 << 15)/15,
									 (  4 << 15)/15,(  6 << 15)/15,(  8 << 15)/15,
									 ( 10 << 15)/15,( 12 << 15)/15,( 14 << 15)/15};

//These store the persistent state of the packed mantissas
static uint_16 m_1[3];
static uint_16 m_2[3];
static uint_16 m_4[2];
static uint_16 m_1_pointer;
static uint_16 m_2_pointer;
static uint_16 m_4_pointer;

//Conversion from bap to number of bits in the mantissas
//zeros account for cases 0,1,2,4 which are special cased
static uint_16 qnttztab[16] = { 0, 0, 0, 3, 0 , 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16};

static uint_16 mantissa_get_dither(void);
static uint_16 mantissa_get(bitstream_t *bs, uint_16 bap);
static void mantissa_reset(void);

void
mantissa_unpack(bsi_t *bsi, audblk_t *audblk,bitstream_t *bs)
{
	uint_16 i,j;

	mantissa_reset();

	for(i=0; i< bsi->nfchans; i++)
	{
		for(j=0; j < audblk->endmant[i]; j++)
			audblk->chmant[i][j] = mantissa_get(bs,audblk->fbw_bap[i][j]);

		mantissa_reset();
	}

	if(audblk->cplinu)
	{
		/* ncplmant is equal to 12 * ncplsubnd */
		for(j=0; j < 12 * audblk->ncplsubnd; j++)
			audblk->cplmant[j] = mantissa_get(bs,audblk->cpl_bap[j]);

		mantissa_reset();
	}


	if(bsi->lfeon)
	{
		/* There are always 7 mantissas for lfe */
		for(j=0; j < 7 ; j++)
			audblk->lfemant[j] = mantissa_get(bs,audblk->lfe_bap[j]);

		mantissa_reset();
	}

}

/* Fetch an unpacked, left justified, and properly biased/dithered mantissa value */
static uint_16
mantissa_get(bitstream_t *bs, uint_16 bap)
{
	uint_16 result;
	uint_16 group_code;

	//If the bap is 0-5 then we have special cases to take care of
	switch(bap)
	{
		case 0:
			//FIXME change to respect the dither flag
			result = mantissa_get_dither();
			break;

		case 1:
			if(m_1_pointer > 2)
			{
				group_code = bitstream_get(bs,5);

				if(group_code > 26)
					//FIXME do proper block error handling
					printf("\n!! Invalid mantissa !!\n");

				m_1[0] = group_code / 9; 
				m_1[1] = (group_code % 9) / 3; 
				m_1[2] = (group_code % 9) % 3; 
				m_1_pointer = 0;
			}
			result = m_1[m_1_pointer++];
			result = q_1[result];
			break;
		case 2:

			if(m_2_pointer > 2)
			{
				group_code = bitstream_get(bs,7);

				if(group_code > 124)
					//FIXME do proper block error handling
					printf("\n!! Invalid mantissa !!\n");

				m_2[0] = group_code / 25;
				m_2[1] = (group_code % 25) / 5 ;
				m_2[2] = (group_code % 25) % 5 ; 
				m_2_pointer = 0;
			}
			result = m_2[m_2_pointer++];
			result = q_2[result];
			break;

		case 3:
			result = bitstream_get(bs,3);
			result = q_3[result];
			break;

		case 4:
			if(m_4_pointer > 1)
			{
				group_code = bitstream_get(bs,7);

				if(group_code > 120)
					//FIXME do proper block error handling
					printf("\n!! Invalid mantissa !!\n");

				m_4[0] = group_code / 11;
				m_4[1] = group_code % 11;
				m_4_pointer = 0;
			}
			result = m_4[m_4_pointer++];
			result = q_4[result];
			break;

		case 5:
			result = bitstream_get(bs,4);
			result = q_5[result];
			break;

		default:
			result = bitstream_get(bs,qnttztab[bap]);
			result <<= 16 - qnttztab[bap];
	}


	//if(!(result & 0x8000))
	//	printf("\n!! Invalid mantissa !!\n");

	return result;
}

static void 
mantissa_reset(void)
{
	m_1[2] = m_1[1] = m_1[0] = 0;
	m_2[2] = m_2[1] = m_2[0] = 0;
	m_4[1] = m_4[0] = 0;
	m_1_pointer = m_2_pointer = m_4_pointer = 3;
}

static uint_16 mantissa_get_dither(void)
{
	return 0;
}

/* Converts an unsigned exponent in the range of 0-24 and a 16 bit mantissa
 * to an IEEE single precision floating point value */

void mantissa_convert_to_float(uint_16 exp, uint_16 mant, uint_32 *dest)
{
	uint_16 sign;
	uint_16 exponent;
	uint_16 mantissa;
	int i;

	/* If the mantissa is zero we can simply return zero */
	if(mant == 0)
	{
		*dest = 0;
		return;
	}

	/* Take care of the one asymmetric negative number */
	if(mant == 0x8000)
		mant++;

	/* Extract the sign bit */
	sign = mant & 0x8000 ? 1 : 0;
	

	/* Invert the mantissa if it's negative */
	if(sign)
		mantissa = (~mant) + 1;
	else 
		mantissa = mant;

	/* Shift out the sign bit */
	mantissa <<= 1;

	/* Find the index of the most significant one bit */
	for(i = 0; i < 16; i++)
	{
		if((mantissa << i) & 0x8000)
			break;
	}

	/* Exponent is offset by 127 in IEEE format minus the shift to
	 * align the mantissa to 1.f */
	exponent = 0xff & (127 - exp - (i + 1));
	
	*dest = (sign << 31) | (exponent << 23) | 
		((0x007fffff) & (mantissa << (7 + i + 1)));

}
