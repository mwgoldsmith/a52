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

#include "audio_out.h"
#include "audio_out_internal.h"

static ao_info_t ao_info =
{
	"Null output ",
	"null",
	"Aaron Holtzman <aholtzma@ess.engr.uvic.ca>",
	""
};

static uint_32
ao_open(uint_32 bits,uint_32 rate,uint_32 channels)
{
	//do nothing
	return 0;
}

static void
ao_close(void)
{
}

static void
ao_play(sint_16 *foo,uint_32 bar)
{
	//do nothing
}

static const ao_info_t*
ao_get_info(void)
{
	return &ao_info;
}

LIBAO_EXTERN(null);
