//PLUGIN_INFO(INFO_NAME, "Null audio driver output");
//PLUGIN_INFO(INFO_AUTHOR, "Aaron Holtzman <aholtzma@ess.engr.uvic.ca>");

/*
 *
 *  audio_esd.c
 *    
 *	Copyright (C) Thomas Mirlacher
 *
 * GPL ...
 *
 */


#include <linux/soundcard.h>
#include <esd.h>
#include <oms/log.h>
#include <oms/plugin/output_audio.h>


static int _audio_esd_open (plugin_t *plugin, void *name);
static int _audio_esd_close (plugin_t *plugin);
static int _audio_esd_setup (plugin_output_audio_attr_t *attr);
static int _audio_esd_write (const void *buf, size_t num_bytes);

struct {
	int fd;
	char *server;
} _audio_esd_priv = {
	fd:	-1,
	server:	 0,
};


static plugin_output_audio_t audio_null = {
        open:		_audio_esd_open,
        close:		_audio_esd_close,
        setup:		_audio_esd_setup,
        write:		_audio_esd_write
};


/**
 * open device
 **/

static int _audio_esd_open (plugin_t *plugin, void *name)
{
        return 0;
}


/**
 *
 **/

static int _audio_esd_close (struct plugin_s *plugin)
{
	if (_audio_esd_priv.fd >= 0)
		esd_close (_audio_esd_priv.fd);

	_audio_esd_priv.fd = -1;

        return 0;
}


/**
 *
 **/

static int _audio_esd_setup (plugin_output_audio_attr_t *attr)
{
	esd_format_t format = ESD_STREAM | ESD_PLAY;
	uint32_t rate;
	esd_format_t fmt;

//	if ((_audio_esd_priv.fd = esd_open_sound (_audio_esd_priv.server))<0)
//		return -1;

	format |= ESD_BITS16 | ESD_STEREO;

	_audio_esd_priv.fd = esd_play_stream_fallback (format, attr->rate, _audio_esd_priv.server, "oms");

	return 0;
}


/**
 *
 **/

static int _audio_esd_write (const void *buf, size_t num_bytes)
{
	write (_audio_esd_priv.fd, buf, num_bytes);
	return 0;
}


/**
 * Initialize Plugin.
 **/

int plugin_init (char *whoami)
{
	pluginRegister (whoami,
		PLUGIN_ID_OUTPUT_AUDIO,
		"esd",
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

