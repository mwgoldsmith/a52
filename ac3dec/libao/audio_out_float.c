/*
 *
 *  output_linux.c
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

#include "config.h"

#include <inttypes.h>
#include <stdio.h>

#include "ac3.h"
#include "audio_out.h"

int float_setup (ao_instance_t * instance, int sample_rate, int * flags,
		 sample_t * level, sample_t * bias)
{
    *flags = AC3_STEREO;
    *level = 1;
    *bias = 0;

    return 0;
}

int float_play (ao_instance_t * instance, int flags, sample_t * samples)
{
    fwrite (samples, sizeof (sample_t), 256 * 2, stdout);

    return 0;
}

void float_close (ao_instance_t * instance)
{
}

static ao_instance_t instance = {float_setup, float_play, float_close};

ao_instance_t * ao_float_open (void)
{
    return &instance;
}
