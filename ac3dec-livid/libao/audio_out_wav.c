/*
 *
 *  audio_out_wav.c
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
 *  This file is based on output_linux.c by Aaron Holtzman.
 *  All .wav modifications were done by Jorgen Lundman <lundman@lundman.net>
 *  Any .wav bugs and errors should be reported to him.
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
#include <signal.h>

#include "audio_out.h"
#include "audio_out_internal.h"

#define WAVE_FORMAT_PCM  0x0001
#define FORMAT_MULAW     0x0101
#define IBM_FORMAT_ALAW  0x0102
#define IBM_FORMAT_ADPCM 0x0103


struct riff_struct 
{
  unsigned char id[4];   /* RIFF */
  unsigned long len;
  unsigned char wave_id[4]; /* WAVE */
};


struct chunk_struct 
{
	unsigned char id[4];
	unsigned long len;
};

struct common_struct 
{
	unsigned short wFormatTag;
	unsigned short wChannels;
	unsigned long dwSamplesPerSec;
	unsigned long dwAvgBytesPerSec;
	unsigned short wBlockAlign;
	unsigned short wBitsPerSample;  /* Only for PCM */
};

struct wave_header 
{
	struct riff_struct   riff;
	struct chunk_struct  format;
	struct common_struct common;
	struct chunk_struct  data;

	struct riff_struct   riffdata;
	struct chunk_struct  dataformat;
};


static ao_info_t ao_info =
{
	"WAV file output",
	"wav",
	"Aaron Holtzman <aholtzma@ess.engr.uvic.ca>",
	""
};

static char output_file[] = "output.wav";
static int fd;

static struct wave_header wave;
static void (*old_sig)(int);

static 
void signal_handler(int sig)
{
	ao_close();
	signal(sig, old_sig);
	raise(sig);
}

static uint_32 
ao_open(uint_32 bits, uint_32 rate, uint_32 channels)
{

	fd=open(output_file,O_WRONLY | O_TRUNC | O_CREAT, 0644);

	if(fd < 0) 
	{
		fprintf(stderr,"%s: Opening audio output %s\n", strerror(errno), output_file);
		goto ERR;
	}

	/* Write out a ZEROD wave header first */
	memset(&wave, 0, sizeof(wave));

	/* Store information */
	wave.common.wChannels = channels;
	wave.common.wBitsPerSample = bits;
	wave.common.dwSamplesPerSec = rate;

	if (write(fd, &wave, sizeof(wave)) != sizeof(wave)) 
	{
		fprintf(stderr,"failed to write wav-header: %s\n", strerror(errno));
		goto ERR;
	}

	//install our handler to properly write the riff header
	old_sig = signal(SIGINT,signal_handler);

	return 1;

ERR:
	if(fd >= 0) { close(fd); }
	return 0;
}


/*
 * play the sample to the already opened file descriptor
 */
static void
ao_play(sint_16* output_samples, uint_32 num_bytes)
{
	if(fd < 0)
		return;
  
	write(fd,output_samples,1024 * 6);

}


static void
ao_close(void)
{
	off_t size;

	/* Find how long our file is in total, including header */
	size = lseek(fd, 0, SEEK_CUR);

  if (size < 0) 
	{
		fprintf(stderr,"lseek failed - wav-header is corrupt\n");
		goto ERR;
	}

  /* Rewind file */
	if (lseek(fd, 0, SEEK_SET) < 0) 
	{
		fprintf(stderr,"rewind failed - wav-header is corrupt\n");
		goto ERR;
	}

	// Fill out our wav-header with some information. 
	size -= 8;

	strcpy(wave.riff.id, "RIFF");
	wave.riff.len = size + 24;
	strcpy(wave.riff.wave_id, "WAVE");
	size -= 4;

	size -= 8;
	strcpy(wave.format.id, "fmt ");
	wave.format.len = sizeof(struct common_struct);

	wave.common.wFormatTag = WAVE_FORMAT_PCM;
	wave.common.dwAvgBytesPerSec = 
		wave.common.wChannels * wave.common.dwSamplesPerSec *
		(wave.common.wBitsPerSample >> 4);

	wave.common.wBlockAlign = wave.common.wChannels * 
		(wave.common.wBitsPerSample >> 4);

	strcpy(wave.data.id, "data");

	size -= sizeof(struct common_struct);
	wave.data.len = size;

	size -= 8;
	strcpy(wave.riffdata.id, "RIFF");
	wave.riffdata.len = size;
	strcpy(wave.riffdata.wave_id, "WAVE");
	size -= 4;

	size -= 8;
	strcpy(wave.dataformat.id, "DATA");
	wave.dataformat.len = size;

	if (write(fd, &wave, sizeof(wave)) < sizeof(wave)) 
	{
		fprintf(stderr,"wav-header write failed -- file is corrupt\n");
		goto ERR;
	}

ERR:
	close(fd);
}

static const ao_info_t*
ao_get_info(void)
{
	return &ao_info;
}

//export our ao implementation
LIBAO_EXTERN(wav);
