/* 
 *    exponent.c
 *
 *	Aaron Holtzman - May 1999
 *
 */


#include <stdlib.h>
#include "ac3.h"
#include "decode.h"
#include "exponent.h"

static void exp_unpack_ch(uint_16 type,uint_16 expstr,uint_16 ngrps,uint_16 initial_exp, 
		uint_16 exps[], uint_16 *dest);

void
exponent_unpack( bsi_t *bsi, audblk_t *audblk, stream_coeffs_t *coeffs)
{
	uint_16 i;

	for(i=0; i< bsi->nfchans; i++)
		exp_unpack_ch(UNPACK_FBW, audblk->chexpstr[i], audblk->nchgrps[i], audblk->exps[i][0], 
				&audblk->exps[i][1], audblk->fbw_exp[i]);

	if(audblk->cplinu)
		exp_unpack_ch(UNPACK_CPL, audblk->cplexpstr,audblk->ncplgrps, audblk->cplabsexp,	
				audblk->cplexps, audblk->cpl_exp);

	if(bsi->lfeon)
		exp_unpack_ch(UNPACK_LFE, audblk->lfeexpstr, 2, audblk->lfeexps[0], 
				&audblk->lfeexps[1], audblk->lfe_exp);
}


static void
exp_unpack_ch(uint_16 type,uint_16 expstr,uint_16 ngrps,uint_16 initial_exp, 
		uint_16 exps[], uint_16 *dest)
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

