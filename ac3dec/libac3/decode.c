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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <inttypes.h>
#include "ac3.h"
#include "ac3_internal.h"
#include "bitstream.h"
#include "imdct.h"
#include "bit_allocate.h"
#include "parse.h"

//our global config structure
uint32_t error_flag = 0;

static audblk_t audblk;
static ac3_state_t state;

//the floating point samples for one audblk
stream_samples_t samples;

//the integer samples for the entire frame (with enough space for 2 ch out)
//if this size change, be sure to change the size when muting
static int16_t s16_samples[2 * 6 * 256];

void
ac3_init(void)
{
    imdct_init();
}

int ac3_frame_length(uint8_t * buf)
{
    int dummy;

    return parse_syncinfo (buf, &dummy, &dummy);
}

static int16_t blah (float f)
{
#if 0
    if (f > 32767)
	return 32767;
    else if (f < -32768)
	return -32768;
    else
#endif
	return f;
}

static void float_to_int (float * f, int16_t * s16) 
{
    int i;

    for (i = 0; i < 256; i++) {
	s16[2*i] = blah (f[i]);
	s16[2*i+1] = blah (f[i+256]);
    }
}

ac3_frame_t *
ac3_decode_frame(uint8_t * buf)
{
    static ac3_frame_t frame;
    uint32_t i;
    int dummy;

    if (!parse_syncinfo (buf, &frame.sampling_rate, &dummy))
	goto error;

    frame.audio_data = s16_samples;

    if (parse_bsi (&state, buf))
	goto error;

    for (i = 0; i < 6; i++) {
	if (parse_audblk (&state, &audblk))
	    goto error;

	float_to_int (*samples, s16_samples + i * 512);
    }

    return &frame;	

error:
    printf ("error\n");
    memset (s16_samples, 0, sizeof(int16_t) * 256 * 2 * 6);	//mute frame

    error_flag = 0;
    return &frame;
}
