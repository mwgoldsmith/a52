/* 
 *    mantissa.c
 *
 *	Aaron Holtzman - May 1999
 *
 */


#include <stdlib.h>
#include "ac3.h"
#include "decode.h"
#include "unpack.h"


//These store the persistent state of the packed mantissas
static uint_16 m_1[3];
static uint_16 m_2[3];
static uint_16 m_4[2];
static uint_16 m_1_pointer;
static uint_16 m_2_pointer;
static uint_16 m_4_pointer;

//Conversion from bap to number of bits in the mantissas
//zeros account for cases 0,1,2,4 which are special cased
static uint_16 qnttztab[16] = { 0, 0, 0, 3, 0 , 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16};

uint_16 mantissa_left_justify(uint_16 src,uint_16 size);
uint_16 mantissa_get_dither(void);

/* Fetch an unpacked, left justified, and properly biased/dithered mantissa value */
uint_16
mantissa_get(bitstream_t *bs, uint_16 bap)
{
	uint_16 result;
	uint_16 group_code;

	
	//If the bap is 0-5 then we have special cases to take care of
	switch(bap)
	{
		case 0:
			result = mantissa_get_dither();
			break;

		case 1:
			group_code = bitstream_get(bs,5);
			if(m_1_pointer > 2)
			{
				m_1[0] = group_code / 9; 
				m_1[1] = (group_code % 9) / 3; 
				m_1[2] = (group_code % 9) % 3; 
				m_1_pointer = 0;
			}
			result = m_1[m_1_pointer++];
			break;
		case 2:

			group_code = bitstream_get(bs,7);
			if(m_2_pointer > 2)
			{
				m_2[0] = group_code / 25;
				m_2[1] = (group_code % 25) / 5 ;
				m_2[2] = (group_code % 25) % 5 ; 
				m_2_pointer = 0;
			}
			result = m_2[m_2_pointer++];
			break;

		case 4:
			group_code = bitstream_get(bs,7);
			if(m_4_pointer > 1)
			{
				m_4[0] = group_code / 11;
				m_4[1] = group_code % 11;
				m_4_pointer = 0;
			}
			result = m_4[m_4_pointer++];
			break;

		default:
			result = bitstream_get(bs,qnttztab[bap]);
	}

	return result;
}

void 
mantissa_reset(void)
{
	m_1[2] = m_1[1] = m_1[0] = 0;
	m_2[2] = m_2[1] = m_2[0] = 0;
	m_4[1] = m_4[0] = 0;
	m_1_pointer = m_2_pointer = m_4_pointer = 3;
}

//FIXME
uint_16 mantissa_get_dither(void)
{

}

//FIXME
uint_16 mantissa_left_justify(uint_16 src,uint_16 size)
{

}

