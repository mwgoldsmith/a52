/*
 * audio_out_irix.c
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

#include "config.h"

#ifdef LIBAO_IRIX

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include <audio.h>


typedef signed short int16_t;
typedef unsigned int uint32_t;
#include "output.h"

static int init = 0;
static ALport alport = 0;
static ALconfig alconfig = 0;
static int bytesPerWord = 1;
static int nChannels = 2;


/*
 * open the audio device for writing to
 */
int output_open(int bits, int rate, int channels)
{
  ALpv params[2];
  int  dev = AL_DEFAULT_OUTPUT;
  int  wsize = AL_SAMPLE_16;

  nChannels = channels;

  if (!init) {
    init = 1;
    alconfig = alNewConfig();

    if (alSetQueueSize(alconfig, BUFFER_SIZE) < 0) {
        fprintf(stderr, "alSetQueueSize failed: %s\n",
                alGetErrorString(oserror()));
        return 0;
    }

    if (alSetChannels(alconfig, channels) < 0) {
        fprintf(stderr, "alSetChannels(%d) failed: %s\n",
                channels, alGetErrorString(oserror()));
        return 0;
    }

    if (alSetDevice(alconfig, dev) < 0) {
        fprintf(stderr, "alSetDevice failed: %s\n",
                        alGetErrorString(oserror()));
        return 0;
    }

    if (alSetSampFmt(alconfig, AL_SAMPFMT_TWOSCOMP) < 0) {
        fprintf(stderr, "alSetSampFmt failed: %s\n",
                        alGetErrorString(oserror()));
        return 0;
    }

    alport = alOpenPort("A52Decode", "w", 0);
    if (!alport) {
        fprintf(stderr, "alOpenPort failed: %s\n",
                        alGetErrorString(oserror()));
        return 0;
    }

    switch (bits) {
        case 8:         
                bytesPerWord = 1;
                wsize = AL_SAMPLE_8;
                break;
        case 16: 
                bytesPerWord = 2;
                wsize = AL_SAMPLE_16;
                break;
        case 24:
                bytesPerWord = 4;
                wsize = AL_SAMPLE_24;
                break;
        default:
                printf("Irix audio: unsupported bit with %d\n", bits);
                break;
    }

    if (alSetWidth(alconfig, wsize) < 0) {
        fprintf(stderr, "alSetWidth failed: %s\n", alGetErrorString(oserror()));
        return 0;
    }
        
    params[0].param = AL_RATE;
    params[0].value.ll = alDoubleToFixed((double)rate);
    params[1].param = AL_MASTER_CLOCK;
    params[1].value.i = AL_CRYSTAL_MCLK_TYPE;
    if ( alSetParams(dev, params, 1) < 0) {
        printf("alSetParams() failed: %s\n", alGetErrorString(oserror()));
        return 0;
    }
  }

	printf("I've synced the IRIX code with the mainline blindly.\n Let me know if it works.\n");

  return 1;
}

/*
 * play the sample to the already opened file descriptor
 */

void output_play(int16_t* output_samples, uint32_t num_bytes)
{
	alWriteFrames(alport, output_samples, 6 * 256); 
}

void
output_close(void)
{
  alClosePort(alport);
  alFreeConfig(alconfig);
  alport = 0;
  alconfig = 0;
  init = 0;
}

#endif
