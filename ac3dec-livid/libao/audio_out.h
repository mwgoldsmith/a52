/*
 *
 *  audio_out.h 
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

#ifndef __AUDIO_OUT_H__
#define __AUDIO_OUT_H__

#include <inttypes.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ao_info_s
{
	/* driver name ("OSS Audio driver") */
	const char *name;
	/* short name (for config strings) ("oss") */
	const char *short_name;
	/* author ("Aaron Holtzman <aholtzma@ess.engr.uvic.ca>") */
	const char *author;
	/* any additional comments */
	const char *comment;
} ao_info_t;

typedef struct ao_functions_s
{
	int (*open)(uint32_t bits, uint32_t rate, uint32_t channels);
	void (*play)(int16_t* output_samples, size_t num_bytes);
	void (*close)(void);
	const ao_info_t* (*get_info)(void);
} ao_functions_t;

extern ao_functions_t* audio_out_drivers[];

#ifdef __cplusplus
}
#endif

#endif
