/* 
 *  bit_allocate.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include "ac3.h"

/* Misc LUTs for bit allocation process */

sint_16 slowdec[]  = { 0x0f,  0x11,  0x13,  0x15  };
sint_16 fastdec[]  = { 0x3f,  0x53,  0x67,  0x7b  };
sint_16 slowgain[] = { 0x540, 0x4d8, 0x478, 0x410 };
sint_16 dbpbtab[]  = { 0x000, 0x700, 0x900, 0xb00 };

sint_16 floortab[] = { 0x2f0, 0x2b0, 0x270, 0x230, 0x1f0, 0x170, 0x0f0, 0xf800 };
sint_16 fastgain[] = { 0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380, 0x400  };


sint_16 bndtab[] = {  0,  1,  2,   3,   4,   5,   6,   7,   8,   9, 
                     10, 11, 12,  13,  14,  15,  16,  17,  18,  19,
                     20, 21, 22,  23,  24,  25,  26,  27,  28,  31,
                     34, 37, 40,  43,  46,  49,  55,  61,  67,  73,
                     79, 85, 97, 109, 121, 133, 157, 181, 205, 229 };

sint_16 bndsz[]  = { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
                     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
                     1,  1,  1,  1,  1,  1,  1,  1,  3,  3,
                     3,  3,  3,  3,  3,  6,  6,  6,  6,  6,
                     6, 12, 12, 12, 12, 24, 24, 24, 24, 24 };

sint_16 masktab[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
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


sint_16 latab[] = { 0x0040, 0x003f, 0x003e, 0x003d, 0x003c, 0x003b, 0x003a, 0x0039,
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

sint_16 hth[][50] = {{ 0x04d0, 0x04d0, 0x0440, 0x0400, 0x03e0, 0x03c0, 0x03b0, 0x03b0,  
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


sint_16 baptab[] = { 0,  1,  1,  1,  1,  1,  2,  2,  3,  3,  3,  4,  4,  5,  5,  6,
                     6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 
                     10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14,
                     14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15 };




void bit_allocate()
{
	/* for fbw channels */ 
	for(ch=0; ch<nfchans; ch++) 
	{ 
		strtmant[ch] = 0; 
		if(chincpl[ch]) 
			endmant[ch] = 37 + (12 * cplbegf); /* channel is coupled */ 
		else 
			endmant[ch] = 37 + (3 * (chbwcod + 12)); /* channel is not coupled */ 
	} 

	/* for coupling channel */ 
	cplstrtmant = 37 + (12 * cplbegf); 
	cplendmant = 37 + (12 * (cplendf + 3)); 

	/* for lfe channel */ 
	lfestartmant = 0 ; lfeendmant = 7 ;

	start = strtmant[ch] ; 
	end = endmant[ch] ; 
	lowcomp = 0 ; 
	fgain = fastgain[fgaincod[ch]]; 
	/* Table 7.11 */ 
	snroffset[ch] = ((csnroffst - 15) << 4 + fsnroffst[ch]) << 2 ;
}


sdecay = slowdec[sdcycod]; 
fdecay = fastdec[fdcycod];
sgain = slowgain[sgaincod]; 
dbknee = dbpbtab[dbpbcod]; 
floor = floortab[floorcod]; 


static void 
bit_allocate_channel(sint_16 start,sint_16 end)
{
	sint_16 psd[256];

	for (bin=start; bin<end; bin++) 
	{ 
		psd[bin] = (3072 - (exps[bin] << 7)); 
	}


	/* Integrate the psd function over each bit allocation band */
	j = start; 
	k = masktab[start]; 
	
	do 
	{ 
		lastbin = min(bndtab[k] + bndsz[k], end); 
		bndpsd[k] = psd[j]; 
		j++; 
		//FIXME I think there's a problem with using a for loop here.
		//If the start frequency is in the last bin of a band, then
		//the first element of the next band gets integrated in before
		//the termination clause kicks in. This code comes from the 
		//standard though. I'll assume it works until I find it doesn't
		for (i = j; i < lastbin; i++) 
		{ 
			bndpsd[k] = logadd(bndpsd[k], psd[j]);
			j++; 
		} 
		
		k++; 
	} while (lastbin <= end );

	/* Compute excitation function */
	bndstrt = masktab[start]; 
	bndend = masktab[end - 1] + 1; 
	
	if (bndstrt == 0) /* For fbw and lfe channels */ 
	{ 
		/* Note: Do not call calc_lowcomp() for the last band of the lfe channel, (bin = 6) */ 
		lowcomp = calc_lowcomp(lowcomp, bndpsd[0], bndpsd[1], 0); 
		excite[0] = bndpsd[0] - fgain - lowcomp; 
		lowcomp = calc_lowcomp(lowcomp, bndpsd[1], bndpsd[2], 1);
		excite[1] = bndpsd[1] - fgain - lowcomp; 
		begin = 7 ; 
		
		for (bin = 2; bin < 7; bin++) 
		{ 
			lowcomp = calc_lowcomp(lowcomp, bndpsd[bin], bndpsd[bin+1], bin); 
			fastleak = bndpsd[bin] - fgain; 
			slowleak = bndpsd[bin] - sgain; 
			excite[bin] = fastleak - lowcomp; 
			
			if (bndpsd[bin] <= bndpsd[bin+1]) 
			{
				begin = bin + 1 ; 
				break; 
			} 
		} 
		
		for (bin = begin; bin < min(bndend, 22); bin++) 
		{ 
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
	{ 
		begin = bndstrt; 
	} 

	for (bin = begin; bin < bndend; bin++) 
	{ 
		fastleak -= fdecay; 
		fastleak = max(fastleak, bndpsd[bin] - fgain); 
		slowleak -= sdecay; 
		slowleak = max(slowleak, bndpsd[bin] - sgain); 
		excite[bin] = max(fastleak, slowleak) ; 
	} 

	/* Compute the masking curve */

	for (bin = bndstrt; bin < bndend; bin++) 
	{ 
		if (bndpsd[bin] < dbknee) 
		{ 
			excite[bin] += ((dbknee - bndpsd[bin]) >> 2); 
		} 
		mask[bin] = max(excite[bin], hth[fscod][bin]);
	}
	
	/* Perform delta bit modulation if necessary */
	if ((deltbae == 0) || (deltbae == 1)) 
	{ 
		sint_16 band = 0; 
		
		for (seg = 0; seg < deltnseg+1; seg++) 
		{ 
			band += deltoffst[seg]; 
			if (deltba[seg] >= 4) 
			{ 
				delta = (deltba[seg] - 3) << 7;
			} 
			else 
			{ 
				delta = (deltba[seg] - 4) << 7;
			} 
			
			for (k = 0; k < deltlen[seg]; k++) 
			{ 
				mask[band] += delta; 
				band++; 
			} 
		} 
	}


	/* Compute the bit allocation pointer for each bin */
	i = start; 
	j = masktab[start]; 

	do 
	{ 
		lastbin = min(bndtab[j] + bndsz[j], end);
		mask[j] -= snroffset; 
		mask[j] -= floor; 
		if (mask[j] < 0) 
		{ 
			mask[j] = 0; 
		}
		mask[j] &= 0x1fe0;
		mask[j] += floor; 
		for (k = i; k < lastbin; k++) 
		{ 
			address = (psd[i] - mask[j]) >> 5; 
			address = min(63, max(0, address)); 
			bap[i] = baptab[address]; 
			i++; 
		} 
		j++;
	} while (lastbin <= end);
}

static inline uint_16
max(sint_16 a,sint_16 b)
{
	return (a > b ? a : b);
}
	
static inline uint_16
min(sint_16 a,sint_16 b)
{
	return (a < b ? a : b);
}
static sint_16 
calc_lowcomp(sint_16 a,sint_16 b0,sint_16 b1,sint_16 bin) 
{ 

	if (bin < 7) 
	{ 
		if ((b0 + 256) == b1)
			a = 384; 
	 	else if (b0 > b1) 
			a = max(0, a - 64); 
	} 
	else if (bin < 20) 
	{ 
		if ((b0 + 256) == b1) 
			a = 320; 
		else if (b0 > b1) 
			a = max(0, a - 64) ; 
	}
	else  
		a = max(0, a - 128); 
	
	return(a);
}

static sint_16 
logadd(sint_16 a,sint_16  b) 
{ 
	sint_16 c;
	sint_16 address;

	c = a - b; 
	address = min((abs(c) >> 1), 255); 
	
	if (c >= 0) 
		return(a + latab(address)); 
	else 
		return(b + latab(address)); 
}

