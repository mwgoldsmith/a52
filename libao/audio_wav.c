//PLUGIN_INFO(INFO_NAME, "WAV file audio driver output");
//PLUGIN_INFO(INFO_AUTHOR, "Aaron Holtzman <aholtzma@ess.engr.uvic.ca>");


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

#include <oms/oms.h>
#include <oms/plugin/output_audio.h>

#define WAVE_FORMAT_PCM  0x0001
#define FORMAT_MULAW     0x0101
#define IBM_FORMAT_ALAW  0x0102
#define IBM_FORMAT_ADPCM 0x0103


struct riff_struct 
{
	u_char id[4];   /* RIFF */
	u_long len;
	u_char wave_id[4]; /* WAVE */
};


struct chunk_struct 
{
	u_char id[4];
	u_long len;
};

struct common_struct 
{
	u_short wFormatTag;
	u_short wChannels;
	u_long dwSamplesPerSec;
	u_long dwAvgBytesPerSec;
	u_short wBlockAlign;
	u_short wBitsPerSample;  /* Only for PCM */
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


static int fd;

static struct wave_header wave;
static void (*old_sig)(int);

static int _audio_wav_open (plugin_t *plugin, void *name);
static int _audio_wav_close (plugin_t *plugin);
static int _audio_wav_setup (plugin_output_audio_attr_t *attr);
static int _audio_wav_write (const void *buf, size_t num_bytes);

static plugin_output_audio_t audio_wav = {
	open:		_audio_wav_open,
	close:		_audio_wav_close,
	setup:		_audio_wav_setup,
	write:		_audio_wav_write
};


/**
 *
 **/

static void signal_handler(int sig)
{
// FIXME
	_audio_wav_close(NULL);
	signal(sig, old_sig);
	raise(sig);
}


/**
 *
 **/

static int _audio_wav_open (plugin_t *plugin, void *name)
{
// Open the device driver
	if ((fd = open ((char *) name, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0) {
		fprintf(stderr,"%s: Opening audio device %s\n", strerror(errno), (char *) name);
		return -1;
	}

	return 0;
}

/**
 *
 **/

static int _audio_wav_close (struct plugin_s *plugin)
{
	off_t size;

// Find how long our file is in total, including header
	size = lseek (fd, 0, SEEK_CUR);

	if (size < 0) {
		fprintf(stderr,"lseek failed - wav-header is corrupt\n");
		goto error;
	}

// Rewind file
	if (lseek(fd, 0, SEEK_SET) < 0) {
		fprintf(stderr,"rewind failed - wav-header is corrupt\n");
		goto error;
	}

// Fill some information into the wav-header
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

	if (write(fd, &wave, sizeof(wave)) < sizeof(wave)) {
		fprintf(stderr,"wav-header write failed -- file is corrupt\n");
		goto error;
	}

error:
	return close (fd);
}


/**
 *
 **/

static int _audio_wav_setup (plugin_output_audio_attr_t *attr)
{
	/* Write out a ZEROD wave header first */
	memset(&wave, 0, sizeof(wave));

	/* Store information */
	wave.common.wChannels = attr->channels;
	wave.common.wBitsPerSample = attr->bits;
	wave.common.dwSamplesPerSec = attr->rate;

	if (write(fd, &wave, sizeof(wave)) != sizeof(wave)) {
		fprintf(stderr,"failed to write wav-header: %s\n", strerror(errno));
		goto error;
	}

	//install our handler to properly write the riff header
	old_sig = signal(SIGINT,signal_handler);

	return 1;

error:
	if(fd >= 0) { close(fd); }
	return 0;
}


/**
 * play the sample to the already opened file descriptor
 **/

static int _audio_wav_write (const void *buf, size_t num_bytes)
{
	return write (fd, buf, num_bytes);
}


/**
 * Initialize Plugin.
 **/

int plugin_init (char *whoami)
{
	pluginRegister (whoami,
		PLUGIN_ID_OUTPUT_AUDIO,
		"wav",
		NULL,
		NULL,
		&audio_wav);

        return 0;
}


/**
 * Cleanup Plugin.
 **/

void plugin_exit (void)
{
}
