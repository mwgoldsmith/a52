/*
 *
 *  downmix.c
 *    
 *	Copyright (C) Aaron Holtzman - Sept 1999
 *
 *	Originally based on code by Yuqing Deng.
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "ac3.h"
#include "ac3_internal.h"


#include "decode.h"
#include "downmix.h"
#include "debug.h"


#define LEVEL_PLUS3DB 1.4142135623730951
#define LEVEL_3DB 0.7071067811865476
#define LEVEL_6DB 0.5

#define AC3_CHANNEL 0
#define AC3_MONO 1
#define AC3_STEREO 2
#define AC3_3F 3
#define AC3_2F1R 4
#define AC3_3F1R 5
#define AC3_2F2R 6
#define AC3_3F2R 7
#define AC3_CHANNEL1 8
#define AC3_CHANNEL2 9
#define AC3_DOLBY 10

static void mix1to1 (float * samples, float level)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] *= level;
}

static void mix2to1 (float * samples, float level)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = (samples[i] + samples[i + 256]) * level;
}

static void mix3to1 (float * samples, float level, float clev)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = ((samples[i] + samples[i + 512]) * level +
		      samples[i + 256] * clev);
}

static void mix21to1 (float * samples, float level, float slev)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = ((samples[i] + samples[i + 256]) * level +
		      samples[i + 512] * slev);
}

static void mix31to1 (float * samples, float level, float clev, float slev)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = ((samples[i] + samples[i + 512]) * level +
		      samples[i + 256] * clev + samples[i + 768] * slev);
}

static void mix22to1 (float * samples, float level, float slev)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = ((samples[i] + samples[i + 256]) * level +
		      (samples[i + 512] + samples[i + 768]) * slev);
}

static void mix32to1 (float * samples, float level, float clev, float slev)
{
    int i;

    for (i = 0; i < 256; i++)
	samples[i] = ((samples[i] + samples[i + 512]) * level +
		      samples[i + 256] * clev +
		      (samples[i + 768] + samples[i + 1024]) * slev);
}

static void mix1to2 (float * src, float * dest, float level)
{
    int i;

    for (i = 0; i < 256; i++)
	dest[i] = src[i] *= level;
}

static void mix3to2 (float * samples, float level, float clev)
{
    int i;
    float common;

    for (i = 0; i < 256; i++) {
	common = samples[i + 256] * clev;
	samples[i] = samples[i] * level + common;
	samples[i + 256] = samples[i + 512] * level + common;
    }
}

static void mix21to2 (float * left, float * right, float level, float slev)
{
    int i;
    float common;

    for (i = 0; i < 256; i++) {
	common = right[i + 256] * slev;
	left[i] = left[i] * level + common;
	right[i] = right[i] * level + common;
    }
}

static void mix11to1 (float * front, float * rear, float level, float slev)
{
    int i;

    for (i = 0; i < 256; i++)
	front[i] = front[i] * level + rear[i] * slev;
}

static void mix31to2 (float * samples, float level, float clev, float slev)
{
    int i;
    float common;

    for (i = 0; i < 256; i++) {
	common = samples[i + 256] * clev + samples[i + 768] * slev;
	samples[i] = samples[i] * level + common;
	samples[i + 256] = samples[i + 512] * level + common;
    }
}

static void mix32to2 (float * samples, float level, float clev, float slev)
{
    int i;
    float center;

    for (i = 0; i < 256; i++) {
	center = samples[i + 256] * clev;
	samples[i] = samples[i] * level + center + samples[i + 768] * slev;
	samples[i + 256] = (samples[i + 512] * level + center +
			    samples[i + 1024] * slev);
    }
}

static void mix21toS (float * samples, float level, float level3db)
{
    int i;
    float surround;

    for (i = 0; i < 256; i++) {
	surround = samples[i + 512] * level3db;
	samples[i] = samples[i] * level - surround;
	samples[i + 256] = samples[i + 256] * level + surround;
    }
}

static void mix22toS (float * samples, float level, float level3db)
{
    int i;
    float surround;

    for (i = 0; i < 256; i++) {
	surround = (samples[i + 512] + samples[i + 768]) * level3db;
	samples[i] = samples[i] * level - surround;
	samples[i + 256] = samples[i + 256] * level + surround;
    }
}

static void mix31toS (float * samples, float level, float level3db)
{
    int i;
    float common, surround;

    for (i = 0; i < 256; i++) {
	common = samples[i + 256] * level3db;
	surround = samples[i + 768] * level3db;
	samples[i] = samples[i] * level + common - surround;
	samples[i + 256] = samples[i + 512] * level + common + surround;
    }
}

static void mix32toS (float * samples, float level, float level3db)
{
    int i;
    float common, surround;

    for (i = 0; i < 256; i++) {
	common = samples[i + 256] * level3db;
	surround = (samples[i + 768] + samples[i + 1024]) * level3db;
	samples[i] = samples[i] * level + common - surround;
	samples[i + 256] = samples[i + 512] * level + common + surround;
    }
}

int downmix (float * samples, int acmod, int output,
	     float level, int adjust_level, float clev, float slev)
{
    /* FIXME test if output variable is valid */

#define CONVERT(acmod,output) (((output) << 3) + (acmod))

    switch (CONVERT (acmod, output)) {
    case CONVERT (AC3_CHANNEL, AC3_MONO):
	mix2to1 (samples, level * LEVEL_6DB);
	return AC3_MONO;

    case CONVERT (AC3_CHANNEL, AC3_CHANNEL1):
	mix1to1 (samples, level);
	return AC3_CHANNEL1;

    case CONVERT (AC3_CHANNEL, AC3_CHANNEL2):
	mix1to1 (samples + 256, level);
	return AC3_CHANNEL2;

    case CONVERT (AC3_CHANNEL, AC3_CHANNEL):
    case CONVERT (AC3_CHANNEL, AC3_STEREO):
    case CONVERT (AC3_CHANNEL, AC3_DOLBY):
    case CONVERT (AC3_CHANNEL, AC3_3F):
    case CONVERT (AC3_CHANNEL, AC3_2F1R):
    case CONVERT (AC3_CHANNEL, AC3_3F1R):
    case CONVERT (AC3_CHANNEL, AC3_2F2R):
    case CONVERT (AC3_CHANNEL, AC3_3F2R):
	mix1to1 (samples, level);
	mix1to1 (samples + 256, level);
	return AC3_CHANNEL;


    case CONVERT (AC3_MONO, AC3_MONO):
    case CONVERT (AC3_MONO, AC3_CHANNEL1):
    case CONVERT (AC3_MONO, AC3_CHANNEL2):
	mix1to1 (samples, level);
	return AC3_MONO;

    case CONVERT (AC3_MONO, AC3_CHANNEL):
    case CONVERT (AC3_MONO, AC3_STEREO):
    case CONVERT (AC3_MONO, AC3_DOLBY):
    case CONVERT (AC3_MONO, AC3_3F):
    case CONVERT (AC3_MONO, AC3_2F1R):
    case CONVERT (AC3_MONO, AC3_3F1R):
    case CONVERT (AC3_MONO, AC3_2F2R):
    case CONVERT (AC3_MONO, AC3_3F2R):
	if (!adjust_level)
	    level *= LEVEL_3DB;
	mix1to2 (samples, samples + 256, level);
	return AC3_DOLBY;


    case CONVERT (AC3_STEREO, AC3_MONO):
    case CONVERT (AC3_STEREO, AC3_CHANNEL1):
    case CONVERT (AC3_STEREO, AC3_CHANNEL2):
    mix_2to1:
	mix2to1 (samples, level * (adjust_level ? LEVEL_6DB : LEVEL_3DB));
	return AC3_MONO;

    case CONVERT (AC3_STEREO, AC3_CHANNEL):
    case CONVERT (AC3_STEREO, AC3_STEREO):
    case CONVERT (AC3_STEREO, AC3_DOLBY):
    case CONVERT (AC3_STEREO, AC3_3F):
    case CONVERT (AC3_STEREO, AC3_2F1R):
    case CONVERT (AC3_STEREO, AC3_3F1R):
    case CONVERT (AC3_STEREO, AC3_2F2R):
    case CONVERT (AC3_STEREO, AC3_3F2R):
    mix_2to2:
	mix1to1 (samples, level);
	mix1to1 (samples + 256, level);
	return AC3_DOLBY;	// or AC3_STEREO depending on dsurmod


    case CONVERT (AC3_3F, AC3_MONO):
    case CONVERT (AC3_3F, AC3_CHANNEL1):
    case CONVERT (AC3_3F, AC3_CHANNEL2):
    mix_3to1:
	if (adjust_level)
	    level *= LEVEL_3DB / (1 + clev);
	mix3to1 (samples, level * LEVEL_3DB, level * clev * LEVEL_PLUS3DB);
	return AC3_MONO;

    case CONVERT (AC3_3F, AC3_CHANNEL):
    case CONVERT (AC3_3F, AC3_STEREO):
    case CONVERT (AC3_3F, AC3_2F1R):
    case CONVERT (AC3_3F, AC3_2F2R):
    mix_3to2:
	if (clev != LEVEL_3DB) {
	    if (adjust_level)
		level /= 1 + clev;
	    mix3to2 (samples, level, level * clev);
	    return AC3_STEREO;
	}	// else: fall thru

    case CONVERT (AC3_3F, AC3_DOLBY):
	if (adjust_level)
	    level *= 1 / (1 + LEVEL_3DB);
	mix3to2 (samples, level, level * LEVEL_3DB);
	return AC3_DOLBY;

    case CONVERT (AC3_3F, AC3_3F):
    case CONVERT (AC3_3F, AC3_3F1R):
    case CONVERT (AC3_3F, AC3_3F2R):
    mix_3to3:
	mix1to1 (samples, level);
	mix1to1 (samples + 256, level);
	mix1to1 (samples + 512, level);
	return AC3_3F;


    case CONVERT (AC3_2F1R, AC3_MONO):
    case CONVERT (AC3_2F1R, AC3_CHANNEL1):
    case CONVERT (AC3_2F1R, AC3_CHANNEL2):
	if (slev == 0)
	    goto mix_2to1;
	if (adjust_level)
	    level *= LEVEL_PLUS3DB / (2 + slev);
	mix21to1 (samples, level * LEVEL_3DB, level * slev * LEVEL_3DB);
	return AC3_MONO;

    case CONVERT (AC3_2F1R, AC3_CHANNEL):
    case CONVERT (AC3_2F1R, AC3_STEREO):
    case CONVERT (AC3_2F1R, AC3_3F):
	if (slev == 0)
	    goto mix_2to2;
	if (adjust_level)
	    level /= 1 + slev * LEVEL_3DB;
	mix21to2 (samples, samples + 256, level, level * slev * LEVEL_3DB);
	return AC3_STEREO;

    case CONVERT (AC3_2F1R, AC3_DOLBY):
	if (adjust_level)
	    level *= 1 / (1 + LEVEL_3DB);
	mix21toS (samples, level, level * LEVEL_3DB);
	return AC3_DOLBY;

    case CONVERT (AC3_2F1R, AC3_2F1R):
    case CONVERT (AC3_2F1R, AC3_3F1R):
	mix1to1 (samples, level);
	mix1to1 (samples + 256, level);
	mix1to1 (samples + 512, level);
	return AC3_2F1R;

    case CONVERT (AC3_2F1R, AC3_2F2R):
    case CONVERT (AC3_2F1R, AC3_3F2R):
	mix1to1 (samples, level);
	mix1to1 (samples + 256, level);
	mix1to2 (samples + 512, samples + 768, level * LEVEL_3DB);
	return AC3_2F2R;


    case CONVERT (AC3_3F1R, AC3_MONO):
    case CONVERT (AC3_3F1R, AC3_CHANNEL1):
    case CONVERT (AC3_3F1R, AC3_CHANNEL2):
	if (slev == 0)
	    goto mix_3to1;
	if (adjust_level)
	    level *= LEVEL_PLUS3DB / (2 + 2 * clev + slev);
	mix31to1 (samples, level * LEVEL_3DB,
		  level * clev * LEVEL_PLUS3DB, level * slev * LEVEL_3DB);
	return AC3_MONO;

    case CONVERT (AC3_3F1R, AC3_CHANNEL):
    case CONVERT (AC3_3F1R, AC3_STEREO):
	if (slev == 0)
	    goto mix_3to2;
	if (adjust_level)
	    level /= 1 + clev + slev * LEVEL_3DB;
	mix31to2 (samples, level, level * clev, level * slev * LEVEL_3DB);
	return AC3_STEREO;

    case CONVERT (AC3_3F1R, AC3_DOLBY):
	if (adjust_level)
	    level *= 1 / (1 + 2 * LEVEL_3DB);
	mix31toS (samples, level, level * LEVEL_3DB);
	return AC3_DOLBY;

    case CONVERT (AC3_3F1R, AC3_3F):
	if (slev == 0)
	    goto mix_3to3;
	if (adjust_level)
	    level /= 1 + slev * LEVEL_3DB;
	mix21to2 (samples, samples + 512, level, level * slev * LEVEL_3DB);
	mix1to1 (samples + 256, level);
	return AC3_3F;

    case CONVERT (AC3_3F1R, AC3_2F1R):
	if (adjust_level)
	    level /= 1 + clev;
	mix3to2 (samples, level, level * clev);
	mix1to1 (samples + 768, level);
	return AC3_2F1R;	// FIXME rear pointer

    case CONVERT (AC3_3F1R, AC3_3F1R):
	mix1to1 (samples, level);
	mix1to1 (samples + 256, level);
	mix1to1 (samples + 512, level);
	mix1to1 (samples + 768, level);
	return AC3_3F1R;

    case CONVERT (AC3_3F1R, AC3_2F2R):
	if (adjust_level)
	    level /= 1 + clev;
	mix3to2 (samples, level, level * clev);
	mix1to2 (samples + 768, samples + 512, level * LEVEL_3DB);
	return AC3_2F2R;

    case CONVERT (AC3_3F1R, AC3_3F2R):
	mix1to1 (samples, level);
	mix1to1 (samples + 256, level);
	mix1to1 (samples + 512, level);
	mix1to2 (samples + 768, samples + 1024, level * LEVEL_3DB);
	return AC3_3F2R;


    case CONVERT (AC3_2F2R, AC3_MONO):
    case CONVERT (AC3_2F2R, AC3_CHANNEL1):
    case CONVERT (AC3_2F2R, AC3_CHANNEL2):
	if (slev == 0)
	    goto mix_2to1;
	if (adjust_level)
	    level *= LEVEL_3DB / (1 + slev);
	mix22to1 (samples, level * LEVEL_3DB, level * slev * LEVEL_3DB);
	return AC3_MONO;

    case CONVERT (AC3_2F2R, AC3_CHANNEL):
    case CONVERT (AC3_2F2R, AC3_STEREO):
    case CONVERT (AC3_2F2R, AC3_3F):
	if (slev == 0)
	    goto mix_2to2;
	if (adjust_level)
	    level /= (1 + slev);
	mix11to1 (samples, samples + 512, level, level * slev);
	mix11to1 (samples + 256, samples + 768, level, level * slev);
	return AC3_STEREO;

    case CONVERT (AC3_2F2R, AC3_DOLBY):
	if (adjust_level)
	    level *= 1 / (1 + 2 * LEVEL_3DB);
	mix22toS (samples, level, level * LEVEL_3DB);
	return AC3_DOLBY;

    case CONVERT (AC3_2F2R, AC3_2F1R):
    case CONVERT (AC3_2F2R, AC3_3F1R):
	if (adjust_level)
	    level *= LEVEL_3DB;
	mix1to1 (samples, level);
	mix1to1 (samples + 256, level);
	mix2to1 (samples + 512, level * LEVEL_3DB);
	return AC3_2F1R;

    case CONVERT (AC3_2F2R, AC3_2F2R):
    case CONVERT (AC3_2F2R, AC3_3F2R):
	mix1to1 (samples, level);
	mix1to1 (samples + 256, level);
	mix1to1 (samples + 512, level);
	mix1to1 (samples + 768, level);
	return AC3_2F2R;


    case CONVERT (AC3_3F2R, AC3_MONO):
    case CONVERT (AC3_3F2R, AC3_CHANNEL1):
    case CONVERT (AC3_3F2R, AC3_CHANNEL2):
	if (slev == 0)
	    goto mix_3to1;
	if (adjust_level)
	    level *= LEVEL_3DB / (1 + clev + slev);
	mix32to1 (samples, level * LEVEL_3DB,
		  level * clev * LEVEL_PLUS3DB, level * slev * LEVEL_3DB);
	return AC3_MONO;

    case CONVERT (AC3_3F2R, AC3_CHANNEL):
    case CONVERT (AC3_3F2R, AC3_STEREO):
	if (slev == 0)
	    goto mix_3to2;
	if (adjust_level)
	    level /= 1 + clev + slev;
	mix32to2 (samples, level, level * clev, level * slev);
	return AC3_STEREO;

    case CONVERT (AC3_3F2R, AC3_DOLBY):
	if (adjust_level)
	    level *= 1 / (1 + 3 * LEVEL_3DB);
	mix32toS (samples, level, level * LEVEL_3DB);
	return AC3_DOLBY;

    case CONVERT (AC3_3F2R, AC3_3F):
	if (slev == 0)
	    goto mix_3to3;
	if (adjust_level)
	    level /= 1 + slev;
	mix11to1 (samples, samples + 768, level, level * slev);
	mix1to1 (samples + 256, level);
	mix11to1 (samples + 512, samples + 1024, level, level * slev);
	return AC3_3F;

    case CONVERT (AC3_3F2R, AC3_2F1R):
	if (adjust_level) {
	    if (clev > LEVEL_PLUS3DB - 1)
		level /= 1 + clev;
	    else
		level *= LEVEL_3DB;
	}
	mix3to2 (samples, level, level * clev);
	mix2to1 (samples + 768, level * LEVEL_3DB);
	return AC3_2F1R;	// FIXME rear pointer

    case CONVERT (AC3_3F2R, AC3_3F1R):
	if (adjust_level)
	    level *= LEVEL_3DB;
	mix1to1 (samples, level);
	mix1to1 (samples + 256, level);
	mix1to1 (samples + 512, level);
	mix2to1 (samples + 768, level * LEVEL_3DB);
	return AC3_3F1R;

    case CONVERT (AC3_3F2R, AC3_2F2R):
	if (adjust_level)
	    level /= 1 + clev;
	mix3to2 (samples, level, level * clev);
	mix1to1 (samples + 768, level);
	mix1to1 (samples + 1024, level);
	return AC3_2F2R;	// FIXME rear pointer

    case CONVERT (AC3_3F2R, AC3_3F2R):
	mix1to1 (samples, level);
	mix1to1 (samples + 256, level);
	mix1to1 (samples + 512, level);
	mix1to1 (samples + 768, level);
	mix1to1 (samples + 1024, level);
	return AC3_3F2R;

    default:	// NOT REACHED
	return -1;
    }
}
