/* 
 *    convert.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "decode.h"
#include "uncouple.h"

static
void convert_to_float(uint_16 exp, uint_16 mant, uint_32 *dest);

void
uncouple(bsi_t *bsi,audblk_t *audblk,stream_coeffs_t *coeffs)
{
	int i,j;

	for(i=0; i< bsi->nfchans; i++)
	{
		for(j=0; j < audblk->endmant[i]; j++)
			 convert_to_float(audblk->fbw_exp[i][j],audblk->chmant[i][j],
					 (uint_32*) &coeffs->fbw[i][j]);
	}

	if(audblk->cplinu)
	{
		uint_16 cplstrtmant;

		cplstrtmant = audblk->cplstrtmant;

		for(i=0; i< bsi->nfchans; i++)
		{
			if(audblk->chincpl[i])
			{

				//FIXME do the conversion once in a local buffer
				//and then memcpy
				/* ncplmant is equal to 12 * ncplsubnd */
				for(j=0; j < 12 * audblk->ncplsubnd; j++)
					 convert_to_float(audblk->cpl_exp[j],audblk->cplmant[j],
							(uint_32*) &coeffs->fbw[i][j + cplstrtmant]);
			}
		}

	}


	if(bsi->lfeon)
	{
		/* There are always 7 mantissas for lfe */
		for(j=0; j < 7 ; j++)
			 convert_to_float(audblk->lfe_exp[j],audblk->lfemant[j],
					 (uint_32*) &coeffs->lfe[j]);

	}

}

/* Converts an unsigned exponent in the range of 0-24 and a 16 bit mantissa
 * to an IEEE single precision floating point value */
static
void convert_to_float(uint_16 exp, uint_16 mant, uint_32 *dest)
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
