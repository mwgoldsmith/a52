/*
 *
 *  output_irix.c
 *    
 *      Copyright (C) Aaron Holtzman - May 1999
 *      Port to IRIX by Jim Miller, SGI - Nov 1999
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
 *
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include <audio.h>

#include "ac3.h"
#include "decode.h"
#include "debug.h"
#include "output.h"
#include "downmix.h"
#include "ring_buffer.h"


#define BUFFER_SIZE 1024

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

    alport = alOpenPort("AC3Decode", "w", 0);
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

  return 1;
}

static void
output_flush(void)
{
  int i,j = 0;
  sint_16 *out_buf = 0;

    out_buf = rb_begin_read();
    if(out_buf) {
        alWriteFrames(alport, out_buf, BUFFER_SIZE /(nChannels*bytesPerWord)); 
    } 

    rb_end_read();
}

/*
 * play the sample to the already opened file descriptor
 */
void output_play(bsi_t *bsi,stream_samples_t *samples)
{

  int i;
        float *left,*right;
        float norm = 1.0;
        float left_tmp = 0.0;
        float right_tmp = 0.0;
        sint_16 *out_buf;

        if(!alport)
                return;

        out_buf = rb_begin_write();

        /* Keep trying to dump frames from the ring buffer until we get a
         * write slot available */
        while(!out_buf)
        {
                output_flush();
                out_buf = rb_begin_write();
        }

        //Downmix if necessary
        downmix(bsi,samples);

        //Determine a normalization constant if the signal exceeds
        //100% digital [-1.0,1.0]
        //
        //perhaps use the dynamic range info to do this instead
        for(i=0; i< 256;i++)
        {
    left_tmp = samples->channel[0][i];
    right_tmp = samples->channel[1][i];

                if(left_tmp > norm)
                        norm = left_tmp;
                if(left_tmp < -norm)
                        norm = -left_tmp;

                if(right_tmp > norm)
                        norm = right_tmp;
                if(right_tmp < -norm)
                        norm = -right_tmp;
        }
        norm = 32000.0/norm;

        /* Take the floating point audio data and convert it into
         * 16 bit signed PCM data */
        left = samples->channel[0];
        right = samples->channel[1];

        for(i=0; i < 256; i++)
        {
        //      if((fabs(*left * norm) > 32768.0) || (fabs(*right * norm) > 32768.0))
        //              printf("clipping (%f, %f)\n",*left,*right);
                out_buf[i * 2 ]    = (sint_16) (*left++  * norm);
                out_buf[i * 2 + 1] = (sint_16) (*right++ * norm);

        }
        rb_end_write();

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
