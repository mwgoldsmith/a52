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

#ifdef LIBAO_OSS

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#if defined(__OpenBSD__)
#include <soundcard.h>
#elif defined(__FreeBSD__)
#include <machine/soundcard.h>
#else
#include <sys/soundcard.h>
#endif

#include "ac3.h"
#include "audio_out.h"

typedef struct oss_instance_s {
    ao_instance_t ao;
    int fd;
    int sample_rate;
    int set_params;
} oss_instance_t;

int oss_setup (ao_instance_t * _instance, int sample_rate, int * flags,
	       sample_t * level, sample_t * bias)
{
    oss_instance_t * instance = (oss_instance_t *) _instance;

    if (instance->sample_rate && (instance->sample_rate != sample_rate))
	return 1;
    instance->sample_rate = sample_rate;

    *flags = AC3_STEREO;
    *level = 1;
    *bias = 384;

    return 0;
}

static inline int16_t convert (int32_t i)
{
    if (i > 0x43c07fff)
	return 32767;
    else if (i < 0x43bf8000)
	return -32768;
    else
	return i - 0x43c00000;
}

static inline void float_to_int (float * _f, int16_t * s16) 
{
    int i;
    int32_t * f = (int32_t *) _f;	// XXX assumes IEEE float format

    for (i = 0; i < 256; i++) {
	s16[2*i] = convert (f[i]);
	s16[2*i+1] = convert (f[i+256]);
    }
}

int oss_play (ao_instance_t * _instance, int flags, sample_t * samples)
{
    oss_instance_t * instance = (oss_instance_t *) _instance;
    int16_t int16_samples[256*2];

    if (instance->set_params) {
	int tmp;

	tmp = 2;
	if ((ioctl (instance->fd, SNDCTL_DSP_CHANNELS, &tmp) < 0) ||
	    (tmp != 2)) {
	    fprintf (stderr, "Can not set number of channels\n");
	    return 1;
	}

	tmp = instance->sample_rate;
	if ((ioctl (instance->fd, SNDCTL_DSP_SPEED, &tmp) < 0) ||
	    (tmp != instance->sample_rate)) {
	    fprintf (stderr, "Can not set sample rate\n");
	    return 1;
	}

	instance->set_params = 0;
    }

    float_to_int (samples, int16_samples);
    write (instance->fd, int16_samples, 256 * 2 * sizeof (int16_t));

    return 0;
}

void oss_close (ao_instance_t * _instance)
{
    oss_instance_t * instance = (oss_instance_t *) _instance;

    close (instance->fd);
}

ao_instance_t * ao_oss_open (void)
{
    oss_instance_t * instance;
    int format;

    instance = malloc (sizeof (oss_instance_t));
    if (instance == NULL)
	return NULL;

    instance->ao.setup = oss_setup;
    instance->ao.play = oss_play;
    instance->ao.close = oss_close;

    instance->sample_rate = 0;
    instance->set_params = 1;

    instance->fd = open ("/dev/dsp", O_WRONLY);
    if (instance->fd < 0) {
	fprintf (stderr, "Can not open /dev/dsp\n");
	free (instance);
	return NULL;
    }

    format = AFMT_S16_NE;
    if ((ioctl (instance->fd, SNDCTL_DSP_SETFMT, &format) < 0) ||
	(format != AFMT_S16_NE)) {
	fprintf (stderr, "Can not set sample format\n");
	free (instance);
	return NULL;
    }

    return (ao_instance_t *) instance;
}

#endif
