/* 
 *  decode.c
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

static void decode_fill_syncinfo(bitstream_t *bs);
static void decode_fill_bsi(bitstream_t *bs);
static void decode_fill_audblk(bitstream_t *bs);

static stream_coeffs_t stream_coeffs;
static stream_samples_t stream_samples;
static audblk_t audblk;
static bsi_t bsi;
static syncinfo_t syncinfo;

int main(int argc,char argv[])
{
	int i;
	bitstream_t *bs;

	bs = bitstream_open("foo.dat");
	//FIXME check for end of stream and exit
	while(1)
	{
		decode_fill_syncinfo(bs);
		decode_fill_bsi(bs);
		for(i=0; i < 6; i++)
		{
			decode_fill_audblk(bs);
			/* Take audblk info and turn it into floating point
			 * frequency coefficients for all streams */
			//coeff_fill(&bsi,&audblk,&stream_coeffs);
			/* FIXME Perform dynamic range compensation */
			/* Convert the frequency data into time samples */
			imdct(&stream_coeffs,&stream_samples);
			/* FIXME downmix all channels into the left / right */
			/* Send the samples to the output device */
			//output_samples(&stream_samples);
		}
	}

}

void
decode_fill_syncinfo(bitstream_t *bs)
{
	uint_32 data;

	/* See if we have the sync word */
	data = bitstream_get(bs,16);
	if(data != 0x0b77)
	{
		//FIXME - if we don't have sync, then do proper synchronization
		printf("Arghh!\n");
		exit(1);
	}

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


	//FIXME insert stats here
}

/*
 * This routine fills a bsi struct from the AC3 stream
 */
void
decode_fill_bsi(bitstream_t *bs)
{
	uint_32 i;
	uint_16 nfchans[] = [2,1,2,3,3,4,4,5];

	/* Check the AC-3 version number */
	bsi.bsid = bitstream_get(bs,5);

	/* Get the audio service provided by the steram */
	bsi.bsmod = bitstream_get(bs,3);

	/* Get the audio coding mode (ie how many channels)*/
	bsi.acmod = bitstream_get(bs,3);
	/* Predecode the number of full bandwidth channels as we use this
	 * number a lot */
	bsi.nfchans = nfchans[acmod];

	/* If it is in use, get the centre channel mix level */
	if ((bsi.acmod & 0x1) && (bsi.acmod != 0x1))
		bsi.cmixlev = bitstream_get(bs,2);

	/* If it is in use, get the surround channel mix level */
	if (bsi.acmod & 0x4)
		bsi.surmixlev = bitstream_get(bs,2);

	/* Get the dolby surround mode if in 2/0 mode */
	if(acmode==0x2)
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

	/* Does timecode two exist? */
	bsi.timecod2e = bitstream_get(bs,1);

	if(bsi.timecod1e)
		bsi.timecod1 = bitstream_get(bs,14);

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

	//FIXME insert stats here
}

/* More pain inducing parsing */
void
decode_fill_audblk(bitstream_t *bs)
{
	int i;

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
	if (bsi.dynrnge)
	{
		/* Get dynamic range info */
		audblk.dynrng = bitstream_get(bs,8);
	}

	/* If we're in dual mono mode then get the second channel DR info */
	if (bsi.acmod == 0)
	{
		/* Does dynamic range control two exist? */
		audblk.dynrng2e = bitstream_get(bs,1);
		if (bsi.dynrng2e)
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
			if(acmod == 0x2)
				audblk.phsflginu = bitstream_get(bs,1);
			audblk.cplbegf = bitstream_get(bs,4);
			audblk.cplendf = bitstream_get(bs,4);
			audlbk.ncplsubnd = 3 + audlbk.cplendf - audlbk.cplbegf;

			for(i=1



		}


	}

	


}
