/*
 *
 *  audio_out_solaris.c
 *
 *  Copyright (C) Aaron Holtzman - May 1999
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
#include <sys/audioio.h>
#include <sys/ioctl.h>
#include <stropts.h>
#include <signal.h>
#include <math.h>

//FIXME broken solaris headers!
int usleep(unsigned int useconds);

#include "audio_out.h"
#include "audio_out_internal.h"

static ao_info_t ao_info =
{
	"Solaris audio output ",
	"sol",
	"Aaron Holtzman <aholtzma@ess.engr.uvic.ca>",
	""
};

/* Global to keep track of old state */
static audio_info_t info;
static char dev[] = "/dev/audio";
static int fd;

/*
 * open the audio device for writing to
 */
static int
ao_open(uint32_t bits, uint32_t rate, uint32_t channels)
{

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

	/* Setup our parameters */
	AUDIO_INITINFO(&info);

	info.play.sample_rate = rate;
	info.play.precision = bits;
	info.play.channels = channels;
	//info.play.buffer_size = 1024;
	info.play.encoding = AUDIO_ENCODING_LINEAR;
	//info.play.port = AUDIO_SPEAKER;
	//info.play.gain = 110;

	/* Write our configuration */
	/* An implicit GETINFO is also performed so we can get
	 * the buffer_size */

	if(ioctl(fd, AUDIO_SETINFO, &info) < 0)
	{
		fprintf(stderr, "%s: Writing audio config block\n",
			strerror(errno));
		goto ERR;
	}

	return 1;

ERR:
	if(fd >= 0) { close(fd); }
	return 0;
}

/*
 * play the sample to the already opened file descriptor
 */
static void 
ao_play(int16_t* output_samples, size_t num_bytes)
{
	write(fd,output_samples,1024 * 6);
}


static void
ao_close(void)
{
	close(fd);
}

static const ao_info_t*
ao_get_info(void)
{
	return &ao_info;
}

//export our ao implementation
LIBAO_EXTERN(norm);
