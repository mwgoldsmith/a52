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

#include "ac3.h"
#include "audio_out.h"

typedef struct null_instance_s {
    ao_instance_t ao;
    int channels;
} null_instance_t;

int null_setup (ao_instance_t * _instance, int sample_rate, int * flags,
		sample_t * level, sample_t * bias)
{
    null_instance_t * instance = (null_instance_t *) _instance;

    *flags = instance->channels;
    *level = 1;
    *bias = 0;

    return 0;
}

int null_play (ao_instance_t * instance, int flags, sample_t * samples)
{
    return 0;
}

void null_close (ao_instance_t * instance)
{
}

static null_instance_t instance = {{null_setup, null_play, null_close}, 0};

ao_instance_t * ao_null_open (void)
{
    instance.channels = AC3_3F2R | AC3_LFE;

    return (ao_instance_t *) &instance;
}

ao_instance_t * ao_null2_open (void)
{
    instance.channels = AC3_STEREO;

    return (ao_instance_t *) &instance;
}

ao_instance_t * ao_null4_open (void)
{
    instance.channels = AC3_2F2R;

    return (ao_instance_t *) &instance;
}
