/* 
 *    unpack.c
 *
 *	Copyright (C) Aaron Holtzman - May 1999
 *
 *  This file is part of ac3dec, a free Dolby AC-3 stream decoder.
 *	
 *  ac3dec is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  ac3dec is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */


#include <stdlib.h>
#include "ac3.h"
#include "decode.h"
#include "unpack.h"

#define UNPACK_FBW  1
#define UNPACK_CPL  1
#define UNPACK_LFE  1

static void unpack_exp(uint_16 type,uint_16 expstr,uint_16 ngrps,uint_16 initial_exp, 
		uint_16 exps[], sint_32 *dest);

void
unpack_exponents( bsi_t *bsi, audblk_t *audblk, stream_coeffs_t *coeffs)
{
	uint_16 i;

	for(i=0; i< bsi->nfchans; i++)
		unpack_exp(UNPACK_FBW, audblk->chexpstr[i], audblk->nchgrps[i], audblk->exps[i][0], 
				&audblk->exps[i][1], (uint_32*)coeffs->fbw[i]);

	if(audblk->cplinu)
		unpack_exp(UNPACK_CPL, audblk->cplexpstr,audblk->ncplgrps, audblk->cplabsexp,	
				audblk->cplexps, (uint_32*)coeffs->cpl);

	if(bsi->lfeon)
		unpack_exp(UNPACK_LFE, audblk->lfeexpstr, 2, audblk->lfeexps[0], 
				&audblk->lfeexps[1], (uint_32*)coeffs->lfe);
}


static void
unpack_exp(uint_16 type,uint_16 expstr,uint_16 ngrps,uint_16 initial_exp, 
		uint_16 exps[], sint_32 *dest)
{
	uint_16 i,j;
	sint_16 exp_acc;
	sint_16 exp_1,exp_2,exp_3;

	if(expstr == EXP_REUSE)
		return;

	/* Handle the initial absolute exponent */
	exp_acc = initial_exp;
	j = 0;

	/* In the case of a fbw channel then the initial absolute values is 
	 * also an exponent */
	if(type != UNPACK_CPL)
	{
		switch(expstr)
		{
			case EXP_D45:
				dest[j++] = exp_acc;
				dest[j++] = exp_acc;
			case EXP_D25:
				dest[j++] = exp_acc;
			case EXP_D15:
				dest[j++] = exp_acc;
		}
	}

	/* Loop through the groups and fill the dest array appropriately */
	for(i=0; i< ngrps; i++)
	{
		exp_1 = exps[i] / 25;
		exp_2 = (exps[i] - (exp_1 * 25)) / 5;
		exp_3 = exps[i] - (exp_1 * 25) - (exp_2 * 5) ;

		exp_acc += (exp_1 - 2);

		switch(expstr)
		{
			case EXP_D45:
				dest[j++] = exp_acc;
				dest[j++] = exp_acc;
			case EXP_D25:
				dest[j++] = exp_acc;
			case EXP_D15:
				dest[j++] = exp_acc;
		}

		exp_acc += (exp_2 - 2);

		switch(expstr)
		{
			case EXP_D45:
				dest[j++] = exp_acc;
				dest[j++] = exp_acc;
			case EXP_D25:
				dest[j++] = exp_acc;
			case EXP_D15:
				dest[j++] = exp_acc;
		}

		exp_acc += (exp_3 - 2);

		switch(expstr)
		{
			case EXP_D45:
				dest[j++] = exp_acc;
				dest[j++] = exp_acc;
			case EXP_D25:
				dest[j++] = exp_acc;
			case EXP_D15:
				dest[j++] = exp_acc;
		}
	}	
}

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

/* Fetch an unpacked and properly biased/dithered mantissa value */
uint_16
unpack_mantissa(bitstream_t *bs, uint_16 bap)
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
unpack_reset(void)
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

