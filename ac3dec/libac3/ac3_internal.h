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

void bit_allocate (ac3_state_t * state, ac3_ba_t * ba, int bndstart,
		   int start, int end, int fastleak, int slowleak,
		   uint8_t * exp, int8_t * bap);

int downmix_init (int input, int flags, sample_t * level,
		  sample_t clev, sample_t slev);
void downmix (sample_t * samples, int acmod, int output,
	      sample_t level, sample_t bias, sample_t clev, sample_t slev);

void imdct_init (uint32_t mm_accel);
extern void (* imdct_256) (sample_t * data, sample_t * delay);
extern void (* imdct_512) (sample_t * data, sample_t * delay);
void imdct_do_256_mlib (sample_t * data, sample_t * delay);
void imdct_do_512_mlib (sample_t * data, sample_t * delay);
