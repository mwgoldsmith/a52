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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#if defined(__OpenBSD__)
#include <soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#include <sys/ioctl.h>


#include "ac3.h"
#include "decode.h"
#include "debug.h"
#include "output.h"
#include "downmix.h"
#include "ring_buffer.h"


#define BUFFER_SIZE 1024 

static char dev[] = "/dev/dsp";
static int fd;

//FIXME uncomment all the matlab calls in this module
//      and you'll get a handy matlab output file
//      of the signal.
//#include "matlab.h"
//static matlab_file_t *foo;
//


/*
 * open the audio device for writing to
 */
int output_open(int bits, int rate, int channels)
{
  int tmp;
  
  /*
   * Open the device driver
   */

	fd=open(dev,O_WRONLY | O_NDELAY);
  if(fd < 0) 
  {
    dprintf("%s: Opening audio device %s\n",
        strerror(errno), dev);
    goto ERR;
  }
	dprintf("Opened audio device \"%s\"\n",dev);

	fcntl(fd,F_SETFL,O_NONBLOCK);

	tmp = BUFFER_SIZE;
  ioctl(fd,SNDCTL_DSP_SETFRAGMENT,&tmp);


  tmp = bits;
  ioctl(fd,SNDCTL_DSP_SAMPLESIZE,&tmp);

  tmp = channels == 2 ? 1 : 0;
  ioctl(fd,SNDCTL_DSP_STEREO,&tmp);

  tmp = rate;
  ioctl(fd,SNDCTL_DSP_SPEED, &tmp);

	/* Initialize the ring buffer */
	rb_init();

	//FIXME remove
	//	foo = matlab_open("foo.m");
	//
	
	return 1;

ERR:
  if(fd >= 0) { close(fd); }
  return 0;
}

static void
output_flush(void)
{
	int i,j = 0;
	sint_16 *out_buf = 0;

	i = 0;

	do
	{
		out_buf = rb_begin_read();
		if(out_buf)
			i = write(fd, out_buf,BUFFER_SIZE);
		else
			break;

		if(i == BUFFER_SIZE)
		{
			rb_end_read();
			j++;
		}
	}
	while(i == BUFFER_SIZE);
	
	//FIXME remove
	//fprintf(stderr,"(output) Flushed %d blocks, wrote %d bytes last frame\n",j,i);
}

/*
 * play the sample to the already opened file descriptor
 */
void output_play(bsi_t *bsi,stream_samples_t *samples)
{
  int i;
	float *left,*right;
	float norm;
	float left_tmp = 0.0;
	float right_tmp = 0.0;
	sint_16 *out_buf;

	if(fd < 0)
		return;

	out_buf = rb_begin_write();

	/* Keep trying to dump frames from the ring buffer until we get a 
	 * write slot available */
	while(!out_buf)
	{
		usleep(1000);
		output_flush();
		out_buf = rb_begin_write();
	} 

	//FIXME remove
	//matlab_write(foo,samples->channel[0],512);
	
	//Downmix if necessary 
	downmix(bsi,samples);

	//Determine a normalization constant if the signal exceeds 
	//100% digital [-1.0,1.0]
	//
	//perhaps use the dynamic range info to do this instead
	
	norm = 1.0;

	for(i=0; i< 256;i++)
	{
    left_tmp = samples->channel[0][i];
    right_tmp = samples->channel[1][i];

		if(left_tmp > norm)
			norm = left_tmp;
		else if(left_tmp < -norm)
			norm = -left_tmp;

		if(right_tmp > norm)
			norm = right_tmp;
		else if(right_tmp < -norm)
			norm = -right_tmp; 
	}
	norm = 32000.0f/norm;

	/* Take the floating point audio data and convert it into
	 * 16 bit signed PCM data */
	left = samples->channel[0];
	right = samples->channel[1];

	for(i=0; i < 256; i++)
	{
	//	if((fabs(*left * norm) > 32768.0) || (fabs(*right * norm) > 32768.0))
	//		printf("clipping (%f, %f)\n",*left,*right);
		out_buf[i * 2 ]    = (sint_16) (*left++  * norm);
		out_buf[i * 2 + 1] = (sint_16) (*right++ * norm);

	}
	rb_end_write();

}


void
output_close(void)
{
	close(fd);
}
