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
#include <math.h>
#include "ac3.h"
#include "ac3_internal.h"
#include "bitstream.h"
#include "imdct.h"
#include "bit_allocate.h"
#include "parse.h"

void
ac3_init(void)
{
    imdct_init();
}

static int16_t blah (int32_t i)
{
    if (i > 0x43c07fff)
	return 32767;
    else if (i < 0x43bf8000)
	return -32768;
    else
	return i - 0x43c00000;
}

void float_to_int (float * _f, int16_t * s16) 
{
    int i;
    int32_t * f = (int32_t *) _f;	// XXX assumes IEEE float format

#if 0
    i = *_f = 0;
    *_f += 384;
    printf ("%x %x\n", i, *f);

    i = *_f = 1.0 / 32768;
    *_f += 384;
    printf ("%x %x\n", i, *f);

    i = *_f = -1.0 / 32768;
    *_f += 384;
    printf ("%x %x\n", i, *f);
#endif

    for (i = 0; i < 256; i++) {
	s16[2*i] = blah (f[i]);
	s16[2*i+1] = blah (f[i+256]);
    }
}
