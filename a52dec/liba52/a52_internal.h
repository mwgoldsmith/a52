/*
 * a52_internal.h
 * Copyright (C) 2000-2002 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of a52dec, a free ATSC A-52 stream decoder.
 * See http://liba52.sourceforge.net/ for updates.
 *
 * a52dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * a52dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

typedef struct a52_ba_s {
    uint8_t bai;		/* fine SNR offset, fast gain */
    uint8_t deltbae;		/* delta bit allocation exists */
    int8_t deltba[50];		/* per-band delta bit allocation */
} a52_ba_t;

struct a52_state_s {
    uint8_t fscod;		/* sample rate */
    uint8_t halfrate;		/* halfrate factor */
    uint8_t acmod;		/* coded channels */
    uint8_t lfeon;		/* coded lfe channel */
    sample_t clev;		/* centre channel mix level */
    sample_t slev;		/* surround channels mix level */

    int output;			/* type of output */
    sample_t level;		/* output level */
    sample_t bias;		/* output bias */

    int dynrnge;		/* apply dynamic range */
    sample_t dynrng;		/* dynamic range */
    void * dynrngdata;		/* dynamic range callback funtion and data */
    sample_t (* dynrngcall) (sample_t range, void * dynrngdata);

    uint8_t chincpl;		/* channel coupled */
    uint8_t phsflginu;		/* phase flags in use (stereo only) */
    uint8_t cplstrtmant;	/* coupling channel start mantissa */
    uint8_t cplendmant;		/* coupling channel end mantissa */
    uint32_t cplbndstrc;	/* coupling band structure */
    sample_t cplco[5][18];	/* coupling coordinates */

    /* derived information */
    uint8_t cplstrtbnd;		/* coupling start band (for bit allocation) */
    uint8_t ncplbnd;		/* number of coupling bands */

    uint8_t rematflg;		/* stereo rematrixing */

    uint8_t endmant[5];		/* channel end mantissa */

    uint16_t bai;		/* bit allocation information */

    uint8_t csnroffst;		/* coarse SNR offset */
    a52_ba_t cplba;		/* coupling bit allocation parameters */
    a52_ba_t ba[5];		/* channel bit allocation parameters */
    a52_ba_t lfeba;		/* lfe bit allocation parameters */

    uint8_t cplfleak;		/* coupling fast leak init */
    uint8_t cplsleak;		/* coupling slow leak init */

    uint8_t cpl_exp[256];	/* decoded coupling channel exponents */
    int8_t cpl_bap[256];	/* derived coupling bit allocation */
    uint8_t fbw_exp[5][256];	/* decoded channel exponents */
    int8_t fbw_bap[5][256];	/* derived channel bit allocation */
    uint8_t lfe_exp[7];		/* decoded lfe channel exponents */
    int8_t lfe_bap[7];		/* derived lfe channel bit allocation */

    sample_t * samples;
    int downmixed;
};

#define LEVEL_PLUS6DB 2.0
#define LEVEL_PLUS3DB 1.4142135623730951
#define LEVEL_3DB 0.7071067811865476
#define LEVEL_45DB 0.5946035575013605
#define LEVEL_6DB 0.5

#define EXP_REUSE (0)
#define EXP_D15   (1)
#define EXP_D25   (2)
#define EXP_D45   (3)

#define DELTA_BIT_REUSE (0)
#define DELTA_BIT_NEW (1)
#define DELTA_BIT_NONE (2)
#define DELTA_BIT_RESERVED (3)

void a52_bit_allocate (a52_state_t * state, a52_ba_t * ba, int bndstart,
		       int start, int end, int fastleak, int slowleak,
		       uint8_t * exp, int8_t * bap);

int a52_downmix_init (int input, int flags, sample_t * level,
		      sample_t clev, sample_t slev);
int a52_downmix_coeff (sample_t * coeff, int acmod, int output, sample_t level,
		       sample_t clev, sample_t slev);
void a52_downmix (sample_t * samples, int acmod, int output, sample_t bias,
		  sample_t clev, sample_t slev);
void a52_upmix (sample_t * samples, int acmod, int output);

void a52_imdct_init (uint32_t mm_accel);
void a52_imdct_256 (sample_t * data, sample_t * delay, sample_t bias);
void a52_imdct_512 (sample_t * data, sample_t * delay, sample_t bias);
