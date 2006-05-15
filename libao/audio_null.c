//PLUGIN_INFO(INFO_NAME, "Null audio driver output");
//PLUGIN_INFO(INFO_AUTHOR, "Aaron Holtzman <aholtzma@ess.engr.uvic.ca>");

/*
 *
 *  audio_out_null.c
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


#include <oms/oms.h>
#include <oms/plugin/output_audio.h>


static int _audio_null_open (plugin_t *plugin, void *name);
static int _audio_null_close (plugin_t *plugin);
static int _audio_null_setup (plugin_output_audio_attr_t *attr);
static int _audio_null_write (const void *buf, size_t num_bytes);

static plugin_output_audio_t audio_null = {
        open:		_audio_null_open,
        close:		_audio_null_close,
        setup:		_audio_null_setup,
        write:		_audio_null_write
};


/**
 * open device
 **/

static int _audio_null_open (plugin_t *plugin, void *name)
{
	//do nothing
        return 0;
}


/**
 *
 **/

static int _audio_null_close (struct plugin_s *plugin)
{
        return 0;
}


/**
 *
 **/

static int _audio_null_setup (plugin_output_audio_attr_t *attr)
{
	//do nothing
	return 0;
}


/**
 *
 **/

static int _audio_null_write (const void *buf, size_t num_bytes)
{
	//do nothing
	return 0;
}


/**
 * Initialize Plugin.
 **/

int plugin_init (char *whoami)
{
	pluginRegister (whoami,
		PLUGIN_ID_OUTPUT_AUDIO,
		"null",
		NULL,
		NULL,
		&audio_null);

	return 0;
}


/**
 * Cleanup Plugin.
 **/

void plugin_exit (void)
{
}

