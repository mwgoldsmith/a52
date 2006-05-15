//PLUGIN_INFO(INFO_NAME, "audio driver output");
//PLUGIN_INFO(INFO_AUTHOR, "Aaron Holtzman <aholtzma@ess.engr.uvic.ca>");

/*
 *
 *  audio_out_sys.c
 *    
 *	Copyright (C) Aaron Holtzman - May 1999
 *	Port to IRIX by Jim Miller, SGI - Nov 1999
 *	Port to output plugin/unified support for linux/solaris/...
 *		by Thome Mirlacher - Jul 2000
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


// FIXME: which define do we need for IRIX ???
//	the irix part doesn't work right now ...

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>

#if defined (__OpenBSD__)
#	include <soundcard.h>
#elif defined (__FreeBSD__)
#	include <machine/soundcard.h>
#elif defined (__sun__)
#	include <sys/audioio.h>
#	include <stropts.h>
#	include <signal.h>
#elif defined (__irix__) 
#	include <audio.h>
#else
#	include <sys/soundcard.h>
#endif


#include <oms/oms.h>
#include <oms/plugin/output_audio.h>

// FIXME: move all global variables into a priv struct
static int fd;
#if defined (__irix__)
static ALport alport = 0;
static ALconfig alconfig = 0;
static int bytesPerWord = 1;
static int nChannels = 2;
#endif
 
static int _audio_sys_open (plugin_t *plugin, void *name);
static int _audio_sys_close (plugin_t *plugin);
static int _audio_sys_setup (plugin_output_audio_attr_t *attr);
static int _audio_sys_write (const void *buf, size_t num_bytes);

static plugin_output_audio_t audio_sys = {
        open:		_audio_sys_open,
        close:		_audio_sys_close,
        setup:		_audio_sys_setup,
        write:		_audio_sys_write
};


/**
 * open device
 **/

static int _audio_sys_open (plugin_t *plugin, void *name)
{
#if defined (__irix__)
	ALpv params[2];
	int  dev = AL_DEFAULT_OUTPUT;
	int  wsize = AL_SAMPLE_16;

	nChannels = attr->channels;

	alconfig = alNewConfig();

	if (alSetQueueSize(alconfig, BUFFER_SIZE) < 0) {
		fprintf(stderr, "alSetQueueSize failed: %s\n", alGetErrorString(oserror()));
		return -1;
	}

	if (alSetChannels(alconfig, attr->channels) < 0) {
		fprintf(stderr, "alSetChannels(%d) failed: %s\n", attr->channels, alGetErrorString(oserror()));
		return -1;
	}

	if (alSetDevice(alconfig, dev) < 0) {
		fprintf(stderr, "alSetDevice failed: %s\n", alGetErrorString(oserror()));
		return -1;
	}

	if (alSetSampFmt(alconfig, AL_SAMPFMT_TWOSCOMP) < 0) {
		fprintf(stderr, "alSetSampFmt failed: %s\n", alGetErrorString(oserror()));
		return -1;
	}

	alport = alOpenPort("AC3Decode", "w", 0);

	if (!alport) {
		fprintf(stderr, "alOpenPort failed: %s\n", alGetErrorString(oserror()));
		return -1;
	}

	switch (attr->bits) {
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
		fprintf(stderr,"Irix audio: unsupported bit with %d\n", attr->bits);
		break;
	}

	if (alSetWidth(alconfig, wsize) < 0) {
		fprintf(stderr, "alSetWidth failed: %s\n", alGetErrorString(oserror()));
		return -1;
	}

	params[0].param = AL_RATE;
	params[0].value.ll = alDoubleToFixed((double)attr->rate);
	params[1].param = AL_MASTER_CLOCK;
	params[1].value.i = AL_CRYSTAL_MCLK_TYPE;
	if ( alSetParams(dev, params, 1) < 0) {
		printf("alSetParams() failed: %s\n", alGetErrorString(oserror()));
		return -1;
	}

#else
// Open the device driver
	if ((fd = open ((char *) name, O_WRONLY)) < 0) {
		fprintf(stderr,"%s: Opening audio device %s\n", strerror(errno), (char *) name);
		return -1;
	}
        fprintf (stderr, "Opened audio device \"%s\"\n", (char *) name);
#endif
	return 0;
}


/**
 *
 **/

static int _audio_sys_close (struct plugin_s *plugin)
{
#if defined (__irix__)
	alClosePort(alport);
        alFreeConfig(alconfig);
        alport = 0;
        alconfig = 0;
        init = 0;

	return 0;
#else
	return close (fd);
#endif
}


/**
 * setup audio device
 **/

static int _audio_sys_setup (plugin_output_audio_attr_t *attr)
{
#if defined (__sun__)
/* Setup our parameters */
	audio_info_t info;

	AUDIO_INITINFO (&info);

	info.play.precision = attr->bits;
	info.play.sample_rate = attr->rate;
	info.play.channels = attr->channels;
	//info.play.buffer_size = 1024;
	info.play.encoding = AUDIO_ENCODING_LINEAR;
	//info.play.port = AUDIO_SPEAKER;
	//info.play.gain = 110;

/* Write our configuration */
	if (ioctl (fd, AUDIO_SETINFO, &info) < 0) {
		fprintf (stderr, "%s: Writing audio config block\n", strerror (errno));
		return -1;
        }
#else
	int tmp;
  
	tmp = attr->channels == 2 ? 1 : 0;
	ioctl (fd, SNDCTL_DSP_STEREO, &tmp);

//FIXME: the bits should be more a format - now we're only ablt to use
//	8 and 16 bit values ...

	switch (attr->bits) {
	case 16:
		tmp = AFMT_S16_NE;
		break;
	default:
		tmp = attr->bits;
	}
	ioctl (fd, SNDCTL_DSP_SAMPLESIZE, &tmp);

	tmp = attr->rate;
	ioctl (fd, SNDCTL_DSP_SPEED, &tmp);

	//this is cheating
	tmp = 256;
	ioctl (fd, SNDCTL_DSP_SETFRAGMENT, &tmp);
#endif
	return 0;
}


/**
 * play the sample to the already opened file descriptor
 **/

static int _audio_sys_write (const void *buf, size_t num_bytes)
{
#if defined (__irix__)
	alWriteFrames (alport, buf, num_bytes);
	return num_bytes;
#else
	return write (fd, buf, num_bytes);
#endif
}


/**
 * Initialize Plugin.
 **/

int plugin_init (char *whoami)
{
        pluginRegister (whoami,
                PLUGIN_ID_OUTPUT_AUDIO,
                "sys",
		NULL,
		NULL,
                &audio_sys);

        return 0;
}


/**
 * Cleanup Plugin.
 **/

void plugin_exit (void)
{
}
