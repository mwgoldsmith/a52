/* 
 *    dither.h
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
 */


extern uint16_t lfsr_state;
extern const uint16_t dither_lut[256]; 

static inline int16_t dither_gen(void)
{
    int16_t state;

    state = dither_lut[lfsr_state >> 8] ^ (lfsr_state << 8);
	
    lfsr_state = (uint16_t) state;

    return ((state * (int32_t) (0.707106 * 256.0))>>8);
}
