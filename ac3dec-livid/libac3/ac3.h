/* 
 *    ac3.h
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

#ifndef __AC3_H__
#define __AC3_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __OMS__
#include <oms/plugin/output_audio.h>
#ifndef ao_functions_t
#define ao_functions_t plugin_output_audio_t
#endif
#else
//FIXME normally I wouldn't nest includes, but we'll leave this here until I get
//another chance to move things around
#include "audio_out.h"
#endif

#include <inttypes.h>

#define AC3_DOLBY_SURR_ENABLE (1<<0)
#define AC3_3DNOW_ENABLE      (1<<1)
#define AC3_MMX_ENABLE        (1<<2)
#define AC3_ALTIVEC_ENABLE    (1<<3)

typedef struct ac3_config_s {
	// Bit flags that enable various things
	uint32_t flags;
	// Number of discrete channels in final output (for downmixing)
	uint16_t num_output_ch;
	// Which channel of a dual mono stream to select
	uint16_t dual_mono_ch_sel;
} ac3_config_t;

void ac3_init (void);
size_t ac3_decode_data (ac3_config_t *, ao_functions_t *, uint8_t * data_start, uint8_t * data_end);
//void ac3_close (ao_functions_t *);
//void ac3_drop (int flag);
//void ac3_output_init (int flag);

#ifdef __cplusplus
}
#endif

#endif
