/* 
 *    parse.c
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
#include "ac3.h"
#include "ac3_internal.h"


#include "bitstream.h"
#include "stats.h"
#include "debug.h"
#include "parse.h"

/* Misc LUT */
static const uint16_t nfchans[8] = {2,1,2,3,3,4,4,5};

int parse_syncinfo (uint8_t * buf, int * sample_rate, int * bit_rate)
{
    static int rate[] = { 32,  40,  48,  56,  64,  80,  96, 112,
			 128, 160, 192, 224, 256, 320, 384, 448,
			 512, 576, 640};
    int frmsizecod;
    int bitrate;

    if ((buf[0] != 0x0b) || (buf[1] != 0x77))	// check syncword
	return 0;

    frmsizecod = buf[4] & 63;
    if (frmsizecod >= 38)
	return 0;
    *bit_rate = bitrate = rate [frmsizecod >> 1];

    switch (buf[4] & 0xc0) {
    case 0:	// 48 KHz
	*sample_rate = 48000;
	return 4 * bitrate;
    case 0x40:
	*sample_rate = 44100;
	return 2 * (320 * bitrate / 147 + (frmsizecod & 1));
    case 0x80:
	*sample_rate = 32000;
	return 6 * bitrate;
    default:
	return 0;
    }
}

/*
 * This routine fills a bsi struct from the AC3 stream
 */

int parse_bsi(bsi_t *bsi, uint8_t * buf)
{
    int chaninfo;

    if (buf[5] >= 0x48)		// bsid >= 9
	return 1;

    /* Get the audio coding mode (ie how many channels)*/
    bsi->acmod = buf[6] >> 5;
    /* Predecode the number of full bandwidth channels as we use this
     * number a lot */
    bsi->nfchans = nfchans[bsi->acmod];

    bitstream_set_ptr (buf + 6);
    bitstream_get (3);	// skip acmod we already parsed

    /* If it is in use, get the centre channel mix level */
    if ((bsi->acmod & 0x1) && (bsi->acmod != 0x1))
	bsi->cmixlev = bitstream_get(2);

    /* If it is in use, get the surround channel mix level */
    if (bsi->acmod & 0x4)
	bsi->surmixlev = bitstream_get(2);

    /* Get the dolby surround mode if in 2/0 mode */
    if(bsi->acmod == 0x2)
	bitstream_get(2);	// dsurmod

    /* Is the low frequency effects channel on? */
    bsi->lfeon = bitstream_get(1);

    chaninfo = (bsi->acmod) ? 1 : 2;
    do {
	bitstream_get(5);	// dialnorm
	if (bitstream_get(1))	// compre
	    bitstream_get(8);	// compr
	if (bitstream_get(1))	// langcode
	    bitstream_get(8);	// langcod
	if (bitstream_get(1))	// audprodie
	    bitstream_get(7);	// mixlevel + roomtyp
    } while (--chaninfo);

    /* Get the copyright bit */
    bsi->copyrightb = bitstream_get(1);

    /* Get the original bit */
    bsi->origbs = bitstream_get(1);
	
    /* Does timecode one exist? */
    bsi->timecod1e = bitstream_get(1);
    if(bsi->timecod1e)
	bsi->timecod1 = bitstream_get(14);

    /* Does timecode two exist? */
    bsi->timecod2e = bitstream_get(1);
    if(bsi->timecod2e)
	bsi->timecod2 = bitstream_get(14);

    /* Does addition info exist? */
    bsi->addbsie = bitstream_get(1);
    if(bsi->addbsie) {
	int i;

	/* Get how much info is there */
	bsi->addbsil = bitstream_get(6);

	/* Get the additional info */
	for(i=0;i<(bsi->addbsil + 1);i++)
	    bsi->addbsi[i] = bitstream_get(8);
    }

    stats_print_bsi(bsi);
    return 0;
}

/* More pain inducing parsing */
void parse_audblk(bsi_t *bsi,audblk_t *audblk)
{
    int i,j;

    for (i=0;i < bsi->nfchans; i++) {
	/* Is this channel an interleaved 256 + 256 block ? */
	audblk->blksw[i] = bitstream_get(1);
    }

    for (i=0;i < bsi->nfchans; i++) {
	/* Should we dither this channel? */
	audblk->dithflag[i] = bitstream_get(1);
    }

    /* Does dynamic range control exist? */
    audblk->dynrnge = bitstream_get(1);
    if (audblk->dynrnge) {
	/* Get dynamic range info */
	audblk->dynrng = bitstream_get(8);
    }

    /* If we're in dual mono mode then get the second channel DR info */
    if (bsi->acmod == 0) {
	/* Does dynamic range control two exist? */
	audblk->dynrng2e = bitstream_get(1);
	if (audblk->dynrng2e) {
	    /* Get dynamic range info */
	    audblk->dynrng2 = bitstream_get(8);
	}
    }

    /* Does coupling strategy exist? */
    audblk->cplstre = bitstream_get(1);
    if (audblk->cplstre) {
	/* Is coupling turned on? */
	audblk->cplinu = bitstream_get(1);
	if(audblk->cplinu) {
	    for(i=0;i < bsi->nfchans; i++)
		audblk->chincpl[i] = bitstream_get(1);
	    if(bsi->acmod == 0x2)
		audblk->phsflginu = bitstream_get(1);
	    audblk->cplbegf = bitstream_get(4);
	    audblk->cplendf = bitstream_get(4);
	    audblk->ncplsubnd = (audblk->cplendf + 2) - audblk->cplbegf + 1;

	    /* Calculate the start and end bins of the coupling channel */
	    audblk->cplstrtmant = (audblk->cplbegf * 12) + 37 ; 
	    audblk->cplendmant =  ((audblk->cplendf + 3) * 12) + 37;

	    /* The number of combined subbands is ncplsubnd minus each combined
	     * band */
	    audblk->ncplbnd = audblk->ncplsubnd; 

	    for(i=1; i< audblk->ncplsubnd; i++) {
		audblk->cplbndstrc[i] = bitstream_get(1);
		audblk->ncplbnd -= audblk->cplbndstrc[i];
	    }
	}
    }

    if(audblk->cplinu) {
	/* Loop through all the channels and get their coupling co-ords */	
	for(i=0;i < bsi->nfchans;i++) {
	    if(!audblk->chincpl[i])
		continue;

	    /* Is there new coupling co-ordinate info? */
	    audblk->cplcoe[i] = bitstream_get(1);
	    if(audblk->cplcoe[i]) {
		audblk->mstrcplco[i] = bitstream_get(2); 
		for(j=0;j < audblk->ncplbnd; j++) {
		    audblk->cplcoexp[i][j] = bitstream_get(4); 
		    audblk->cplcomant[i][j] = bitstream_get(4); 
		}
	    }
	}

	/* If we're in dual mono mode, there's going to be some phase info */
	if( (bsi->acmod == 0x2) && audblk->phsflginu && 
	    (audblk->cplcoe[0] || audblk->cplcoe[1])) {
	    for(j=0;j < audblk->ncplbnd; j++)
		audblk->phsflg[j] = bitstream_get(1);
	}
    }

    /* If we're in dual mono mode, there may be a rematrix strategy */
    if(bsi->acmod == 0x2) {
	audblk->rematstr = bitstream_get(1);
	if(audblk->rematstr) {
	    if (audblk->cplinu == 0)
		for(i = 0; i < 4; i++) 
		    audblk->rematflg[i] = bitstream_get(1);
	    if((audblk->cplbegf > 2) && audblk->cplinu)
		for(i = 0; i < 4; i++) 
		    audblk->rematflg[i] = bitstream_get(1);
	    if((audblk->cplbegf <= 2) && audblk->cplinu)
		for(i = 0; i < 3; i++) 
		    audblk->rematflg[i] = bitstream_get(1);
	    if((audblk->cplbegf == 0) && audblk->cplinu) 
		for(i = 0; i < 2; i++) 
		    audblk->rematflg[i] = bitstream_get(1);
	}
    }

    if (audblk->cplinu) {
	/* Get the coupling channel exponent strategy */
	audblk->cplexpstr = bitstream_get(2);
	audblk->ncplgrps = (audblk->cplendmant - audblk->cplstrtmant) / 
	    (3 << (audblk->cplexpstr-1));
    }

    for(i = 0; i < bsi->nfchans; i++)
	audblk->chexpstr[i] = bitstream_get(2);

    /* Get the exponent strategy for lfe channel */
    if(bsi->lfeon) 
	audblk->lfeexpstr = bitstream_get(1);

    /* Determine the bandwidths of all the fbw channels */
    for(i = 0; i < bsi->nfchans; i++) { 
	uint16_t grp_size;

	if(audblk->chexpstr[i] != EXP_REUSE) { 
	    if (audblk->cplinu && audblk->chincpl[i]) 
		audblk->endmant[i] = audblk->cplstrtmant;
	    else {
		audblk->chbwcod[i] = bitstream_get(6); 
		audblk->endmant[i] = ((audblk->chbwcod[i] + 12) * 3) + 37;
	    }

	    /* Calculate the number of exponent groups to fetch */
	    grp_size =  3 * (1 << (audblk->chexpstr[i] - 1));
	    audblk->nchgrps[i] = (audblk->endmant[i] - 1 + (grp_size - 3)) / grp_size;
	}
    }

    /* Get the coupling exponents if they exist */
    if(audblk->cplinu && (audblk->cplexpstr != EXP_REUSE)) {
	audblk->cplabsexp = bitstream_get(4);
	for(i=0;i< audblk->ncplgrps;i++)
	    audblk->cplexps[i] = bitstream_get(7);
    }

    /* Get the fwb channel exponents */
    for(i=0;i < bsi->nfchans; i++) {
	if(audblk->chexpstr[i] != EXP_REUSE) {
	    audblk->exps[i][0] = bitstream_get(4);			
	    for(j=1;j<=audblk->nchgrps[i];j++)
		audblk->exps[i][j] = bitstream_get(7);
	    audblk->gainrng[i] = bitstream_get(2);
	}
    }

    /* Get the lfe channel exponents */
    if(bsi->lfeon && (audblk->lfeexpstr != EXP_REUSE)) {
	audblk->lfeexps[0] = bitstream_get(4);
	audblk->lfeexps[1] = bitstream_get(7);
	audblk->lfeexps[2] = bitstream_get(7);
    }

    /* Get the parametric bit allocation parameters */
    audblk->baie = bitstream_get(1);
    if(audblk->baie) {
	audblk->sdcycod = bitstream_get(2);
	audblk->fdcycod = bitstream_get(2);
	audblk->sgaincod = bitstream_get(2);
	audblk->dbpbcod = bitstream_get(2);
	audblk->floorcod = bitstream_get(3);
    }

    /* Get the SNR off set info if it exists */
    audblk->snroffste = bitstream_get(1);
    if(audblk->snroffste) {
	audblk->csnroffst = bitstream_get(6);

	if(audblk->cplinu) {
	    audblk->cplfsnroffst = bitstream_get(4);
	    audblk->cplfgaincod = bitstream_get(3);
	}

	for(i = 0;i < bsi->nfchans; i++) {
	    audblk->fsnroffst[i] = bitstream_get(4);
	    audblk->fgaincod[i] = bitstream_get(3);
	}
	if(bsi->lfeon) {
	    audblk->lfefsnroffst = bitstream_get(4);
	    audblk->lfefgaincod = bitstream_get(3);
	}
    }

    /* Get coupling leakage info if it exists */
    if(audblk->cplinu) {
	audblk->cplleake = bitstream_get(1);
	if(audblk->cplleake) {
	    audblk->cplfleak = bitstream_get(3);
	    audblk->cplsleak = bitstream_get(3);
	}
    }
	
    /* Get the delta bit alloaction info */
    audblk->deltbaie = bitstream_get(1);
    if(audblk->deltbaie) {
	if(audblk->cplinu)
	    audblk->cpldeltbae = bitstream_get(2);

	for(i = 0;i < bsi->nfchans; i++)
	    audblk->deltbae[i] = bitstream_get(2);

	if (audblk->cplinu && (audblk->cpldeltbae == DELTA_BIT_NEW)) {
	    audblk->cpldeltnseg = bitstream_get(3);
	    for(i = 0;i < audblk->cpldeltnseg + 1; i++) {
		audblk->cpldeltoffst[i] = bitstream_get(5);
		audblk->cpldeltlen[i] = bitstream_get(4);
		audblk->cpldeltba[i] = bitstream_get(3);
	    }
	}

	for(i = 0;i < bsi->nfchans; i++) {
	    if (audblk->deltbae[i] == DELTA_BIT_NEW) {
		audblk->deltnseg[i] = bitstream_get(3);
		for(j = 0; j < audblk->deltnseg[i] + 1; j++) {
		    audblk->deltoffst[i][j] = bitstream_get(5);
		    audblk->deltlen[i][j] = bitstream_get(4);
		    audblk->deltba[i][j] = bitstream_get(3);
		}
	    }
	}
    }

    /* Check to see if there's any dummy info to get */
    if((audblk->skiple =  bitstream_get(1))) {
	uint16_t skip_data;

	audblk->skipl = bitstream_get(9);
	//XXX remove
	//fprintf(stderr,"(parse) skipping %d bytes\n",audblk->skipl);

	for(i = 0; i < audblk->skipl ; i++) {
	    skip_data = bitstream_get(8);
	    //XXX remove
	    //fprintf(stderr,"skipped data %2x\n",skip_data);
	    //if(skip_data != 0)
	    //{	
	    //dprintf("(parse) Invalid skipped data %2x\n",skip_data);
	    //exit(1);
	    //}
	}
    }

    stats_print_audblk(bsi,audblk);
}
