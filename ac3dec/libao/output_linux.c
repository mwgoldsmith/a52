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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#if defined(__OpenBSD__)
#include <soundcard.h>
#elif defined(__FreeBSD__)
#include <machine/soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#include <sys/ioctl.h>

#include <inttypes.h>

#include "libao.h"


static char dev[] = "/dev/dsp";
static int fd;


/*
 * open the audio device for writing to
 */
int output_open(int bits, int rate, int channels)
{
  int tmp;
  
#if 1
    fd = open("output.pcm",O_WRONLY|O_CREAT|O_TRUNC,0600);
    if (fd < 0)
	printf ("open failed\n");
    return (fd >= 0);
#endif

  /*
   * Open the device driver
   */

	fd=open(dev,O_WRONLY);
  if(fd < 0) 
  {
    fprintf(stderr,"%s: Opening audio device %s\n",
        strerror(errno), dev);
    goto ERR;
  }

  tmp = channels == 2 ? 1 : 0;
  ioctl(fd,SNDCTL_DSP_STEREO,&tmp);

  tmp = bits;
  ioctl(fd,SNDCTL_DSP_SAMPLESIZE,&tmp);

  tmp = rate;
  ioctl(fd,SNDCTL_DSP_SPEED, &tmp);

	//this is cheating
	tmp = 256;
  ioctl(fd,SNDCTL_DSP_SETFRAGMENT,&tmp);



	return 1;

ERR:
  if(fd >= 0) { close(fd); }
  return 0;
}

/*
 * play the sample to the already opened file descriptor
 */
void output_play(int16_t* output_samples, uint32_t num_bytes)
{
//	if(fd < 0)
//		return;

	write(fd,output_samples,1024 * 6);
}


void
output_close(void)
{
	close(fd);
}

#endif
