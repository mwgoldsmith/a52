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

#include <sys/audioio.h>
#include <stropts.h>
#include <signal.h>


#include <oms/oms.h>
#include <oms/plugin/output_audio.h>

// FIXME: move all global variables into a priv struct
static int fd;
 
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
// Open the device driver
	if ((fd = open ((char *) name, O_WRONLY)) < 0) {
		fprintf(stderr,"%s: Opening audio device %s\n", strerror(errno), (char *) name);
		return -1;
	}
        fprintf (stderr, "Opened audio device \"%s\"\n", (char *) name);

	return 0;
}


/**
 *
 **/

static int _audio_sys_close (struct plugin_s *plugin)
{
	return close (fd);
}


/**
 * setup audio device
 **/

static int _audio_sys_setup (plugin_output_audio_attr_t *attr)
{
// Setup parameters
	audio_info_t info;

	AUDIO_INITINFO (&info);

	info.play.precision = attr->bits;
	info.play.sample_rate = attr->rate;
	info.play.channels = attr->channels;
	//info.play.buffer_size = 1024;
	info.play.encoding = AUDIO_ENCODING_LINEAR;
	//info.play.port = AUDIO_SPEAKER;
	//info.play.gain = 110;

// Write configuration
	if (ioctl (fd, AUDIO_SETINFO, &info) < 0) {
		fprintf (stderr, "%s: Writing audio config block\n", strerror (errno));
		return -1;
        }

	return 0;
}


/**
 * play the sample to the already opened file descriptor
 **/

static int _audio_sys_write (const void *buf, size_t num_bytes)
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
                "sys",
                &audio_sys);

        return 0;
}


/**
 * Cleanup Plugin.
 **/

void plugin_exit (void)
{
}
