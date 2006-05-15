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
#include "ac3_internal.h"
#include "bitstream.h"
#include "downmix.h"
#include "imdct.h"
#include "exponent.h"
#include "coeff.h"
#include "bit_allocate.h"
#include "parse.h"
#include "crc.h"
#include "stats.h"
#include "rematrix.h"
#include "sanity_check.h"
#include "debug.h"

//our global config structure
ac3_config_t ac3_config;
uint32_t error_flag = 0;

static audblk_t audblk;
static bsi_t bsi;
static syncinfo_t syncinfo;
static uint32_t is_output_initialized = 0;

//the floating point samples for one audblk
static stream_samples_t samples;

//the integer samples for the entire frame (with enough space for 2 ch out)
//if this size change, be sure to change the size when muting
static int16_t s16_samples[2 * 6 * 256] __attribute__ ((aligned(16)));

// downmix stuff
static float cmixlev_lut[4] = { 0.707, 0.595, 0.500, 0.707 };
static float smixlev_lut[4] = { 0.707, 0.500, 0.0   , 0.500 };
static dm_par_t dm_par; 

//Storage for the syncframe
#define BUFFER_MAX_SIZE 4096
static uint8_t buffer[BUFFER_MAX_SIZE];
static uint32_t buffer_size = 0;;


static int _ac3dec_open (plugin_t *plugin, void *foo);
static int _ac3dec_close (plugin_t *plugin);
static int _ac3dec_read	(plugin_codec_audio_t *plugin, buf_t *buf, buf_entry_t *buf_entry);

static plugin_codec_audio_t codec_ac3dec = {
	open:		_ac3dec_open,
	close:		_ac3dec_close,
	read:		_ac3dec_read,
};


/**
 *
 **/

static uint32_t _decode_buffer_syncframe (syncinfo_t *syncinfo, uint8_t **start, uint8_t *end)
{
	uint8_t *cur = *start;
	uint16_t syncword = syncinfo->syncword;
	uint32_t ret = 0;

	// 
	// Find an ac3 sync frame.
	// 
	while (syncword != 0x0b77) {
		if(cur >= end)
			goto done;
		syncword = (syncword << 8) + *cur++;
	}

	//need the next 3 bytes to decide how big the frame is
	while (buffer_size < 3) {
		if(cur >= end)
			goto done;

		buffer[buffer_size++] = *cur++;
	}
	
	parse_syncinfo (syncinfo, buffer);

	while (buffer_size < syncinfo->frame_size * 2 - 2) {
		if(cur >= end)
			goto done;

		buffer[buffer_size++] = *cur++;
	}

#if !defined (__OPTIMIZE__)
	// Check the crc over the entire frame 
	crc_init();
	crc_process_frame(buffer,syncinfo->frame_size * 2 - 2);

	if (!crc_validate ()) {
		error_flag = 1;
		fprintf (stderr,"** CRC failed - skipping frame **\n");
		crc_init ();
		goto done;
	}
#endif

	//
	//if we got to this point, we found a valid ac3 frame to decode
	//

	bitstream_init (buffer);
	//get rid of the syncinfo struct as we already parsed it
	bitstream_get (24);

	//reset the syncword for next time
	syncword = 0xffff;
	buffer_size = 0;
	ret = 1;

done:
	syncinfo->syncword = syncword;
	*start = cur;
	return ret;
}


/**
 *
 **/

static void _decode_mute (void)
{
	//mute the frame
	memset (s16_samples, 0, sizeof(int16_t) * 256 * 2 * 6);
	error_flag = 0;
}


/**
 *
 **/

static int _ac3dec_open (plugin_t *plugin, void *foo)
{
// FIXME - don't do that statically here
	ac3_config.num_output_ch = 2;
	ac3_config.flags = 0;
// 
	imdct_init ();

#if !defined (__OPTIMIZE__)
	sanity_check_init (&syncinfo, &bsi, &audblk);
#endif

	return 0;
}


/**
 *
 **/

static int _ac3dec_close (plugin_t *plugin)
{
	return 0;
}


/**
 *
 **/

static int _ac3dec_read (plugin_codec_audio_t *plugin, buf_t *buf, buf_entry_t *buf_entry)
{
	uint32_t i;
	uint8_t *data_start = buf_entry->data;
	uint8_t *data_end = data_start+buf_entry->data_len;

	while (_decode_buffer_syncframe (&syncinfo,&data_start,data_end)) {
		parse_bsi(&bsi);

               // compute downmix parameters
               // downmix to tow channels for now
               dm_par.clev = 0.0; dm_par.slev = 0.0; dm_par.unit = 1.0;

		if (bsi.acmod & 0x1)		// have center
			dm_par.clev = cmixlev_lut[bsi.cmixlev];

		if (bsi.acmod & 0x4)		// have surround channels
			dm_par.slev = smixlev_lut[bsi.surmixlev];

		dm_par.unit /= 1.0 + dm_par.clev + dm_par.slev;
		dm_par.clev *= dm_par.unit;
		dm_par.slev *= dm_par.unit;

		for(i=0; i < 6; i++) {
			//Initialize freq/time sample storage
			memset (samples, 0, sizeof(float) * 256 * (bsi.nfchans + bsi.lfeon));

			// Extract most of the audblk info from the bitstream
			// (minus the mantissas 
			parse_audblk(&bsi,&audblk);

			// Take the differential exponent data and turn it into
			// absolute exponents 
			exponent_unpack(&bsi,&audblk); 
			if(error_flag)
				goto error;

			// Figure out how many bits per mantissa 
			bit_allocate(syncinfo.fscod,&bsi,&audblk);

			// Extract the mantissas from the stream and
			// generate floating point frequency coefficients
			coeff_unpack (&bsi, &audblk, samples);
			if(error_flag)
				goto error;

			if(bsi.acmod == 0x2)
				rematrix (&audblk, samples);

			// Convert the frequency samples into time samples 
			imdct (&bsi,&audblk,samples, &s16_samples[i * 2 * 256], &dm_par);

			// Downmix into the requested number of channels
			// and convert floating point to int16_t
			// downmix (&bsi, samples, &s16_samples[i * 2 * 256]);

#if !defined (__OPTIMIZE__)
			if (sanity_check (&syncinfo,&bsi,&audblk))
				goto error;
#endif

			continue;
		}

		if(!is_output_initialized) {
			plugin_output_audio_attr_t attr;
			attr.bits = 16;
			attr.rate = syncinfo.sampling_rate;
			attr.channels = 2;

			codec_ac3dec.output->setup (&attr);

			is_output_initialized = 1;
		}

		codec_ac3dec.output->write (s16_samples, 256 * 6 * 2 * 2);

		continue;
error:
		_decode_mute();
	}

	return 0;	
}

/*****************************************/


/**
 * Initialize Plugin.
 **/


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


/**
 * Cleanup Plugin.
 **/

void plugin_exit (void)
{
}

