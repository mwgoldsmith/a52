/* 
 *    decode.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "decode.h"
#include "bitstream.h"
#include "imdct.h"
#include "exponent.h"
#include "mantissa.h"
#include "bit_allocate.h"
#include "stats.h"

static void decode_fill_syncinfo(bitstream_t *bs);
static void decode_fill_bsi(bitstream_t *bs);
static void decode_fill_audblk(bitstream_t *bs);
static void decode_find_sync(bitstream_t *bs);

static stream_coeffs_t stream_coeffs;
static stream_samples_t stream_samples;
static audblk_t audblk;
static bsi_t bsi;
static syncinfo_t syncinfo;
/* Misc LUTs */
static	uint_16 nfchans[] = {2,1,2,3,3,4,4,5};

int main(int argc,char *argv[])
{
	int i,j=0;
	bitstream_t *bs;

	bs = bitstream_open("foo.ac3");
	imdct_init();
	decode_sanity_check_init();

	/* FIXME check for end of stream and exit */


	while(j++ < 16)
	{
		decode_fill_syncinfo(bs);
		decode_fill_bsi(bs);
		for(i=0; i < 6; i++)
		{
			//FIXME remove debugging stuff
				bs->total_bits_read = 0;

			/* Extract most of the audblk info from the bitstream
			 * (minus the mantissas */
			decode_fill_audblk(bs);

			/* Take the differential exponent data and turn it into
			 * absolute exponents */
			exponent_unpack(&bsi,&audblk,&stream_coeffs); 
			/* Figure out how many bits per mantissa */
			bit_allocate(syncinfo.fscod,&bsi,&audblk);

			/* Extract the mantissas from the data stream */
			mantissa_unpack(&bsi,&audblk,bs);

#if 0
			/* Uncouple coupled channels */
			uncouple(&bsi,&audblk,&stream_coeffs); 

			/* Perform dynamic range compensation */
			dynamic_range(&bsi,&audblk,&stream_coeffs); 

			/* Downmix channels appropriately in the frequency domain */
			downmix(&bsi,&audblk,&stream_coeffs); 

#endif
			/* Convert the frequency data into time samples */
			imdct(&bsi,&stream_coeffs,&stream_samples);

			/* Send the samples to the output device */
			/*output_samples(&stream_samples);*/
		printf("      %ld bits (%ld words) read\n",bs->total_bits_read,bs->total_bits_read/16);
		}
		decode_sanity_check();
	}

	return 0;
}

void
decode_fill_syncinfo(bitstream_t *bs)
{
	uint_32 data;

	/* Make sure we sync'ed */
	decode_find_sync(bs);

	/* FIXME  At this point we should probably go through the data
	 * in the buffer and verify the CRC. Initially, we're just 
	 * incrementally reading data from a file, so there's no buffer
	 * to check. */

	/* Get crc1 - we don't actually use this data though */
	data = bitstream_get(bs,16);

	/* Get the sampling rate */
	syncinfo.fscod  = bitstream_get(bs,2);

	/* Get the frame size code */
	syncinfo.frmsizecod = bitstream_get(bs,6);


	/*FIXME insert stats here*/
	stats_printf_syncinfo(&syncinfo);
}

/*
 * This routine fills a bsi struct from the AC3 stream
 */
void
decode_fill_bsi(bitstream_t *bs)
{
	uint_32 i;

	/* Check the AC-3 version number */
	bsi.bsid = bitstream_get(bs,5);

	/* Get the audio service provided by the steram */
	bsi.bsmod = bitstream_get(bs,3);

	/* Get the audio coding mode (ie how many channels)*/
	bsi.acmod = bitstream_get(bs,3);
	/* Predecode the number of full bandwidth channels as we use this
	 * number a lot */
	bsi.nfchans = nfchans[bsi.acmod];

	/* If it is in use, get the centre channel mix level */
	if ((bsi.acmod & 0x1) && (bsi.acmod != 0x1))
		bsi.cmixlev = bitstream_get(bs,2);

	/* If it is in use, get the surround channel mix level */
	if (bsi.acmod & 0x4)
		bsi.surmixlev = bitstream_get(bs,2);

	/* Get the dolby surround mode if in 2/0 mode */
	if(bsi.acmod == 0x2)
		bsi.dsurmod= bitstream_get(bs,2);

	/* Is the low frequency effects channel on? */
	bsi.lfeon = bitstream_get(bs,1);

	/* Get the dialogue normalization level */
	bsi.dialnorm = bitstream_get(bs,5);

	/* Does compression gain exist? */
	bsi.compre = bitstream_get(bs,1);
	if (bsi.compre)
	{
		/* Get compression gain */
		bsi.compr = bitstream_get(bs,8);
	}

	/* Does language code exist? */
	bsi.langcode = bitstream_get(bs,1);
	if (bsi.langcode)
	{
		/* Get langauge code */
		bsi.langcod = bitstream_get(bs,8);
	}

	/* Does audio production info exist? */
	bsi.audprodie = bitstream_get(bs,1);
	if (bsi.audprodie)
	{
		/* Get mix level */
		bsi.mixlevel = bitstream_get(bs,5);

		/* Get room type */
		bsi.roomtyp = bitstream_get(bs,2);
	}

	/* If we're in dual mono mode then get some extra info */
	if (bsi.acmod ==0)
	{
		/* Get the dialogue normalization level two */
		bsi.dialnorm2 = bitstream_get(bs,5);

		/* Does compression gain two exist? */
		bsi.compr2e = bitstream_get(bs,1);
		if (bsi.compr2e)
		{
			/* Get compression gain two */
			bsi.compr2 = bitstream_get(bs,8);
		}

		/* Does language code two exist? */
		bsi.langcod2e = bitstream_get(bs,1);
		if (bsi.langcod2e)
		{
			/* Get langauge code two */
			bsi.langcod2 = bitstream_get(bs,8);
		}

		/* Does audio production info two exist? */
		bsi.audprodi2e = bitstream_get(bs,1);
		if (bsi.audprodi2e)
		{
			/* Get mix level two */
			bsi.mixlevel2 = bitstream_get(bs,5);

			/* Get room type two */
			bsi.roomtyp2 = bitstream_get(bs,2);
		}
	}

	/* Get the copyright bit */
	bsi.copyrightb = bitstream_get(bs,1);

	/* Get the original bit */
	bsi.origbs = bitstream_get(bs,1);
	
	/* Does timecode one exist? */
	bsi.timecod1e = bitstream_get(bs,1);

	if(bsi.timecod1e)
		bsi.timecod1 = bitstream_get(bs,14);

	/* Does timecode two exist? */
	bsi.timecod2e = bitstream_get(bs,1);

	if(bsi.timecod2e)
		bsi.timecod2 = bitstream_get(bs,14);

	/* Does addition info exist? */
	bsi.addbsie = bitstream_get(bs,1);

	if(bsi.addbsie)
	{
		/* Get how much info is there */
		bsi.addbsil = bitstream_get(bs,6);

		/* Get the additional info */
		for(i=0;i<(bsi.addbsil + 1);i++)
			bsi.addbsi[i] = bitstream_get(bs,8);
	}

	stats_printf_bsi(&bsi);
}

/* More pain inducing parsing */
void
decode_fill_audblk(bitstream_t *bs)
{
	int i,j;

	for (i=0;i < bsi.nfchans; i++)
	{
		/* Is this channel an interleaved 256 + 256 block ? */
		audblk.blksw[i] = bitstream_get(bs,1);
	}

	for (i=0;i < bsi.nfchans; i++)
	{
		/* Should we dither this channel? */
		audblk.dithflag[i] = bitstream_get(bs,1);
	}

	/* Does dynamic range control exist? */
	audblk.dynrnge = bitstream_get(bs,1);
	if (audblk.dynrnge)
	{
		/* Get dynamic range info */
		audblk.dynrng = bitstream_get(bs,8);
	}

	/* If we're in dual mono mode then get the second channel DR info */
	if (bsi.acmod == 0)
	{
		/* Does dynamic range control two exist? */
		audblk.dynrng2e = bitstream_get(bs,1);
		if (audblk.dynrng2e)
		{
			/* Get dynamic range info */
			audblk.dynrng2 = bitstream_get(bs,8);
		}
	}

	/* Does coupling strategy exist? */
	audblk.cplstre = bitstream_get(bs,1);
	if (audblk.cplstre)
	{
		/* Is coupling turned on? */
		audblk.cplinu = bitstream_get(bs,1);
		if(audblk.cplinu)
		{
			for(i=0;i < bsi.nfchans; i++)
				audblk.chincpl[i] = bitstream_get(bs,1);
			if(bsi.acmod == 0x2)
				audblk.phsflginu = bitstream_get(bs,1);
			audblk.cplbegf = bitstream_get(bs,4);
			audblk.cplendf = bitstream_get(bs,4);
			audblk.ncplsubnd = (audblk.cplendf + 2) - audblk.cplbegf + 1;

			/* Calculate the start and end bins of the coupling channel */
			audblk.cplstrtmant = (audblk.cplbegf * 12) + 37 ; 
			audblk.cplendmant =  ((audblk.cplendf + 3) * 12) + 37;

			/* The number of combined subbands is ncplsubnd minus each combined
			 * band */
			audblk.ncplbnd = audblk.ncplsubnd; 

			for(i=1;i < audblk.ncplsubnd; i++)
			{
				audblk.cplbndstrc[i] = bitstream_get(bs,1);
				audblk.ncplbnd -= audblk.cplbndstrc[i];
			}

			/* Loop through all the channels and get their coupling co-ords */	
			for(i=0;i < bsi.nfchans;i++)
			{
				if(!audblk.chincpl[i])
					continue;

				/* Is there new coupling co-ordinate info? */
				audblk.cplcoe[i] = bitstream_get(bs,1);

				if(audblk.cplcoe[i])
				{
					audblk.mstrcplco[i] = bitstream_get(bs,1); 
					for(j=0;j < audblk.ncplbnd; j++)
					{
						audblk.cplcoexp[i][j] = bitstream_get(bs,4); 
						audblk.cplcomant[i][j] = bitstream_get(bs,4); 
					}
				}
			}

			/* If we're in dual mono mode, there's going to be some phase info */
			if( (bsi.acmod == 0x2) && audblk.phsflginu && 
					(audblk.cplcoe[0] || audblk.cplcoe[1]))
			{
				for(j=0;j < audblk.ncplbnd; j++)
					audblk.phsflg[j] = bitstream_get(bs,1); 

			}
		}
	}

	/* If we're in dual mono mode, there may be a rematrix strategy */
	if(bsi.acmod == 0x2)
	{
		audblk.rematstr = bitstream_get(bs,1);
		if(audblk.rematstr)
		{
			if((audblk.cplbegf > 2) || (audblk.cplinu == 0)) 
			{ 
				for(i = 0; i < 4; i++) 
					audblk.rematflg[i] = bitstream_get(bs,1);
			}
			if((audblk.cplbegf <= 2) && audblk.cplinu) 
			{ 
				for(i = 0; i < 3; i++) 
					audblk.rematflg[i] = bitstream_get(bs,1);
			} 
			if((audblk.cplbegf == 0) && audblk.cplinu) 
				for(i = 0; i < 2; i++) 
					audblk.rematflg[i] = bitstream_get(bs,1);

		}
	}

	if (audblk.cplinu)
	{
		/* Get the coupling channel exponent strategy */
		audblk.cplexpstr = bitstream_get(bs,2);
		if (audblk.cplexpstr == 0)
			audblk.ncplgrps = 0;	
		else
			audblk.ncplgrps = (audblk.cplendmant - audblk.cplstrtmant) / 
				(3 * (3 << (audblk.cplstrtmant-1)));
	}

	for(i = 0; i < bsi.nfchans; i++)
		audblk.chexpstr[i] = bitstream_get(bs,2);

	/* Get the exponent strategy for lfe channel */
	if(bsi.lfeon) 
		audblk.lfeexpstr = bitstream_get(bs,1);

	/* Determine the bandwidths of all the fbw channels */
	for(i = 0; i < bsi.nfchans; i++) 
	{ 
		uint_16 grp_size;

		if(audblk.chexpstr[i] != EXP_REUSE) 
		{ 
			if (!audblk.chincpl[i]) 
			{
				audblk.chbwcod[i] = bitstream_get(bs,6); 
				audblk.endmant[i] = ((audblk.chbwcod[i] + 12) * 3) + 37;
			}
			else
				audblk.endmant[i] = audblk.cplstrtmant;

			/* Calculate the number of exponent groups to fetch */
			grp_size =  (3 << (audblk.chexpstr[i] - 1));
			audblk.nchgrps[i] = (audblk.endmant[i] - 1 + (grp_size - 3)) / grp_size;
		}
	}

	/* Get the coupling exponents if they exist */
	if(audblk.cplinu && (audblk.cplexpstr != EXP_REUSE))
	{
		audblk.cplabsexp = bitstream_get(bs,4);
		for(i=0;i< audblk.ncplgrps;i++)
			audblk.cplexps[i] = bitstream_get(bs,7);
	}

	/* Get the fwb channel exponents */
	for(i=0;i < bsi.nfchans; i++)
	{
		if(audblk.chexpstr[i] != EXP_REUSE)
		{
			audblk.exps[i][0] = bitstream_get(bs,4);			
			for(j=1;j<=audblk.nchgrps[i];j++)
				audblk.exps[i][j] = bitstream_get(bs,7);
			audblk.gainrng[i] = bitstream_get(bs,2);
		}
	}

	/* Get the lfe channel exponents */
	if(bsi.lfeon && (audblk.lfeexpstr != EXP_REUSE))
	{
		audblk.lfeexps[0] = bitstream_get(bs,4);
		audblk.lfeexps[1] = bitstream_get(bs,7);
		audblk.lfeexps[2] = bitstream_get(bs,7);
	}

	/* Get the parametric bit allocation parameters */
	audblk.baie = bitstream_get(bs,1);

	if(audblk.baie)
	{
		audblk.sdcycod = bitstream_get(bs,2);
		audblk.fdcycod = bitstream_get(bs,2);
		audblk.sgaincod = bitstream_get(bs,2);
		audblk.dbpbcod = bitstream_get(bs,2);
		audblk.floorcod = bitstream_get(bs,3);
	}

	/* Get the SNR off set info if it exists */
	audblk.snroffste = bitstream_get(bs,1);

	if(audblk.snroffste)
	{
		audblk.csnroffst = bitstream_get(bs,6);

		if(audblk.cplinu)
		{
			audblk.cplfsnroffst = bitstream_get(bs,4);
			audblk.cplfgaincod = bitstream_get(bs,3);
		}

		for(i = 0;i < bsi.nfchans; i++)
		{
			audblk.fsnroffst[i] = bitstream_get(bs,4);
			audblk.fgaincod[i] = bitstream_get(bs,3);
		}
		if(bsi.lfeon)
		{

			audblk.lfefsnroffst = bitstream_get(bs,4);
			audblk.lfefgaincod = bitstream_get(bs,3);
		}
	}

	/* Get coupling leakage info if it exists */
	if(audblk.cplinu)
	{
		audblk.cplleake = bitstream_get(bs,1);	
		
		if(audblk.cplleake)
		{
			audblk.cplfleak = bitstream_get(bs,3);
			audblk.cplsleak = bitstream_get(bs,3);
		}
	}
	
	/* Get the delta bit alloaction info */
	audblk.deltbaie = bitstream_get(bs,1);	
	
	if(audblk.deltbaie)
	{
		if(audblk.cplinu)
			audblk.cpldeltbae = bitstream_get(bs,2);

		for(i = 0;i < bsi.nfchans; i++)
			audblk.deltbae[i] = bitstream_get(bs,2);

		if (audblk.cplinu && (audblk.cpldeltbae == DELTA_BIT_NEW))
		{
			audblk.cpldeltnseg = bitstream_get(bs,3);
			for(i = 0;i < audblk.cpldeltnseg + 1; i++)
			{
				audblk.cpldeltoffst[i] = bitstream_get(bs,5);
				audblk.cpldeltlen[i] = bitstream_get(bs,4);
				audblk.cpldeltba[i] = bitstream_get(bs,3);
			}
		}

		for(i = 0;i < bsi.nfchans; i++)
		{
			if (audblk.cplinu && (audblk.deltbae[i] == DELTA_BIT_NEW))
			{
				audblk.deltnseg[i] = bitstream_get(bs,3);
				for(j = 0; j < audblk.cpldeltnseg + 1; j++)
				{
					audblk.deltoffst[i][j] = bitstream_get(bs,5);
					audblk.deltlen[i][j] = bitstream_get(bs,4);
					audblk.deltba[i][j] = bitstream_get(bs,3);
				}
			}
		}
	}

	/* Check to see if there's any dummy info to get */
	if(/* skiple = */ bitstream_get(bs,1))
	{
		uint_16 skipl;
		uint_16 skip_data;

		skipl = bitstream_get(bs,9);
		for(i = 0; i < skipl + 20; i++)
		{
			skip_data = bitstream_get(bs,8);
			//FIXME remove
			//printf("skipdata = %2x\n",skip_data);
		}

	}

	stats_printf_audblk(&audblk);
}

static 
void decode_find_sync(bitstream_t *bs)
{
	uint_16 sync_word;
	uint_32 i = 0;

	sync_word = bitstream_get(bs,16);

	while(1)
	{
		if(sync_word == 0x0b77)
			break;
		sync_word = sync_word * 2;
		sync_word |= bitstream_get(bs,1);
		i++;
	}
	fprintf(stderr,"(sync) %ld bits skipped to synchronize\n",i);
}

void decode_sanity_check_init(void)
{
	syncinfo.magic = DECODE_MAGIC_NUMBER;
	bsi.magic = DECODE_MAGIC_NUMBER;
	audblk.magic1 = DECODE_MAGIC_NUMBER;
	audblk.magic2 = DECODE_MAGIC_NUMBER;
	audblk.magic3 = DECODE_MAGIC_NUMBER;
}

void decode_sanity_check(void)
{
	int i;

	if(syncinfo.magic != DECODE_MAGIC_NUMBER)
		goto err;
	
	if(bsi.magic != DECODE_MAGIC_NUMBER)
		goto err;

	if(audblk.magic1 != DECODE_MAGIC_NUMBER)
		goto err;

	if(audblk.magic2 != DECODE_MAGIC_NUMBER)
		goto err;

	if(audblk.magic3 != DECODE_MAGIC_NUMBER)
		goto err;

	for(i = 0;i < 5 ; i++)
	{
		if (audblk.fbw_exp[i][255] !=0 || audblk.fbw_exp[i][254] !=0 || 
				audblk.fbw_exp[i][253] !=0)
			goto err;

		if (audblk.fbw_bap[i][255] !=0 || audblk.fbw_bap[i][254] !=0 || 
				audblk.fbw_bap[i][253] !=0)
			goto err;

		if (audblk.chmant[i][255] !=0 || audblk.chmant[i][254] !=0 || 
				audblk.chmant[i][253] !=0)
			goto err;
	}

	if (audblk.cpl_exp[255] !=0 || audblk.cpl_exp[254] !=0 || 
			audblk.cpl_exp[253] !=0)
		goto err;

	if (audblk.cpl_bap[255] !=0 || audblk.cpl_bap[254] !=0 || 
			audblk.cpl_bap[253] !=0)
		goto err;

	if (audblk.cplmant[255] !=0 || audblk.cplmant[254] !=0 || 
			audblk.cplmant[253] !=0)
		goto err;

	if ((audblk.cplinu == 1) && (audblk.cplbegf > (audblk.cplendf+2)))
			goto err;

	for(i=0; i < bsi.nfchans; i++)
	{
		if((audblk.chincpl[i] == 0) && (audblk.chbwcod[i] > 60))
			goto err;
	}

	return;
err:
	printf("\n!! Sanity check failed !!");
	//exit(1);
}	

