/* 
 *    parse.c
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
 *
 */

#include <string.h>
#include "ac3.h"
#include "ac3_internal.h"

#include "bitstream.h"
#include "parse.h"
#include "bit_allocate.h"
#include "dither.h"
#include "imdct.h"
#include "downmix.h"

extern stream_samples_t samples;	// FIXME
static float delay[6][256];

static const uint8_t nfchans[8] = {2, 1, 2, 3, 3, 4, 4, 5};

int parse_syncinfo (uint8_t * buf, int * sample_rate, int * bit_rate)
{
    static int rate[] = { 32,  40,  48,  56,  64,  80,  96, 112,
			 128, 160, 192, 224, 256, 320, 384, 448,
			 512, 576, 640};
    int frmsizecod;
    int bitrate;

    if ((buf[0] != 0x0b) || (buf[1] != 0x77))	// syncword
	return 0;

    frmsizecod = buf[4] & 63;
    if (frmsizecod >= 38)
	return 0;
    *bit_rate = bitrate = rate [frmsizecod >> 1];

    switch (buf[4] & 0xc0) {
    case 0:	// 48 KHz
	*sample_rate = 48000;
	return 4 * bitrate;
    case 0x40:
	*sample_rate = 44100;
	return 2 * (320 * bitrate / 147 + (frmsizecod & 1));
    case 0x80:
	*sample_rate = 32000;
	return 6 * bitrate;
    default:
	return 0;
    }
}

int parse_bsi (ac3_state_t * state, uint8_t * buf)
{
    int chaninfo;
#define LEVEL_3DB 0.7071067811865476
#define LEVEL_45DB 0.5946035575013605
#define LEVEL_6DB 0.5
    static float clev[4] = {LEVEL_3DB, LEVEL_45DB, LEVEL_6DB, LEVEL_45DB};
    static float slev[4] = {LEVEL_3DB, LEVEL_6DB, 0, LEVEL_6DB};

    state->fscod = buf[4] >> 6;

    if (buf[5] >= 0x48)		// bsid >= 9
	return 1;

    state->acmod = buf[6] >> 5;
    state->nfchans = nfchans[state->acmod];

    bitstream_set_ptr (buf + 6);
    bitstream_get (3);	// skip acmod we already parsed

    if ((state->acmod & 0x1) && (state->acmod != 0x1))
	state->clev = clev[bitstream_get (2)];	// cmixlev

    if (state->acmod & 0x4)
	state->slev = slev[bitstream_get (2)];	// surmixlev

    if (state->acmod == 0x2)
	bitstream_get (2);	// dsurmod

    state->lfeon = bitstream_get (1);

    chaninfo = (state->acmod) ? 0 : 1;
    do {
	bitstream_get (5);	// dialnorm
	if (bitstream_get (1))	// compre
	    bitstream_get (8);	// compr
	if (bitstream_get (1))	// langcode
	    bitstream_get (8);	// langcod
	if (bitstream_get (1))	// audprodie
	    bitstream_get (7);	// mixlevel + roomtyp
    } while (chaninfo--);

    bitstream_get (2);		// copyrightb + origbs

    if (bitstream_get (1))	// timecod1e
	bitstream_get (14);	// timecod1
    if (bitstream_get (1))	// timecod2e
	bitstream_get (14);	// timecod2

    if (bitstream_get (1)) {	// addbsie
	int addbsil;

	addbsil = bitstream_get (6);
	do {
	    bitstream_get (8);	// addbsi
	} while (addbsil--);
    }

    return 0;
}

static int8_t exp_1[128] = {
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    25,25,25
};
static int8_t exp_2[128] = {
    -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    25,25,25
};
static int8_t exp_3[128] = {
    -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,
    -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,
    -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,
    -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,
    -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,
    25,25,25
};

static int parse_exponents (int expstr, int ngrps, uint8_t exponent,
			    uint8_t * dest)
{
    int exps;

    while (ngrps--) {
	exps = bitstream_get (7);

	exponent += exp_1[exps];
	if (exponent > 24)
	    return 1;

	switch (expstr) {
	case EXP_D45:
	    *(dest++) = exponent;
	    *(dest++) = exponent;
	case EXP_D25:
	    *(dest++) = exponent;
	case EXP_D15:
	    *(dest++) = exponent;
	}

	exponent += exp_2[exps];
	if (exponent > 24)
	    return 1;

	switch (expstr) {
	case EXP_D45:
	    *(dest++) = exponent;
	    *(dest++) = exponent;
	case EXP_D25:
	    *(dest++) = exponent;
	case EXP_D15:
	    *(dest++) = exponent;
	}

	exponent += exp_3[exps];
	if (exponent > 24)
	    return 1;

	switch (expstr) {
	case EXP_D45:
	    *(dest++) = exponent;
	    *(dest++) = exponent;
	case EXP_D25:
	    *(dest++) = exponent;
	case EXP_D15:
	    *(dest++) = exponent;
	}
    }	

    return 0;
}

static int parse_deltba (int8_t * deltba)
{
    int deltnseg, deltlen, delta, j;

    memset (deltba, 0, 50);

    deltnseg = bitstream_get (3);
    j = 0;
    do {
	j += bitstream_get (5);
	deltlen = bitstream_get (4);
	delta = bitstream_get (3);
	delta -= (delta >= 4) ? 3 : 4;
	if (!deltlen)
	    continue;
	if (j + deltlen >= 50)
	    return 1;
	while (deltlen--)
	    deltba[j++] = delta;
    } while (deltnseg--);

    return 0;
}

static inline int zero_snr_offsets (ac3_state_t * state, audblk_t * audblk)
{
    int i;

    if ((audblk->csnroffst) ||
	(audblk->cplinu && audblk->cplba.fsnroffst) ||
	(state->lfeon && audblk->lfeba.fsnroffst))
	return 0;
    for (i = 0; i < state->nfchans; i++)
	if (audblk->ba[i].fsnroffst)
	    return 0;
    return 1;
}

#define Q0 ((-2 << 15) / 3.0)
#define Q1 (0)
#define Q2 ((2 << 15) / 3.0)

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

#define Q0 ((-4 << 15) / 5.0)
#define Q1 ((-2 << 15) / 5.0)
#define Q2 (0)
#define Q3 ((2 << 15) / 5.0)
#define Q4 ((4 << 15) / 5.0)

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

static const float q_3[8] = {
    (-6 << 15)/7.0, (-4 << 15)/7.0, (-2 << 15)/7.0, 0,
    ( 2 << 15)/7.0, ( 4 << 15)/7.0, ( 6 << 15)/7.0, 0
};

#define Q0 ((-10 << 15) / 11.0)
#define Q1 ((-8 << 15) / 11.0)
#define Q2 ((-6 << 15) / 11.0)
#define Q3 ((-4 << 15) / 11.0)
#define Q4 ((-2 << 15) / 11.0)
#define Q5 (0)
#define Q6 ((2 << 15) / 11.0)
#define Q7 ((4 << 15) / 11.0)
#define Q8 ((6 << 15) / 11.0)
#define Q9 ((8 << 15) / 11.0)
#define QA ((10 << 15) / 11.0)

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

static const float q_5[16] = {
    (-14 << 15)/15.0,(-12 << 15)/15.0,(-10 << 15)/15.0,
    ( -8 << 15)/15.0,( -6 << 15)/15.0,( -4 << 15)/15.0,
    ( -2 << 15)/15.0,   0            ,(  2 << 15)/15.0,
    (  4 << 15)/15.0,(  6 << 15)/15.0,(  8 << 15)/15.0,
    ( 10 << 15)/15.0,( 12 << 15)/15.0,( 14 << 15)/15.0,
    0
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

static float * scale_factor = (float *) u32_scale_factors;

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
		       int dither, int end)
{
    int i;

    i = 0;
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
	    if (dithflag[ch])						\
		samples[ch][i] =					\
		    audblk->cplco[ch][bnd] * dither_gen () * cplcoeff;	\
	    else							\
		samples[ch][i] = 0;					\
	}								\
    i++;								\
    continue;

static void coeff_get_cpl (ac3_state_t * state, audblk_t * audblk,
			   uint8_t * dithflag, stream_samples_t samples)
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

int parse_audblk (ac3_state_t * state, audblk_t * audblk)
{
    static int rematrix_band[4] = {25, 37, 61, 253};
    int i, chaninfo;
    uint8_t cplexpstr, chexpstr[5], lfeexpstr, do_bit_alloc, done_cpl;
    uint8_t blksw[5], dithflag[5];

    for (i = 0; i < state->nfchans; i++)
	blksw[i] = bitstream_get (1);

    for (i = 0; i < state->nfchans; i++)
	dithflag[i] = bitstream_get (1);

    chaninfo = (state->acmod) ? 0 : 1;
    do {
	if (bitstream_get (1))	// dynrnge
	    bitstream_get (8);	// dynrng
    } while (chaninfo--);

    if (bitstream_get (1)) {	// cplstre
	audblk->cplinu = bitstream_get (1);
	if (audblk->cplinu) {
	    static int bndtab[16] = {31, 35, 37, 39, 41, 42, 43, 44,
				     45, 45, 46, 46, 47, 47, 48, 48};
	    int cplbegf;
	    int cplendf;
	    int ncplsubnd;

	    for (i = 0; i < state->nfchans; i++)
		audblk->chincpl[i] = bitstream_get (1);
	    switch (state->acmod) {
	    case 0: case 1:
		return 1;
	    case 2:
		audblk->phsflginu = bitstream_get (1);
	    }
	    cplbegf = bitstream_get (4);
	    cplendf = bitstream_get (4);

	    if (cplendf + 3 - cplbegf < 0)
		return 1;
	    audblk->ncplbnd = ncplsubnd = cplendf + 3 - cplbegf;
	    audblk->cplstrtbnd = bndtab[cplbegf];
	    audblk->cplstrtmant = cplbegf * 12 + 37;
	    audblk->cplendmant = cplendf * 12 + 73;

	    for (i = 0; i < ncplsubnd - 1; i++) {
		audblk->cplbndstrc[i] = bitstream_get (1);
		audblk->ncplbnd -= audblk->cplbndstrc[i];
	    }
	    audblk->cplbndstrc[i] = 0;	// last value is a sentinel
	}
    }

    if (audblk->cplinu) {
	int j, cplcoe;

	cplcoe = 0;
	for (i = 0; i < state->nfchans; i++)
	    if (audblk->chincpl[i])
		if (bitstream_get (1)) {	// cplcoe
		    int mstrcplco, cplcoexp, cplcomant;

		    cplcoe = 1;
		    mstrcplco = 3 * bitstream_get (2);
		    for (j = 0; j < audblk->ncplbnd; j++) {
			cplcoexp = bitstream_get (4);
			cplcomant = bitstream_get (4);
			if (cplcoexp == 15)
			    cplcomant <<= 14;
			else
			    cplcomant = (cplcomant | 0x10) << 13;
			audblk->cplco[i][j] =
			    cplcomant * scale_factor[cplcoexp + mstrcplco];
		    }
		}
	if ((state->acmod == 0x2) && audblk->phsflginu && cplcoe)
	    for (j = 0; j < audblk->ncplbnd; j++)
		if (bitstream_get (1))	// phsflg
		    audblk->cplco[1][j] = -audblk->cplco[1][j];
    }

    if ((state->acmod == 0x2) && (bitstream_get (1))) {	// rematstr
	int end;

	end = (audblk->cplinu) ? audblk->cplstrtmant : 73;
	i = 0;
	do
	    audblk->rematflg[i] = bitstream_get (1);
	while (rematrix_band[i++] < end);
    }

    cplexpstr = EXP_REUSE;
    lfeexpstr = EXP_REUSE;
    if (audblk->cplinu)
	cplexpstr = bitstream_get (2);
    for (i = 0; i < state->nfchans; i++)
	chexpstr[i] = bitstream_get (2);
    if (state->lfeon) 
	lfeexpstr = bitstream_get (1);

    for (i = 0; i < state->nfchans; i++)
	if (chexpstr[i] != EXP_REUSE) {
	    if (audblk->cplinu && audblk->chincpl[i])
		audblk->endmant[i] = audblk->cplstrtmant;
	    else {
		int chbwcod;

		chbwcod = bitstream_get (6);
		if (chbwcod > 60)
		    return 1;
		audblk->endmant[i] = chbwcod * 3 + 73;
	    }
	}

    do_bit_alloc = 0;

    if (cplexpstr != EXP_REUSE) {
	int cplabsexp, ncplgrps;

	do_bit_alloc = 1;
	ncplgrps = ((audblk->cplendmant - audblk->cplstrtmant) /
		    (3 << (cplexpstr - 1)));
	cplabsexp = bitstream_get (4) << 1;
	if (parse_exponents (cplexpstr, ncplgrps, cplabsexp,
			     audblk->cpl_exp + audblk->cplstrtmant))
	    return 1;
    }
    for (i = 0; i < state->nfchans; i++)
	if (chexpstr[i] != EXP_REUSE) {
	    int grp_size, nchgrps;

	    do_bit_alloc = 1;
	    grp_size = 3 * (1 << (chexpstr[i] - 1));
	    nchgrps = (audblk->endmant[i] - 1 + (grp_size - 3)) / grp_size;
	    audblk->fbw_exp[i][0] = bitstream_get (4);
	    if (parse_exponents (chexpstr[i], nchgrps, audblk->fbw_exp[i][0],
				 audblk->fbw_exp[i] + 1))
		return 1;
	    bitstream_get (2);	// gainrng
	}
    if (lfeexpstr != EXP_REUSE) {
	do_bit_alloc = 1;
	audblk->lfe_exp[0] = bitstream_get (4);
	if (parse_exponents (lfeexpstr, 2, audblk->lfe_exp[0],
			     audblk->lfe_exp + 1))
	    return 1;
    }

    if (bitstream_get (1)) {	// baie
	do_bit_alloc = 1;
	audblk->sdcycod = bitstream_get (2);
	audblk->fdcycod = bitstream_get (2);
	audblk->sgaincod = bitstream_get (2);
	audblk->dbpbcod = bitstream_get (2);
	audblk->floorcod = bitstream_get (3);
    }
    if (bitstream_get (1)) {	//snroffste
	do_bit_alloc = 1;
	audblk->csnroffst = bitstream_get (6);
	if (audblk->cplinu) {
	    audblk->cplba.fsnroffst = bitstream_get (4);
	    audblk->cplba.fgaincod = bitstream_get (3);
	}
	for (i = 0; i < state->nfchans; i++) {
	    audblk->ba[i].fsnroffst = bitstream_get (4);
	    audblk->ba[i].fgaincod = bitstream_get (3);
	}
	if (state->lfeon) {
	    audblk->lfeba.fsnroffst = bitstream_get (4);
	    audblk->lfeba.fgaincod = bitstream_get (3);
	}
    }
    if ((audblk->cplinu) && (bitstream_get (1))) {	// cplleake
	do_bit_alloc = 1;
	audblk->cplfleak = 2304 - (bitstream_get (3) << 8);
	audblk->cplsleak = 2304 - (bitstream_get (3) << 8);
    }

    if (bitstream_get (1)) {	// deltbaie
	do_bit_alloc = 1;
	if (audblk->cplinu)
	    audblk->cplba.deltbae = bitstream_get (2);
	for (i = 0; i < state->nfchans; i++)
	    audblk->ba[i].deltbae = bitstream_get (2);
	if (audblk->cplinu && (audblk->cplba.deltbae == DELTA_BIT_NEW) &&
	    parse_deltba (audblk->cplba.deltba))
	    return 1;
	for (i = 0; i < state->nfchans; i++)
	    if ((audblk->ba[i].deltbae == DELTA_BIT_NEW) &&
		parse_deltba (audblk->ba[i].deltba))
		return 1;
    }

    if (do_bit_alloc) {
	if (zero_snr_offsets (state, audblk)) {
	    memset (audblk->cpl_bap, 0, sizeof (audblk->cpl_bap));
	    memset (audblk->fbw_bap, 0, sizeof (audblk->fbw_bap));
	    memset (audblk->lfe_bap, 0, sizeof (audblk->lfe_bap));
	} else {
	    if (audblk->cplinu)
		bit_allocate (state->fscod, audblk, &audblk->cplba,
			      audblk->cplstrtbnd,
			      audblk->cplstrtmant, audblk->cplendmant,
			      audblk->cplfleak, audblk->cplsleak,
			      audblk->cpl_exp, audblk->cpl_bap);
	    for (i = 0; i < state->nfchans; i++)
		bit_allocate (state->fscod, audblk, audblk->ba + i, 0, 0,
			      audblk->endmant[i], 0, 0,
			      audblk->fbw_exp[i], audblk->fbw_bap[i]);
	    if (state->lfeon) {
		audblk->lfeba.deltbae = DELTA_BIT_NONE;
		bit_allocate (state->fscod, audblk, &audblk->lfeba, 0, 0, 7,
			      0, 0, audblk->lfe_exp, audblk->lfe_bap);
	    }
	}
    }

    if (bitstream_get (1)) {	// skiple
	i = bitstream_get (9);	// skipl
	while (i--)
	    bitstream_get (8);
    }

    q_1_pointer = q_2_pointer = q_4_pointer = -1;
    done_cpl = 0;

    for (i = 0; i < state->nfchans; i++) {
	int j;

	coeff_get (samples[i], audblk->fbw_exp[i], audblk->fbw_bap[i],
		   dithflag[i], audblk->endmant[i]);

	if (audblk->cplinu && audblk->chincpl[i]) {
	    if (!done_cpl) {
		coeff_get_cpl (state, audblk, dithflag, samples);
		done_cpl = 1;
	    }
	    j = audblk->cplendmant;
	} else
	    j = audblk->endmant[i];
	for (; j < 256; j++)
	    samples[i][j] = 0;
    }

    if (state->acmod == 2) {
	int j, end, band;

	end = ((audblk->endmant[0] < audblk->endmant[1]) ?
	       audblk->endmant[0] : audblk->endmant[1]);

	i = 0;
	j = 13;
	do {
	    if (!audblk->rematflg[i]) {
		j = rematrix_band[i++];
		continue;
	    }
	    band = rematrix_band[i++];
	    if (band > end)
		band = end;
	    do {
		float tmp0, tmp1;

		tmp0 = samples[0][j];
		tmp1 = samples[1][j];
		samples[0][j] = tmp0 + tmp1;
		samples[1][j] = tmp0 - tmp1;
	    } while (++j < band);
	} while (j < end);
    }

    if (state->lfeon) {
	coeff_get (samples[5], audblk->lfe_exp, audblk->lfe_bap, 0, 7);
#if 0
	for (i = 7; i < 256; i++)
	    samples[5][i] = 0;
#endif
    }

    for (i = 0; i < state->nfchans; i++)
	if (blksw[i])
            imdct_256 (samples[i], delay[i]);
        else 
            imdct_512 (samples[i], delay[i]);

#if 0
    if (state->lfeon)
	imdct_512 (samples[5], delay[5]);
#endif

    downmix (*samples, state->acmod, 2, 1, 1, state->clev, state->slev, 384);

    return 0;
}
