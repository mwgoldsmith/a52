/* 
 *    ac3_internal.h
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

/* Exponent strategy constants */
#define EXP_REUSE (0)
#define EXP_D15   (1)
#define EXP_D25   (2)
#define EXP_D45   (3)

/* Delta bit allocation constants */
#define DELTA_BIT_REUSE (0)
#define DELTA_BIT_NEW (1)
#define DELTA_BIT_NONE (2)
#define DELTA_BIT_RESERVED (3)

/* samples work structure */
typedef float stream_samples_t[6][256];

void bit_allocate (int fscod, int halfrate, audblk_t * audblk, ac3_ba_t * ba,
		   int bndstart, int start, int end, int fastleak,
		   int slowleak, uint8_t * exp, int8_t * bap);

int downmix (float * samples, int acmod, int output_flags,
	     float * output_level, float bias, float clev, float slev);

void imdct_init (void);
extern void (* imdct_256) (float data[], float delay[]);
extern void (* imdct_512) (float data[], float delay[]);
