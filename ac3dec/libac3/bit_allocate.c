/* 
 *  bit_allocate.c
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

#include <stdlib.h>
#include <string.h>
#include "ac3.h"
#include "ac3_internal.h"
#include "bit_allocate.h"

static int16_t bndtab[] = {  0,  1,  2,   3,   4,   5,   6,   7,   8,   9, 
			     10, 11, 12,  13,  14,  15,  16,  17,  18,  19,
			     20, 21, 22,  23,  24,  25,  26,  27,  28,  31,
			     34, 37, 40,  43,  46,  49,  55,  61,  67,  73,
			     79, 85, 97, 109, 121, 133, 157, 181, 205, 229 };

static int16_t bndsz[]  = { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
			    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
			    1,  1,  1,  1,  1,  1,  1,  1,  3,  3,
			    3,  3,  3,  3,  3,  6,  6,  6,  6,  6,
			    6, 12, 12, 12, 12, 24, 24, 24, 24, 24 };

static int16_t masktab[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
			     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 28, 28, 29,
			     29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34,
			     34, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 37, 37, 37,
			     37, 37, 37, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 40,
			     40, 40, 40, 40, 40, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
			     41, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43,
			     43, 43, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44, 44,
			     44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
			     45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 46, 46, 46,
			     46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
			     46, 46, 46, 46, 46, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
			     47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48,
			     48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
			     48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
			     49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,  0,  0,  0 };


static int16_t latab[] = { 0x0040, 0x003f, 0x003e, 0x003d, 0x003c, 0x003b, 0x003a, 0x0039,
			   0x0038, 0x0037, 0x0036, 0x0035, 0x0034, 0x0034, 0x0033, 0x0032,
			   0x0031, 0x0030, 0x002f, 0x002f, 0x002e, 0x002d, 0x002c, 0x002c,
			   0x002b, 0x002a, 0x0029, 0x0029, 0x0028, 0x0027, 0x0026, 0x0026,
			   0x0025, 0x0024, 0x0024, 0x0023, 0x0023, 0x0022, 0x0021, 0x0021,
			   0x0020, 0x0020, 0x001f, 0x001e, 0x001e, 0x001d, 0x001d, 0x001c,
			   0x001c, 0x001b, 0x001b, 0x001a, 0x001a, 0x0019, 0x0019, 0x0018,
			   0x0018, 0x0017, 0x0017, 0x0016, 0x0016, 0x0015, 0x0015, 0x0015,
			   0x0014, 0x0014, 0x0013, 0x0013, 0x0013, 0x0012, 0x0012, 0x0012,
			   0x0011, 0x0011, 0x0011, 0x0010, 0x0010, 0x0010, 0x000f, 0x000f,
			   0x000f, 0x000e, 0x000e, 0x000e, 0x000d, 0x000d, 0x000d, 0x000d,
			   0x000c, 0x000c, 0x000c, 0x000c, 0x000b, 0x000b, 0x000b, 0x000b,
			   0x000a, 0x000a, 0x000a, 0x000a, 0x000a, 0x0009, 0x0009, 0x0009,
			   0x0009, 0x0009, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008,
			   0x0007, 0x0007, 0x0007, 0x0007, 0x0007, 0x0007, 0x0006, 0x0006,
			   0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0005, 0x0005,
			   0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0004, 0x0004,
			   0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004,
			   0x0004, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003,
			   0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0002,
			   0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
			   0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
			   0x0002, 0x0002, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
			   0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
			   0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
			   0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
			   0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			   0x0000, 0x0000, 0x0000, 0x0000};

static int16_t hth_[][50] = {{ 0x04d0, 0x04d0, 0x0440, 0x0400, 0x03e0, 0x03c0, 0x03b0, 0x03b0,  
			      0x03a0, 0x03a0, 0x03a0, 0x03a0, 0x03a0, 0x0390, 0x0390, 0x0390,  
			      0x0380, 0x0380, 0x0370, 0x0370, 0x0360, 0x0360, 0x0350, 0x0350,  
			      0x0340, 0x0340, 0x0330, 0x0320, 0x0310, 0x0300, 0x02f0, 0x02f0,
			      0x02f0, 0x02f0, 0x0300, 0x0310, 0x0340, 0x0390, 0x03e0, 0x0420,
			      0x0460, 0x0490, 0x04a0, 0x0460, 0x0440, 0x0440, 0x0520, 0x0800,
			      0x0840, 0x0840 },
			    { 0x04f0, 0x04f0, 0x0460, 0x0410, 0x03e0, 0x03d0, 0x03c0, 0x03b0, 
			      0x03b0, 0x03a0, 0x03a0, 0x03a0, 0x03a0, 0x03a0, 0x0390, 0x0390, 
			      0x0390, 0x0380, 0x0380, 0x0380, 0x0370, 0x0370, 0x0360, 0x0360, 
			      0x0350, 0x0350, 0x0340, 0x0340, 0x0320, 0x0310, 0x0300, 0x02f0, 
			      0x02f0, 0x02f0, 0x02f0, 0x0300, 0x0320, 0x0350, 0x0390, 0x03e0, 
			      0x0420, 0x0450, 0x04a0, 0x0490, 0x0460, 0x0440, 0x0480, 0x0630, 
			      0x0840, 0x0840 },
			    { 0x0580, 0x0580, 0x04b0, 0x0450, 0x0420, 0x03f0, 0x03e0, 0x03d0, 
			      0x03c0, 0x03b0, 0x03b0, 0x03b0, 0x03a0, 0x03a0, 0x03a0, 0x03a0, 
			      0x03a0, 0x03a0, 0x03a0, 0x03a0, 0x0390, 0x0390, 0x0390, 0x0390, 
			      0x0380, 0x0380, 0x0380, 0x0370, 0x0360, 0x0350, 0x0340, 0x0330, 
			      0x0320, 0x0310, 0x0300, 0x02f0, 0x02f0, 0x02f0, 0x0300, 0x0310, 
			      0x0330, 0x0350, 0x03c0, 0x0410, 0x0470, 0x04a0, 0x0460, 0x0440, 
			      0x0450, 0x04e0 }};


static int16_t baptab[] = { 0,  1,  1,  1,  1,  1,  2,  2,  3,  3,  3,  4,  4,  5,  5,  6,
			    6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 
			    10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14,
			    14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15 };

static int16_t sdecay;
static int16_t fdecay;
static int16_t sgain;
static int16_t dbknee;
static int16_t floor;

static inline uint16_t max (int16_t a, int16_t b)
{
    return (a > b ? a : b);
}
	
static inline uint16_t min (int16_t a, int16_t b)
{
    return (a < b ? a : b);
}

#include <stdio.h>

void bit_allocate(int fscod, audblk_t * audblk, ac3_ba_t * ba, uint16_t start,
		  uint16_t end, int16_t fastleak, int16_t slowleak,
		  uint8_t * exp, uint16_t * bap)
{
    static int16_t slowgain[] = { 0x540, 0x4d8, 0x478, 0x410 };
    static int16_t dbpbtab[]  = { 0x000, 0x700, 0x900, 0xb00 };
    static int16_t floortab[] = { 0x2f0, 0x2b0, 0x270, 0x230, 0x1f0, 0x170, 0x0f0, 0xf800 };

    int16_t fgain;
    int16_t snroffset;
    int i, j;
    int lowcomp;
    int8_t * deltba;
    int psd, mask;
    int16_t * hth;

    /* Do some setup before we do the bit alloc */
    sdecay = 15 + 2 * audblk->sdcycod;
    fdecay = 63 + 20 * audblk->fdcycod;
    sgain = slowgain[audblk->sgaincod];
    dbknee = dbpbtab[audblk->dbpbcod];
    floor = floortab[audblk->floorcod];

    fgain = (ba->fgaincod + 1) << 7;
    snroffset = 64 * audblk->csnroffst + 4 * ba->fsnroffst - 960;

    deltba = NULL;
    if ((ba->deltbae == DELTA_BIT_REUSE) || (ba->deltbae == DELTA_BIT_NEW))
	deltba = ba->deltba;

    hth = hth_[fscod];

    i = masktab[start];
    j = start;
    if (start == 0) {	// not the coupling channel
	lowcomp = 0;
	j = end - 1;
	do {
	    if (i < j) {
		if (exp[i+1] == exp[i] - 2)
		    lowcomp = 384;
		else if (lowcomp && (exp[i+1] > exp[i]))
		    lowcomp -= 64;
	    }
	    psd = 3072 - 128 * exp[i];
	    mask = psd - fgain - lowcomp;
	    if (psd < dbknee)
		mask += (dbknee - psd) >> 2;
	    if (mask < hth[i])
		mask = hth[i];
	    if (deltba != NULL)
		mask += deltba[i] << 7;
	    mask -= snroffset + floor;
	    mask = (mask < 0) ? 0 : (mask & 0x1fe0);
	    mask += floor;
	    mask = min (63, max (0, (psd - mask) >> 5));
	    bap[i++] = baptab[mask];
	} while ((i < 3) || ((i < 7) && (exp[i] > exp[i-1])));
	fastleak = psd - fgain;
	slowleak = psd - sgain;

	while (i < 7) {
	    if (i < j) {
		if (exp[i+1] == exp[i] - 2)
		    lowcomp = 384;
		else if (lowcomp && (exp[i+1] > exp[i]))
		    lowcomp -= 64;
	    }
	    psd = 3072 - 128 * exp[i];
	    fastleak -= fdecay;
	    if (fastleak < psd - fgain)
		fastleak = psd - fgain;
	    slowleak -= sdecay;
	    if (slowleak < psd - sgain)
		slowleak = psd - sgain;
	    mask = ((fastleak - lowcomp > slowleak) ?
		    fastleak - lowcomp : slowleak);
	    if (psd < dbknee)
		mask += (dbknee - psd) >> 2;
	    if (mask < hth[i])
		mask = hth[i];
	    if (deltba != NULL)
		mask += deltba[i] << 7;
	    mask -= snroffset + floor;
	    mask = (mask < 0) ? 0 : (mask & 0x1fe0);
	    mask += floor;
	    mask = min (63, max (0, (psd - mask) >> 5));
	    bap[i++] = baptab[mask];
	}

	if (end == 7)	// lfe channel
	    return;

	do {
	    if (exp[i+1] == exp[i] - 2)
		lowcomp = 320;
	    else if (lowcomp && (exp[i+1] > exp[i]))
		lowcomp -= 64;
	    psd = 3072 - 128 * exp[i];
	    fastleak -= fdecay;
	    if (fastleak < psd - fgain)
		fastleak = psd - fgain;
	    slowleak -= sdecay;
	    if (slowleak < psd - sgain)
		slowleak = psd - sgain;
	    mask = ((fastleak - lowcomp > slowleak) ?
		    fastleak - lowcomp : slowleak);
	    if (psd < dbknee)
		mask += (dbknee - psd) >> 2;
	    if (mask < hth[i])
		mask = hth[i];
	    if (deltba != NULL)
		mask += deltba[i] << 7;
	    mask -= snroffset + floor;
	    mask = (mask < 0) ? 0 : (mask & 0x1fe0);
	    mask += floor;
	    mask = min (63, max (0, (psd - mask) >> 5));
	    bap[i++] = baptab[mask];
	} while (i < 20);

	while (lowcomp > 128) {		// two iterations maximum
	    lowcomp -= 128;
	    psd = 3072 - 128 * exp[i];
	    fastleak -= fdecay;
	    if (fastleak < psd - fgain)
		fastleak = psd - fgain;
	    slowleak -= sdecay;
	    if (slowleak < psd - sgain)
		slowleak = psd - sgain;
	    mask = ((fastleak - lowcomp > slowleak) ?
		    fastleak - lowcomp : slowleak);
	    if (psd < dbknee)
		mask += (dbknee - psd) >> 2;
	    if (mask < hth[i])
		mask = hth[i];
	    if (deltba != NULL)
		mask += deltba[i] << 7;
	    mask -= snroffset + floor;
	    mask = (mask < 0) ? 0 : (mask & 0x1fe0);
	    mask += floor;
	    mask = min (63, max (0, (psd - mask) >> 5));
	    bap[i++] = baptab[mask];
	}
	j = i;
    }

    do {
	int startband, endband;

	startband = j;
	endband = min (bndtab[i] + bndsz[i], end);
	psd = 128 * exp[j++];
	while (j < endband) {
	    int next, delta;

	    next = 128 * exp[j++];
	    delta = next - psd;
	    switch (delta >> 9) {
	    case -6: case -5: case -4: case -3: case -2:
		psd = next;
		break;
	    case -1:
		psd = next - latab[(-delta) >> 1];
		break;
	    case 0:
		psd -= latab[delta >> 1];
		break;
	    }
	}
	psd = 3072 - psd;
	// psd max:3361
	fastleak -= fdecay;
	if (fastleak < psd - fgain)
	    fastleak = psd - fgain;
	slowleak -= sdecay;
	if (slowleak < psd - sgain)
	    slowleak = psd - sgain;
	mask = (fastleak > slowleak) ? fastleak : slowleak;
	if (psd < dbknee)
	    mask += (dbknee - psd) >> 2;
	if (mask < hth[i])
	    mask = hth[i];
	if (deltba != NULL)
	    mask += deltba[i] << 7;
	mask -= snroffset + floor;
	mask = (mask < 0) ? 0 : (mask & 0x1fe0);
	mask += floor;
	j = startband;
	do {
	    psd = 3072 - 128 * exp[j];
	    bap[j++] = baptab[min (63, max (0, (psd - mask) >> 5))];
	    // psd-mask max:5019=sgain-deltba+snroffset+31(and)
	    // psd-mask min:-4705=0-maxpsd+fgain-deltba+snroffset
	} while (j < endband);
	i++;
    } while (j < end);
}
