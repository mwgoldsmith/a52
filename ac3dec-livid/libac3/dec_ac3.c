//PLUGIN_INFO(INFO_NAME, "AC3 decoder");
//PLUGIN_INFO(INFO_AUTHOR, "Aaron Holtzman <aholtzma@ess.engr.uvic.ca>");

/* 
 *    decode.c
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
#include "config.h"
#endif 

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include <oms/oms.h>
#include <oms/plugin/codec_audio.h>

#include "ac3.h"

static int _ac3dec_open		(plugin_t *plugin, void *foo);
static int _ac3dec_close	(plugin_t *plugin);
static int _ac3dec_read		(plugin_t *plugin, buf_t *buf, buf_entry_t *buf_entry);

static plugin_codec_audio_t codec_ac3dec = {
	open:		_ac3dec_open,
	close:		_ac3dec_close,
	read:		_ac3dec_read,
};


static int _ac3dec_open (plugin_t *plugin, void *foo)
{
	ac3dec_init ();

	return 0;
}


static int _ac3dec_close (plugin_t *plugin)
{
	return 0;
}


static int _ac3dec_read (plugin_t *plugin, buf_t *buf, buf_entry_t *buf_entry)
{
	return ac3dec_decode_data (codec_ac3dec.output, buf, buf_entry);
}

/*****************************************/


int plugin_init (char *whoami)
{
	pluginRegister (whoami,
		PLUGIN_ID_CODEC_AUDIO,
		"ac3",
		NULL,
		NULL,
		&codec_ac3dec);

	return 0;
}


void plugin_exit (void)
{
}

