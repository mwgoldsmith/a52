/*
 * downmix.c
 * Copyright (C) 1999-2001 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of ac3dec, a free Dolby AC-3 stream decoder.
 *
 * ac3dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ac3dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <inttypes.h>

#include "ac3.h"
#include "ac3_internal.h"

#define CONVERT(acmod,output) (((output) << 3) + (acmod))

int downmix_init (int input, int flags, sample_t * level,
		  sample_t clev, sample_t slev)
{
    static uint8_t table[11][8] = {
	{AC3_CHANNEL,	AC3_DOLBY,	AC3_STEREO,	AC3_STEREO,
	 AC3_STEREO,	AC3_STEREO,	AC3_STEREO,	AC3_STEREO},
	{AC3_MONO,	AC3_MONO,	AC3_MONO,	AC3_MONO,
	 AC3_MONO,	AC3_MONO,	AC3_MONO,	AC3_MONO},
	{AC3_CHANNEL,	AC3_DOLBY,	AC3_STEREO,	AC3_STEREO,
	 AC3_STEREO,	AC3_STEREO,	AC3_STEREO,	AC3_STEREO},
	{AC3_CHANNEL,	AC3_DOLBY,	AC3_STEREO,	AC3_3F,
	 AC3_STEREO,	AC3_3F,		AC3_STEREO,	AC3_3F},
	{AC3_CHANNEL,	AC3_DOLBY,	AC3_STEREO,	AC3_STEREO,
	 AC3_2F1R,	AC3_2F1R,	AC3_2F1R,	AC3_2F1R},
	{AC3_CHANNEL,	AC3_DOLBY,	AC3_STEREO,	AC3_STEREO,
	 AC3_2F1R,	AC3_3F1R,	AC3_2F1R,	AC3_3F1R},
	{AC3_CHANNEL,	AC3_DOLBY,	AC3_STEREO,	AC3_3F,
	 AC3_2F2R,	AC3_2F2R,	AC3_2F2R,	AC3_2F2R},
	{AC3_CHANNEL,	AC3_DOLBY,	AC3_STEREO,	AC3_3F,
	 AC3_2F2R,	AC3_3F2R,	AC3_2F2R,	AC3_3F2R},
	{AC3_CHANNEL1,	AC3_MONO,	AC3_MONO,	AC3_MONO,
	 AC3_MONO,	AC3_MONO,	AC3_MONO,	AC3_MONO},
	{AC3_CHANNEL2,	AC3_MONO,	AC3_MONO,	AC3_MONO,
	 AC3_MONO,	AC3_MONO,	AC3_MONO,	AC3_MONO},
	{AC3_CHANNEL,	AC3_DOLBY,	AC3_STEREO,	AC3_DOLBY,
	 AC3_DOLBY,	AC3_DOLBY,	AC3_DOLBY,	AC3_DOLBY}
    };
    int output;

    output = flags & AC3_CHANNEL_MASK;
    if (output > AC3_DOLBY)
	return -1;

    output = table[output][input & 7];

    if ((output == AC3_STEREO) &&
	((input == AC3_DOLBY) || ((input == AC3_3F) && (clev == LEVEL_3DB))))
	output = AC3_DOLBY;

    if (flags & AC3_ADJUST_LEVEL)
	switch (CONVERT (input & 7, output)) {

	case CONVERT (AC3_3F, AC3_MONO):
	    *level *= LEVEL_3DB / (1 + clev);
	    break;

	case CONVERT (AC3_STEREO, AC3_MONO):
	case CONVERT (AC3_2F2R, AC3_2F1R):
	case CONVERT (AC3_3F2R, AC3_3F1R):
	level_3db:
	    *level *= LEVEL_3DB;
	    break;

	case CONVERT (AC3_3F2R, AC3_2F1R):
	    if (clev < LEVEL_PLUS3DB - 1)
		goto level_3db;
	    /* break thru */
	case CONVERT (AC3_3F, AC3_STEREO):
	case CONVERT (AC3_3F1R, AC3_2F1R):
	case CONVERT (AC3_3F1R, AC3_2F2R):
	case CONVERT (AC3_3F2R, AC3_2F2R):
	    *level /= 1 + clev;
	    break;

	case CONVERT (AC3_2F1R, AC3_MONO):
	    *level *= LEVEL_PLUS3DB / (2 + slev);
	    break;

	case CONVERT (AC3_2F1R, AC3_STEREO):
	case CONVERT (AC3_3F1R, AC3_3F):
	    *level /= 1 + slev * LEVEL_3DB;
	    break;

	case CONVERT (AC3_3F1R, AC3_MONO):
	    *level *= LEVEL_3DB / (1 + clev + 0.5 * slev);
	    break;

	case CONVERT (AC3_3F1R, AC3_STEREO):
	    *level /= 1 + clev + slev * LEVEL_3DB;
	    break;

	case CONVERT (AC3_2F2R, AC3_MONO):
	    *level *= LEVEL_3DB / (1 + slev);
	    break;

	case CONVERT (AC3_2F2R, AC3_STEREO):
	case CONVERT (AC3_3F2R, AC3_3F):
	    *level /= 1 + slev;
	    break;

	case CONVERT (AC3_3F2R, AC3_MONO):
	    *level *= LEVEL_3DB / (1 + clev + slev);
	    break;

	case CONVERT (AC3_3F2R, AC3_STEREO):
	    *level /= 1 + clev + slev;
	    break;

	case CONVERT (AC3_MONO, AC3_DOLBY):
	    *level *= LEVEL_PLUS3DB;
	    break;

	case CONVERT (AC3_3F, AC3_DOLBY):
	case CONVERT (AC3_2F1R, AC3_DOLBY):
	    *level *= 1 / (1 + LEVEL_3DB);
	    break;

	case CONVERT (AC3_3F1R, AC3_DOLBY):
	case CONVERT (AC3_2F2R, AC3_DOLBY):
	    *level *= 1 / (1 + 2 * LEVEL_3DB);
	    break;

	case CONVERT (AC3_3F2R, AC3_DOLBY):
	    *level *= 1 / (1 + 3 * LEVEL_3DB);
	    break;
	}

    return output;
}

int downmix_coeff (sample_t * coeff, int acmod, int output, sample_t level,
		   sample_t clev, sample_t slev)
{
    switch (CONVERT (acmod, output & AC3_CHANNEL_MASK)) {

    case CONVERT (AC3_CHANNEL, AC3_CHANNEL):
    case CONVERT (AC3_MONO, AC3_MONO):
    case CONVERT (AC3_STEREO, AC3_STEREO):
    case CONVERT (AC3_3F, AC3_3F):
    case CONVERT (AC3_2F1R, AC3_2F1R):
    case CONVERT (AC3_3F1R, AC3_3F1R):
    case CONVERT (AC3_2F2R, AC3_2F2R):
    case CONVERT (AC3_3F2R, AC3_3F2R):
    case CONVERT (AC3_STEREO, AC3_DOLBY):
	coeff[0] = coeff[1] = coeff[2] = coeff[3] = coeff[4] = level;
	return 0;

    case CONVERT (AC3_CHANNEL, AC3_MONO):
	coeff[0] = coeff[1] = level * LEVEL_6DB;
	return 3;

    case CONVERT (AC3_STEREO, AC3_MONO):
	coeff[0] = coeff[1] = level * LEVEL_3DB;
	return 3;

    case CONVERT (AC3_3F, AC3_MONO):
	coeff[0] = coeff[2] = level * LEVEL_3DB;
	coeff[1] = level * clev * LEVEL_PLUS3DB;
	return 7;

    case CONVERT (AC3_2F1R, AC3_MONO):
	coeff[0] = coeff[1] = level * LEVEL_3DB;
	coeff[2] = level * slev * LEVEL_3DB;
	return 7;

    case CONVERT (AC3_2F2R, AC3_MONO):
	coeff[0] = coeff[1] = level * LEVEL_3DB;
	coeff[2] = coeff[3] = level * slev * LEVEL_3DB;
	return 15;

    case CONVERT (AC3_3F1R, AC3_MONO):
	coeff[0] = coeff[2] = level * LEVEL_3DB;
	coeff[1] = level * clev * LEVEL_PLUS3DB;
	coeff[3] = level * slev * LEVEL_3DB;
	return 15;

    case CONVERT (AC3_3F2R, AC3_MONO):
	coeff[0] = coeff[2] = level * LEVEL_3DB;
	coeff[1] = level * clev * LEVEL_PLUS3DB;
	coeff[3] = coeff[4] = level * slev * LEVEL_3DB;
	return 31;

    case CONVERT (AC3_MONO, AC3_DOLBY):
	coeff[0] = level * LEVEL_3DB;
	return 0;

    case CONVERT (AC3_3F, AC3_DOLBY):
	clev = LEVEL_3DB;
    case CONVERT (AC3_3F, AC3_STEREO):
    case CONVERT (AC3_3F1R, AC3_2F1R):
    case CONVERT (AC3_3F2R, AC3_2F2R):
	coeff[0] = coeff[2] = coeff[3] = coeff[4] = level;
	coeff[1] = level * clev;
	return 7;

    case CONVERT (AC3_2F1R, AC3_DOLBY):
	slev = 1;
    case CONVERT (AC3_2F1R, AC3_STEREO):
	coeff[0] = coeff[1] = level;
	coeff[2] = level * slev * LEVEL_3DB;
	return 7;

    case CONVERT (AC3_3F1R, AC3_DOLBY):
	clev = LEVEL_3DB;
	slev = 1;
    case CONVERT (AC3_3F1R, AC3_STEREO):
	coeff[0] = coeff[2] = level;
	coeff[1] = level * clev;
	coeff[3] = level * slev * LEVEL_3DB;
	return 15;

    case CONVERT (AC3_2F2R, AC3_DOLBY):
	slev = LEVEL_3DB;
    case CONVERT (AC3_2F2R, AC3_STEREO):
	coeff[0] = coeff[1] = level;
	coeff[2] = coeff[3] = level * slev;
	return 15;

    case CONVERT (AC3_3F2R, AC3_DOLBY):
	clev = LEVEL_3DB;
    case CONVERT (AC3_3F2R, AC3_2F1R):
	slev = LEVEL_3DB;
    case CONVERT (AC3_3F2R, AC3_STEREO):
	coeff[0] = coeff[2] = level;
	coeff[1] = level * clev;
	coeff[3] = coeff[4] = level * slev;
	return 31;

    case CONVERT (AC3_3F1R, AC3_3F):
	coeff[0] = coeff[1] = coeff[2] = level;
	coeff[3] = level * slev * LEVEL_3DB;
	return 13;

    case CONVERT (AC3_3F2R, AC3_3F):
	coeff[0] = coeff[1] = coeff[2] = level;
	coeff[3] = coeff[4] = level * slev;
	return 29;

    case CONVERT (AC3_2F2R, AC3_2F1R):
	coeff[0] = coeff[1] = level;
	coeff[2] = coeff[3] = level * LEVEL_3DB;
	return 12;

    case CONVERT (AC3_3F2R, AC3_3F1R):
	coeff[0] = coeff[1] = coeff[2] = level;
	coeff[3] = coeff[4] = level * LEVEL_3DB;
	return 24;

    case CONVERT (AC3_2F1R, AC3_2F2R):
	coeff[0] = coeff[1] = level;
	coeff[2] = level * LEVEL_3DB;
	return 0;

    case CONVERT (AC3_3F1R, AC3_2F2R):
	coeff[0] = coeff[2] = level;
	coeff[1] = level * clev;
	coeff[3] = level * LEVEL_3DB;
	return 7;

    case CONVERT (AC3_3F1R, AC3_3F2R):
	coeff[0] = coeff[1] = coeff[2] = level;
	coeff[3] = level * LEVEL_3DB;
	return 0;

    case CONVERT (AC3_CHANNEL, AC3_CHANNEL1):
	coeff[0] = level;
	coeff[1] = 0;
	return 0;

    case CONVERT (AC3_CHANNEL, AC3_CHANNEL2):
	coeff[0] = 0;
	coeff[1] = level;
	return 0;
    }

    return -1;	/* NOTREACHED */
}

static void mix1to1 (sample_t * samples, sample_t level, sample_t bias)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = samples[i] * level + bias;
}

static void move1to1 (sample_t * src, sample_t * dest,
		      sample_t level, sample_t bias)
{
    int i;

    for (i = 0; i < 256; i++)
	dest[i] = src[i] * level + bias;
}

static void mix2to1 (sample_t * samples, sample_t level, sample_t bias)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = (samples[i] + samples[i + 256]) * level + bias;
}

static void move2to1 (sample_t * src, sample_t * dest,
		      sample_t level, sample_t bias)
{
    int i;

    for (i = 0; i < 256; i++)
	dest[i] = (src[i] + src[i + 256]) * level + bias;
}

static void mix3to1 (sample_t * samples, sample_t level, sample_t clev,
		     sample_t bias)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = ((samples[i] + samples[i + 512]) * level +
		      samples[i + 256] * clev + bias);
}

static void mix21to1 (sample_t * samples, sample_t level, sample_t slev,
		      sample_t bias)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = ((samples[i] + samples[i + 256]) * level +
		      samples[i + 512] * slev + bias);
}

static void mix31to1 (sample_t * samples, sample_t level,
		      sample_t clev, sample_t slev, sample_t bias)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = ((samples[i] + samples[i + 512]) * level +
		      samples[i + 256] * clev + samples[i + 768] * slev +
		      bias);
}

static void mix22to1 (sample_t * samples, sample_t level, sample_t slev,
		      sample_t bias)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = ((samples[i] + samples[i + 256]) * level +
		      (samples[i + 512] + samples[i + 768]) * slev + bias);
}

static void mix32to1 (sample_t * samples, sample_t level,
		      sample_t clev, sample_t slev, sample_t bias)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = ((samples[i] + samples[i + 512]) * level +
		      samples[i + 256] * clev +
		      (samples[i + 768] + samples[i + 1024]) * slev + bias);
}

static void mix1to2 (sample_t * src, sample_t * dest,
		     sample_t level, sample_t bias)
{
    int i;

    for (i = 0; i < 256; i++)
	dest[i] = src[i] = src[i] * level + bias;
}

static void mix3to2 (sample_t * samples, sample_t level, sample_t clev,
		     sample_t bias)
{
    int i;
    sample_t common;

    for (i = 0; i < 256; i++) {
	common = samples[i + 256] * clev + bias;
	samples[i] = samples[i] * level + common;
	samples[i + 256] = samples[i + 512] * level + common;
    }
}

static void mix21to2 (sample_t * left, sample_t * right,
		      sample_t level, sample_t slev, sample_t bias)
{
    int i;
    sample_t common;

    for (i = 0; i < 256; i++) {
	common = right[i + 256] * slev + bias;
	left[i] = left[i] * level + common;
	right[i] = right[i] * level + common;
    }
}

static void mix11to1 (sample_t * front, sample_t * rear,
		      sample_t level, sample_t slev, sample_t bias)
{
    int i;

    for (i = 0; i < 256; i++)
	front[i] = front[i] * level + rear[i] * slev + bias;
}

static void mix31to2 (sample_t * samples, sample_t level,
		      sample_t clev, sample_t slev, sample_t bias)
{
    int i;
    sample_t common;

    for (i = 0; i < 256; i++) {
	common = samples[i + 256] * clev + samples[i + 768] * slev + bias;
	samples[i] = samples[i] * level + common;
	samples[i + 256] = samples[i + 512] * level + common;
    }
}

static void mix32to2 (sample_t * samples, sample_t level,
		      sample_t clev, sample_t slev, sample_t bias)
{
    int i;
    sample_t common;

    for (i = 0; i < 256; i++) {
	common = samples[i + 256] * clev + bias;
	samples[i] = samples[i] * level + common + samples[i + 768] * slev;
	samples[i + 256] = (samples[i + 512] * level + common +
			    samples[i + 1024] * slev);
    }
}

static void mix21toS (sample_t * samples,
		      sample_t level, sample_t level3db, sample_t bias)
{
    int i;
    sample_t surround;

    for (i = 0; i < 256; i++) {
	surround = samples[i + 512] * level3db;
	samples[i] = samples[i] * level - surround + bias;
	samples[i + 256] = samples[i + 256] * level + surround + bias;
    }
}

static void mix22toS (sample_t * samples,
		      sample_t level, sample_t level3db, sample_t bias)
{
    int i;
    sample_t surround;

    for (i = 0; i < 256; i++) {
	surround = (samples[i + 512] + samples[i + 768]) * level3db;
	samples[i] = samples[i] * level - surround + bias;
	samples[i + 256] = samples[i + 256] * level + surround + bias;
    }
}

static void mix31toS (sample_t * samples,
		      sample_t level, sample_t level3db, sample_t bias)
{
    int i;
    sample_t common, surround;

    for (i = 0; i < 256; i++) {
	common = samples[i + 256] * level3db + bias;
	surround = samples[i + 768] * level3db;
	samples[i] = samples[i] * level + common - surround;
	samples[i + 256] = samples[i + 512] * level + common + surround;
    }
}

static void mix32toS (sample_t * samples,
		      sample_t level, sample_t level3db, sample_t bias)
{
    int i;
    sample_t common, surround;

    for (i = 0; i < 256; i++) {
	common = samples[i + 256] * level3db + bias;
	surround = (samples[i + 768] + samples[i + 1024]) * level3db;
	samples[i] = samples[i] * level + common - surround;
	samples[i + 256] = samples[i + 512] * level + common + surround;
    }
}

static void zero (sample_t * samples)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = 0;
}

void downmix_lfe (sample_t * samples, sample_t bias)
{
    mix1to1 (samples, 1, bias);
}

void downmix (sample_t * samples, int acmod, int output, sample_t bias,
	      sample_t clev, sample_t slev)
{
    switch (CONVERT (acmod, output & AC3_CHANNEL_MASK)) {

    case CONVERT (AC3_3F2R, AC3_3F2R):
	mix1to1 (samples + 1024, 1, bias);
    case CONVERT (AC3_3F1R, AC3_3F1R):
    case CONVERT (AC3_2F2R, AC3_2F2R):
	mix1to1 (samples + 768, 1, bias);
    case CONVERT (AC3_3F, AC3_3F):
    case CONVERT (AC3_2F1R, AC3_2F1R):
    mix_3to3:
	mix1to1 (samples + 512, 1, bias);
    case CONVERT (AC3_CHANNEL, AC3_CHANNEL):
    case CONVERT (AC3_STEREO, AC3_STEREO):
    case CONVERT (AC3_STEREO, AC3_DOLBY):
    mix_2to2:
	mix1to1 (samples + 256, 1, bias);
    case CONVERT (AC3_CHANNEL, AC3_CHANNEL1):
    case CONVERT (AC3_MONO, AC3_MONO):
	mix1to1 (samples, 1, bias);
	break;

    case CONVERT (AC3_CHANNEL, AC3_CHANNEL2):
	move1to1 (samples + 256, samples, 1, bias);
	break;

    case CONVERT (AC3_STEREO, AC3_MONO):
    mix_2to1:
	mix2to1 (samples, LEVEL_3DB, bias);
	break;

    case CONVERT (AC3_2F1R, AC3_MONO):
	if (slev == 0)
	    goto mix_2to1;
	mix21to1 (samples, LEVEL_3DB, slev * LEVEL_3DB, bias);
	break;

    case CONVERT (AC3_2F2R, AC3_MONO):
	if (slev == 0)
	    goto mix_2to1;
	mix22to1 (samples, LEVEL_3DB, slev * LEVEL_3DB, bias);
	break;

    case CONVERT (AC3_3F, AC3_MONO):
    mix_3to1:
	mix3to1 (samples, LEVEL_3DB, clev * LEVEL_PLUS3DB, bias);
	break;

    case CONVERT (AC3_3F1R, AC3_MONO):
	if (slev == 0)
	    goto mix_3to1;
	mix31to1 (samples, LEVEL_3DB, clev * LEVEL_PLUS3DB, slev * LEVEL_3DB,
		  bias);
	break;

    case CONVERT (AC3_3F2R, AC3_MONO):
	if (slev == 0)
	    goto mix_3to1;
	mix32to1 (samples, LEVEL_3DB, clev * LEVEL_PLUS3DB, slev * LEVEL_3DB,
		  bias);
	break;

    case CONVERT (AC3_CHANNEL, AC3_MONO):
	mix2to1 (samples, LEVEL_6DB, bias);
	break;

    case CONVERT (AC3_MONO, AC3_DOLBY):
	mix1to2 (samples, samples + 256, LEVEL_3DB, bias);
	break;

    case CONVERT (AC3_3F, AC3_DOLBY):
	clev = LEVEL_3DB;
    case CONVERT (AC3_3F, AC3_STEREO):
    mix_3to2:
	mix3to2 (samples, 1, clev, bias);
	break;

    case CONVERT (AC3_2F1R, AC3_DOLBY):
	mix21toS (samples, 1, LEVEL_3DB, bias);
	break;

    case CONVERT (AC3_3F1R, AC3_DOLBY):
	mix31toS (samples, 1, LEVEL_3DB, bias);
	break;

    case CONVERT (AC3_2F2R, AC3_DOLBY):
	mix22toS (samples, 1, LEVEL_3DB, bias);
	break;

    case CONVERT (AC3_3F2R, AC3_DOLBY):
	mix32toS (samples, 1, LEVEL_3DB, bias);
	break;

    case CONVERT (AC3_2F1R, AC3_STEREO):
	if (slev == 0)
	    goto mix_2to2;
	mix21to2 (samples, samples + 256, 1, slev * LEVEL_3DB, bias);
	break;

    case CONVERT (AC3_3F1R, AC3_STEREO):
	if (slev == 0)
	    goto mix_3to2;
	mix31to2 (samples, 1, clev, slev * LEVEL_3DB, bias);
	break;

    case CONVERT (AC3_2F2R, AC3_STEREO):
	if (slev == 0)
	    goto mix_2to2;
	mix11to1 (samples, samples + 512, 1, slev, bias);
	mix11to1 (samples + 256, samples + 768, 1, slev, bias);
	break;

    case CONVERT (AC3_3F2R, AC3_STEREO):
	if (slev == 0)
	    goto mix_3to2;
	mix32to2 (samples, 1, clev, slev, bias);
	break;

    case CONVERT (AC3_3F1R, AC3_3F):
	if (slev == 0)
	    goto mix_3to3;
	mix21to2 (samples, samples + 512, 1, slev * LEVEL_3DB, bias);
	mix1to1 (samples + 256, 1, bias);
	break;

    case CONVERT (AC3_3F2R, AC3_3F):
	if (slev == 0)
	    goto mix_3to3;
	mix11to1 (samples, samples + 768, 1, slev, bias);
	mix11to1 (samples + 512, samples + 1024, 1, slev, bias);
	mix1to1 (samples + 256, 1, bias);
	break;

    case CONVERT (AC3_2F1R, AC3_2F2R):
	mix1to2 (samples + 512, samples + 768, LEVEL_3DB, bias);
	goto mix_2to2;

    case CONVERT (AC3_3F1R, AC3_3F2R):
	mix1to2 (samples + 768, samples + 1024, LEVEL_3DB, bias);
	goto mix_3to3;

    case CONVERT (AC3_2F2R, AC3_2F1R):
	mix2to1 (samples + 512, LEVEL_3DB, bias);
	goto mix_2to2;

    case CONVERT (AC3_3F2R, AC3_3F1R):
	mix2to1 (samples + 768, LEVEL_3DB, bias);
	goto mix_3to3;

    case CONVERT (AC3_3F1R, AC3_2F2R):
	mix3to2 (samples, 1, clev, bias);
	mix1to2 (samples + 768, samples + 512, LEVEL_3DB, bias);
	break;

    case CONVERT (AC3_3F1R, AC3_2F1R):
	mix3to2 (samples, 1, clev, bias);
	move1to1 (samples + 768, samples + 512, 1, bias);
	break;

    case CONVERT (AC3_3F2R, AC3_2F1R):
	mix3to2 (samples, 1, clev, bias);
	move2to1 (samples + 768, samples + 512, LEVEL_3DB, bias);
	break;

    case CONVERT (AC3_3F2R, AC3_2F2R):
	mix3to2 (samples, 1, clev, bias);
	move1to1 (samples + 768, samples + 512, 1, bias);
	move1to1 (samples + 1024, samples + 768, 1, bias);
	break;
    }
}

void upmix (sample_t * samples, int acmod, int output)
{
    switch (CONVERT (acmod, output & AC3_CHANNEL_MASK)) {

    case CONVERT (AC3_CHANNEL, AC3_CHANNEL1):
	zero (samples + 256);
    mix_1to1:
	mix1to1 (samples, 1, 0);
	break;

    case CONVERT (AC3_CHANNEL, AC3_CHANNEL2):
	move1to1 (samples, samples + 256, 1, 0);
	zero (samples);
	break;

    case CONVERT (AC3_3F2R, AC3_MONO):
	zero (samples + 1024);
    case CONVERT (AC3_3F1R, AC3_MONO):
    case CONVERT (AC3_2F2R, AC3_MONO):
	zero (samples + 768);
    case CONVERT (AC3_3F, AC3_MONO):
    case CONVERT (AC3_2F1R, AC3_MONO):
	zero (samples + 512);
    case CONVERT (AC3_STEREO, AC3_MONO):
	zero (samples + 256);
	mix1to1 (samples, LEVEL_PLUS3DB, 0);
	break;

    case CONVERT (AC3_CHANNEL, AC3_MONO):
	zero (samples + 256);
	mix1to1 (samples, LEVEL_PLUS6DB, 0);
	break;

    case CONVERT (AC3_3F2R, AC3_STEREO):
    case CONVERT (AC3_3F2R, AC3_DOLBY):
	zero (samples + 1024);
    case CONVERT (AC3_3F1R, AC3_STEREO):
    case CONVERT (AC3_3F1R, AC3_DOLBY):
	zero (samples + 768);
    case CONVERT (AC3_3F, AC3_STEREO):
    case CONVERT (AC3_3F, AC3_DOLBY):
    mix_3to2:
	move1to1 (samples + 256, samples + 512, 1, 0);
	zero (samples + 256);
	goto mix_1to1;

    case CONVERT (AC3_2F2R, AC3_STEREO):
    case CONVERT (AC3_2F2R, AC3_DOLBY):
	zero (samples + 768);
    case CONVERT (AC3_2F1R, AC3_STEREO):
    case CONVERT (AC3_2F1R, AC3_DOLBY):
	zero (samples + 512);
    mix_2to2:
	mix1to1 (samples + 256, 1, 0);
	goto mix_1to1;

    case CONVERT (AC3_3F2R, AC3_3F):
	zero (samples + 1024);
    case CONVERT (AC3_3F1R, AC3_3F):
	zero (samples + 768);
    mix_3to3:
	mix1to1 (samples + 512, 1, 0);
	goto mix_2to2;

    case CONVERT (AC3_2F2R, AC3_2F1R):
	zero (samples + 768);
	mix1to1 (samples + 512, LEVEL_PLUS3DB, 0);
	goto mix_2to2;

    case CONVERT (AC3_3F2R, AC3_3F1R):
	zero (samples + 1024);
	mix1to1 (samples + 768, LEVEL_PLUS3DB, 0);
	goto mix_3to3;

    case CONVERT (AC3_3F1R, AC3_2F1R):
	move1to1 (samples + 512, samples + 768, 1, 0);
	goto mix_3to2;

    case CONVERT (AC3_3F2R, AC3_2F1R):
	zero (samples + 1024);
	move1to1 (samples + 512, samples + 768, LEVEL_PLUS3DB, 0);
	goto mix_3to2;

    case CONVERT (AC3_3F2R, AC3_2F2R):
	move1to1 (samples + 768, samples + 1024, 1, 0);
	move1to1 (samples + 512, samples + 768, 1, 0);
	goto mix_3to2;
    }
}
