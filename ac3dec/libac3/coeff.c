/* 
 *    coeff.c
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
#include <stdio.h>
#include "ac3.h"
#include "ac3_internal.h"


#include "decode.h"
#include "bitstream.h"
#include "dither.h"
#include "coeff.h"

#define Q0 ((-2 << 15) / 3)
#define Q1 (0)
#define Q2 ((2 << 15) / 3)

static const float q_1_0[ 32 ] = {
    Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,
    Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,
    Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,
    0,0,0,0,0
};

static const float q_1_1[ 32 ] = {
    Q0,Q0,Q0,Q1,Q1,Q1,Q2,Q2,Q2,
    Q0,Q0,Q0,Q1,Q1,Q1,Q2,Q2,Q2,
    Q0,Q0,Q0,Q1,Q1,Q1,Q2,Q2,Q2,
    0,0,0,0,0
};

static const float q_1_2[ 32 ] = {
    Q0,Q1,Q2,Q0,Q1,Q2,Q0,Q1,Q2,
    Q0,Q1,Q2,Q0,Q1,Q2,Q0,Q1,Q2,
    Q0,Q1,Q2,Q0,Q1,Q2,Q0,Q1,Q2,
    0,0,0,0,0
};

#undef Q0
#undef Q1
#undef Q2

#define Q0 ((-4 << 15) / 5)
#define Q1 ((-2 << 15) / 5)
#define Q2 (0)
#define Q3 ((2 << 15) / 5)
#define Q4 ((4 << 15) / 5)

static const float q_2_0[ 128 ] = {
    Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,Q0,
    Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,Q1,
    Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,Q2,
    Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,Q3,
    Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,Q4,
    0,0,0
};

static const float q_2_1[ 128 ] = {
    Q0,Q0,Q0,Q0,Q0,Q1,Q1,Q1,Q1,Q1,Q2,Q2,Q2,Q2,Q2,Q3,Q3,Q3,Q3,Q3,Q4,Q4,Q4,Q4,Q4,
    Q0,Q0,Q0,Q0,Q0,Q1,Q1,Q1,Q1,Q1,Q2,Q2,Q2,Q2,Q2,Q3,Q3,Q3,Q3,Q3,Q4,Q4,Q4,Q4,Q4,
    Q0,Q0,Q0,Q0,Q0,Q1,Q1,Q1,Q1,Q1,Q2,Q2,Q2,Q2,Q2,Q3,Q3,Q3,Q3,Q3,Q4,Q4,Q4,Q4,Q4,
    Q0,Q0,Q0,Q0,Q0,Q1,Q1,Q1,Q1,Q1,Q2,Q2,Q2,Q2,Q2,Q3,Q3,Q3,Q3,Q3,Q4,Q4,Q4,Q4,Q4,
    Q0,Q0,Q0,Q0,Q0,Q1,Q1,Q1,Q1,Q1,Q2,Q2,Q2,Q2,Q2,Q3,Q3,Q3,Q3,Q3,Q4,Q4,Q4,Q4,Q4,
    0,0,0
};

static const float q_2_2[ 128 ] = {
    Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,
    Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,
    Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,
    Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,
    Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,Q0,Q1,Q2,Q3,Q4,
    0,0,0
};

#undef Q0
#undef Q1
#undef Q2
#undef Q3
#undef Q4

static const float q_3[7] = {
    (-6 << 15)/7, (-4 << 15)/7, (-2 << 15)/7, 0,
    ( 2 << 15)/7, ( 4 << 15)/7, ( 6 << 15)/7
};

#define Q0 ((-10 << 15) / 11)
#define Q1 ((-8 << 15) / 11)
#define Q2 ((-6 << 15) / 11)
#define Q3 ((-4 << 15) / 11)
#define Q4 ((-2 << 15) / 11)
#define Q5 (0)
#define Q6 ((2 << 15) / 11)
#define Q7 ((4 << 15) / 11)
#define Q8 ((6 << 15) / 11)
#define Q9 ((8 << 15) / 11)
#define QA ((10 << 15) / 11)

static const float q_4_0[ 128 ] = {
    Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0,
    Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1,
    Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2,
    Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3,
    Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4,
    Q5, Q5, Q5, Q5, Q5, Q5, Q5, Q5, Q5, Q5, Q5,
    Q6, Q6, Q6, Q6, Q6, Q6, Q6, Q6, Q6, Q6, Q6,
    Q7, Q7, Q7, Q7, Q7, Q7, Q7, Q7, Q7, Q7, Q7,
    Q8, Q8, Q8, Q8, Q8, Q8, Q8, Q8, Q8, Q8, Q8,
    Q9, Q9, Q9, Q9, Q9, Q9, Q9, Q9, Q9, Q9, Q9,
    QA, QA, QA, QA, QA, QA, QA, QA, QA, QA, QA,
    0,  0,  0,  0,  0,  0,  0
};

static const float q_4_1[ 128 ] = {
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    0,  0,  0,  0,  0,  0,  0
};

#undef Q0
#undef Q1
#undef Q2
#undef Q3
#undef Q4
#undef Q5
#undef Q6
#undef Q7
#undef Q8
#undef Q9
#undef QA

static const float q_5[15] = {
    (-14 << 15)/15,(-12 << 15)/15,(-10 << 15)/15,
    ( -8 << 15)/15,( -6 << 15)/15,( -4 << 15)/15,
    ( -2 << 15)/15,   0          ,(  2 << 15)/15,
    (  4 << 15)/15,(  6 << 15)/15,(  8 << 15)/15,
    ( 10 << 15)/15,( 12 << 15)/15,( 14 << 15)/15
};

static const uint32_t u32_scale_factors[25] = 
{
	0x38000000, //2 ^ -(0 + 15)
	0x37800000, //2 ^ -(1 + 15)
	0x37000000, //2 ^ -(2 + 15)
	0x36800000, //2 ^ -(3 + 15)
	0x36000000, //2 ^ -(4 + 15)
	0x35800000, //2 ^ -(5 + 15)
	0x35000000, //2 ^ -(6 + 15)
	0x34800000, //2 ^ -(7 + 15)
	0x34000000, //2 ^ -(8 + 15)
	0x33800000, //2 ^ -(9 + 15)
	0x33000000, //2 ^ -(10 + 15)
	0x32800000, //2 ^ -(11 + 15)
	0x32000000, //2 ^ -(12 + 15)
	0x31800000, //2 ^ -(13 + 15)
	0x31000000, //2 ^ -(14 + 15)
	0x30800000, //2 ^ -(15 + 15)
	0x30000000, //2 ^ -(16 + 15)
	0x2f800000, //2 ^ -(17 + 15)
	0x2f000000, //2 ^ -(18 + 15)
	0x2e800000, //2 ^ -(19 + 15)
	0x2e000000, //2 ^ -(20 + 15)
	0x2d800000, //2 ^ -(21 + 15)
	0x2d000000, //2 ^ -(22 + 15)
	0x2c800000, //2 ^ -(23 + 15)
	0x2c000000  //2 ^ -(24 + 15)
};

float * scale_factor = (float *) u32_scale_factors;	// FIXME static

static float q_1[2];
static float q_2[2];
static float q_4;
static int q_1_pointer;
static int q_2_pointer;
static int q_4_pointer;

#define GET_COEFF(COEFF,DITHER)						\
    switch (bap[i]) {							\
    case 0:								\
	DITHER (scale_factor[exp[i]]);					\
									\
    case -1:								\
	if (q_1_pointer >= 0) {						\
	    COEFF (q_1[q_1_pointer--] * scale_factor[exp[i]]);		\
	} else {							\
	    int code;							\
									\
	    code = bitstream_get (5);					\
									\
	    q_1_pointer = 1;						\
	    q_1[0] = q_1_2[code];					\
	    q_1[1] = q_1_1[code];					\
	    COEFF (q_1_0[code] * scale_factor[exp[i]]);			\
	}								\
									\
    case -2:								\
	if (q_2_pointer >= 0) {						\
	    COEFF (q_2[q_2_pointer--] * scale_factor[exp[i]]);		\
	} else {							\
	    int code;							\
									\
	    code = bitstream_get (7);					\
									\
	    q_2_pointer = 1;						\
	    q_2[0] = q_2_2[code];					\
	    q_2[1] = q_2_1[code];					\
	    COEFF (q_2_0[code] * scale_factor[exp[i]]);			\
	}								\
									\
    case 3:								\
	COEFF (q_3[bitstream_get (3)] * scale_factor[exp[i]]);		\
									\
    case -3:								\
	if (q_4_pointer == 0) {						\
	    q_4_pointer = -1;						\
	    COEFF (q_4 * scale_factor[exp[i]]);				\
	} else {							\
	    int code;							\
									\
	    code = bitstream_get (7);					\
									\
	    q_4_pointer = 0;						\
	    q_4 = q_4_1[code];						\
	    COEFF (q_4_0[code] * scale_factor[exp[i]]);			\
	}								\
									\
    case 4:								\
	COEFF (q_5[bitstream_get (4)] * scale_factor[exp[i]]);		\
									\
    default:								\
	COEFF (((int16_t)(bitstream_get(bap[i]) << (16 - bap[i]))) *	\
	       scale_factor[exp[i]]);					\
    }

#define CHANNEL_COEFF(val)			\
    coeff[i++] = val;				\
    continue;

#define CHANNEL_DITHER(val)			\
    if (dither) {				\
	coeff[i++] = dither_gen () * val;	\
	continue;				\
    } else {					\
	coeff[i++] = 0;				\
	continue;				\
    }

static void coeff_get (float * coeff, uint8_t * exp, int8_t * bap,
		       int dither, int start, int end)
{
    int i;

    i = start;	// FIXME =0 except in coupling
    while (i < end)
	GET_COEFF (CHANNEL_COEFF, CHANNEL_DITHER);
}

#define COUPLING_COEFF(val)	\
    cplcoeff = val;		\
    break;

#define COUPLING_DITHER(val)						\
    cplcoeff = val;							\
    for (ch = 0; ch < state->nfchans; ch++)				\
	if (audblk->chincpl[ch]) {					\
	    if (audblk->dithflag[ch])					\
		samples[ch][i] =					\
		    audblk->cplco[ch][bnd] * dither_gen () * cplcoeff;	\
	    else							\
		samples[ch][i] = 0;					\
	}								\
    i++;								\
    continue;

static void coeff_get_cpl (ac3_state_t * state, audblk_t * audblk,
			   stream_samples_t samples)
{
    int i, i_end, bnd, sub_bnd, ch;
    float cplcoeff;

#define bap audblk->cpl_bap
#define exp audblk->cpl_exp

    sub_bnd = bnd = 0;
    i = audblk->cplstrtmant;
    while (i < audblk->cplendmant) {
	i_end = i + 12;
	while (audblk->cplbndstrc[sub_bnd++])
	    i_end += 12;

	while (i < i_end) {
	    GET_COEFF (COUPLING_COEFF, COUPLING_DITHER);
	    for (ch = 0; ch < state->nfchans; ch++)
		if (audblk->chincpl[ch])
		    samples[ch][i] = audblk->cplco[ch][bnd] * cplcoeff;
	    i++;
	}

	bnd++;
    }

#undef bap
#undef exp
}

void
coeff_unpack(ac3_state_t *state, audblk_t *audblk, stream_samples_t samples)
{
    uint16_t i;
    uint32_t done_cpl = 0;

    q_1_pointer = q_2_pointer = q_4_pointer = -1;

    for (i = 0; i < state->nfchans; i++) {
	coeff_get (samples[i], audblk->fbw_exp[i], audblk->fbw_bap[i],
		   audblk->dithflag[i], 0, audblk->endmant[i]);

	if (audblk->cplinu && audblk->chincpl[i] && !done_cpl) {
	    coeff_get_cpl (state, audblk, samples);
	    done_cpl = 1;
	}
    }

    if (state->lfeon)
	coeff_get (samples[5], audblk->lfe_exp, audblk->lfe_bap, 0, 0, 7);
}
