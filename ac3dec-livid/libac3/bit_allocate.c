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



static inline int16_t logadd(int16_t a,int16_t  b);
static int16_t calc_lowcomp(int16_t a,int16_t b0,int16_t b1,int16_t bin);
static inline uint16_t min(int16_t a,int16_t b);
static inline uint16_t max(int16_t a,int16_t b);
static void ba_compute_psd(int16_t start, int16_t end, int16_t exps[], 
		int16_t psd[], int16_t bndpsd[]);

static void ba_compute_excitation(int16_t start, int16_t end,int16_t fgain,
		int16_t fastleak, int16_t slowleak, int16_t is_lfe, int16_t bndpsd[],
		int16_t excite[]);
static void ba_compute_mask(int16_t start, int16_t end, uint16_t fscod,
		uint16_t deltbae, uint16_t deltnseg, uint16_t deltoffst[], uint16_t deltba[],
		uint16_t deltlen[], int16_t excite[], int16_t mask[]);
static void ba_compute_bap(int16_t start, int16_t end, int16_t snroffset,
		int16_t psd[], int16_t mask[], int16_t bap[]);

/* Misc LUTs for bit allocation process */

static int16_t slowdec[]  = { 0x0f,  0x11,  0x13,  0x15  };
static int16_t fastdec[]  = { 0x3f,  0x53,  0x67,  0x7b  };
static int16_t slowgain[] = { 0x540, 0x4d8, 0x478, 0x410 };
static int16_t dbpbtab[]  = { 0x000, 0x700, 0x900, 0xb00 };

static uint16_t floortab[] = { 0x2f0, 0x2b0, 0x270, 0x230, 0x1f0, 0x170, 0x0f0, 0xf800 };
static int16_t fastgain[] = { 0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380, 0x400  };


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

static int16_t hth[][50] = {{ 0x04d0, 0x04d0, 0x0440, 0x0400, 0x03e0, 0x03c0, 0x03b0, 0x03b0,  
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
static int16_t psd[256];
static int16_t bndpsd[256];
static int16_t excite[256];
static int16_t mask[256];


/**
 *
 **/

static inline uint16_t max(int16_t a,int16_t b)
{
	return (a > b ? a : b);
}


/**
 *
 **/
	
static inline uint16_t min(int16_t a,int16_t b)
{
	return (a < b ? a : b);
}


/**
 *
 **/

static inline int16_t logadd(int16_t a,int16_t  b) 
{ 
	int16_t c;
	int16_t address;

	c = a - b; 
	address = min((abs(c) >> 1), 255); 
	
	if (c >= 0) 
		return(a + latab[address]); 
	else 
		return(b + latab[address]); 
}


/**
 *
 **/

void bit_allocate(uint16_t fscod, bsi_t *bsi, audblk_t *audblk)
{
	uint16_t i;
	int16_t fgain;
	int16_t snroffset;
	int16_t start;
	int16_t end;
	int16_t fastleak;
	int16_t slowleak;

	/* Only perform bit_allocation if the exponents have changed or we
	 * have new sideband information */
	if (audblk->chexpstr[0]  == 0 && audblk->chexpstr[1] == 0 &&
			audblk->chexpstr[2]  == 0 && audblk->chexpstr[3] == 0 &&
			audblk->chexpstr[4]  == 0 && audblk->cplexpstr   == 0 &&
			audblk->lfeexpstr    == 0 && audblk->baie        == 0 &&
			audblk->snroffste    == 0 && audblk->deltbaie    == 0)
		return;

	/* Do some setup before we do the bit alloc */
	sdecay = slowdec[audblk->sdcycod]; 
	fdecay = fastdec[audblk->fdcycod];
	sgain = slowgain[audblk->sgaincod]; 
	dbknee = dbpbtab[audblk->dbpbcod]; 
	floor = floortab[audblk->floorcod]; 

	/* if all the SNR offset constants are zero then the whole block is zero */
	if(!audblk->csnroffst    && !audblk->fsnroffst[0] && 
		 !audblk->fsnroffst[1] && !audblk->fsnroffst[2] && 
		 !audblk->fsnroffst[3] && !audblk->fsnroffst[4] &&
		 !audblk->cplfsnroffst && !audblk->lfefsnroffst) {
		memset(audblk->fbw_bap,0,sizeof(uint16_t) * 256 * 5);
		memset(audblk->cpl_bap,0,sizeof(uint16_t) * 256);
		memset(audblk->lfe_bap,0,sizeof(uint16_t) * 7);
		return;
	}
		 

	for(i = 0; i < bsi->nfchans; i++)
	{
		start = 0;
		end = audblk->endmant[i] ; 
		fgain = fastgain[audblk->fgaincod[i]]; 
		snroffset = (((audblk->csnroffst - 15) << 4) + audblk->fsnroffst[i]) << 2 ;
		fastleak = 0;
		slowleak = 0;

		ba_compute_psd(start, end, audblk->fbw_exp[i], psd, bndpsd);

		ba_compute_excitation(start, end , fgain, fastleak, slowleak, 0, bndpsd, excite);

		ba_compute_mask(start, end, fscod, audblk->deltbae[i], audblk->deltnseg[i], 
				audblk->deltoffst[i], audblk->deltba[i], audblk->deltlen[i], excite, mask);

		ba_compute_bap(start, end, snroffset, psd, mask, audblk->fbw_bap[i]);
	}

	if(audblk->cplinu) {
		start = audblk->cplstrtmant; 
		end = audblk->cplendmant; 
		fgain = fastgain[audblk->cplfgaincod];
		snroffset = (((audblk->csnroffst - 15) << 4) + audblk->cplfsnroffst) << 2 ;
		fastleak = (audblk->cplfleak << 8) + 768; 
		slowleak = (audblk->cplsleak << 8) + 768;

		ba_compute_psd(start, end, audblk->cpl_exp, psd, bndpsd);

		ba_compute_excitation(start, end , fgain, fastleak, slowleak, 0, bndpsd, excite);

		ba_compute_mask(start, end, fscod, audblk->cpldeltbae, audblk->cpldeltnseg, 
				audblk->cpldeltoffst, audblk->cpldeltba, audblk->cpldeltlen, excite, mask);

		ba_compute_bap(start, end, snroffset, psd, mask, audblk->cpl_bap);
	}

	if(bsi->lfeon) {
		start = 0;
		end = 7;
		fgain = fastgain[audblk->lfefgaincod];
		snroffset = (((audblk->csnroffst - 15) << 4) + audblk->lfefsnroffst) << 2 ;
		fastleak = 0;
		slowleak = 0;

		ba_compute_psd(start, end, audblk->lfe_exp, psd, bndpsd);

		ba_compute_excitation(start, end , fgain, fastleak, slowleak, 1, bndpsd, excite);

		/* Perform no delta bit allocation for lfe */
		ba_compute_mask(start, end, fscod, 2, 0, 0, 0, 0, excite, mask);

		ba_compute_bap(start, end, snroffset, psd, mask, audblk->lfe_bap);
	}
}


/**
 *
 **/

static void ba_compute_psd(int16_t start, int16_t end, int16_t exps[], 
		int16_t psd[], int16_t bndpsd[])
{
	int bin,i,j,k;
	int16_t lastbin = 0;
	
	/* Map the exponents into dBs */
	for (bin=start; bin<end; bin++) { 
		psd[bin] = (3072 - (exps[bin] << 7)); 
	}

	/* Integrate the psd function over each bit allocation band */
	j = start; 
	k = masktab[start]; 
	
	do { 
		lastbin = min(bndtab[k] + bndsz[k], end); 
		bndpsd[k] = psd[j]; 
		j++; 

		for (i = j; i < lastbin; i++) { 
			bndpsd[k] = logadd(bndpsd[k], psd[j]);
			j++; 
		} 
		
		k++; 
	} while (end > lastbin);
}


/**
 *
 **/

static void ba_compute_excitation(int16_t start, int16_t end,int16_t fgain,
		int16_t fastleak, int16_t slowleak, int16_t is_lfe, int16_t bndpsd[],
		int16_t excite[])
{
	int bin;
	int16_t bndstrt;
	int16_t bndend;
	int16_t lowcomp = 0;
	int16_t begin = 0;

	/* Compute excitation function */
	bndstrt = masktab[start]; 
	bndend = masktab[end - 1] + 1; 
	
	if (bndstrt == 0) { /* For fbw and lfe channels */ 
		lowcomp = calc_lowcomp(lowcomp, bndpsd[0], bndpsd[1], 0); 
		excite[0] = bndpsd[0] - fgain - lowcomp; 
		lowcomp = calc_lowcomp(lowcomp, bndpsd[1], bndpsd[2], 1);
		excite[1] = bndpsd[1] - fgain - lowcomp; 
		begin = 7 ; 
		
// Note: Do not call calc_lowcomp() for the last band of the lfe channel,(bin=6)
		for (bin = 2; bin < 7; bin++) { 
			if (!(is_lfe && (bin == 6)))
				lowcomp = calc_lowcomp(lowcomp, bndpsd[bin], bndpsd[bin+1], bin); 
			fastleak = bndpsd[bin] - fgain; 
			slowleak = bndpsd[bin] - sgain; 
			excite[bin] = fastleak - lowcomp; 
			
			if (!(is_lfe && (bin == 6))) {
				if (bndpsd[bin] <= bndpsd[bin+1]) {
					begin = bin + 1 ; 
					break; 
				} 
			}
		} 
		
		for (bin = begin; bin < min(bndend, 22); bin++) { 
			if (!(is_lfe && (bin == 6)))
				lowcomp = calc_lowcomp(lowcomp, bndpsd[bin], bndpsd[bin+1], bin); 
			fastleak -= fdecay ; 
			fastleak = max(fastleak, bndpsd[bin] - fgain); 
			slowleak -= sdecay ; 
			slowleak = max(slowleak, bndpsd[bin] - sgain); 
			excite[bin] = max(fastleak - lowcomp, slowleak); 
		} 
		begin = 22; 
	} 
	else /* For coupling channel */ 
		begin = bndstrt; 

	for (bin = begin; bin < bndend; bin++) { 
		fastleak -= fdecay; 
		fastleak = max(fastleak, bndpsd[bin] - fgain); 
		slowleak -= sdecay; 
		slowleak = max(slowleak, bndpsd[bin] - sgain); 
		excite[bin] = max(fastleak, slowleak) ; 
	} 
}


/**
 *
 **/

static void ba_compute_mask(int16_t start, int16_t end, uint16_t fscod,
		uint16_t deltbae, uint16_t deltnseg, uint16_t deltoffst[], uint16_t deltba[],
		uint16_t deltlen[], int16_t excite[], int16_t mask[])
{
	int bin,k;
	int16_t bndstrt;
	int16_t bndend;
	int16_t delta;

	bndstrt = masktab[start]; 
	bndend = masktab[end - 1] + 1; 

	/* Compute the masking curve */

	for (bin = bndstrt; bin < bndend; bin++) { 
		if (bndpsd[bin] < dbknee) { 
			excite[bin] += ((dbknee - bndpsd[bin]) >> 2); 
		} 
		mask[bin] = max(excite[bin], hth[fscod][bin]);
	}
	
	/* Perform delta bit modulation if necessary */
	if ((deltbae == DELTA_BIT_REUSE) || (deltbae == DELTA_BIT_NEW)) { 
		int16_t band = 0; 
		int16_t seg = 0; 
		
		for (seg = 0; seg < deltnseg+1; seg++) { 
			band += deltoffst[seg]; 

			if (deltba[seg] >= 4) { 
				delta = (deltba[seg] - 3) << 7;
			} else { 
				delta = (deltba[seg] - 4) << 7;
			} 
			
			for (k = 0; k < deltlen[seg]; k++) { 
				mask[band] += delta; 
				band++; 
			} 
		} 
	}
}


/**
 *
 **/

static void ba_compute_bap(int16_t start, int16_t end, int16_t snroffset,
		int16_t psd[], int16_t mask[], int16_t bap[])
{
	int i,j,k;
	int16_t lastbin = 0;
	int16_t address = 0;

	/* Compute the bit allocation pointer for each bin */
	i = start; 
	j = masktab[start]; 

	do { 
		lastbin = min(bndtab[j] + bndsz[j], end); 
		mask[j] -= snroffset; 
		mask[j] -= floor; 
		
		if (mask[j] < 0) 
			mask[j] = 0; 

		mask[j] &= 0x1fe0;
		mask[j] += floor; 
		for (k = i; k < lastbin; k++) { 
			address = (psd[i] - mask[j]) >> 5; 
			address = min(63, max(0, address)); 
			bap[i] = baptab[address]; 
			i++; 
		} 
		j++; 
	} while (end > lastbin);
}


/**
 *
 **/

static int16_t calc_lowcomp (int16_t a, int16_t b0, int16_t b1, int16_t bin) 
{ 

	if (bin < 7) { 
		if ((b0 + 256) == b1)
			a = 384; 
	 	else if (b0 > b1) 
			a = max(0, a - 64); 
	} else if (bin < 20) { 
		if ((b0 + 256) == b1) 
			a = 320; 
		else if (b0 > b1) 
			a = max(0, a - 64) ; 
	} else  
		a = max(0, a - 128); 
	
	return(a);
}

