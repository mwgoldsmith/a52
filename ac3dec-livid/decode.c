/* 
 *    decode.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "ac3.h"
#include "decode.h"
#include "bitstream.h"
#include "imdct.h"
#include "exponent.h"
#include "mantissa.h"
#include "bit_allocate.h"
#include "uncouple.h"
#include "parse.h"
#include "output.h"
#include "crc.h"
#include "sys/time.h"
#include "debug.h"

static void decode_find_sync(bitstream_t *bs);

static stream_coeffs_t stream_coeffs;
static stream_samples_t stream_samples;
static audblk_t audblk;
static bsi_t bsi;
static syncinfo_t syncinfo;
static uint_32 frame_count = 0;


int main(int argc,char *argv[])
{
	int i,j=0;
	bitstream_t *bs;
	uint_32 bits_per_audblk;
	FILE *in_file;
	hrtime_t start;

	/* If we get an argument then use it as a filename... otherwise use
	 * stdin */
	if(argc > 1)
	{
		in_file = fopen(argv[1],"r");	
		if(!in_file)
		{
			fprintf(stderr,"%s - Couldn't open file %s\n",strerror(errno),argv[1]);
			exit(1);
		}
	}
	else
		in_file = stdin;


	bs = bitstream_open(in_file);
	imdct_init();
	decode_sanity_check_init();
	output_open(16,48000,2);


	/* FIXME check for end of stream and exit */


	while(j++ < 500)
	{
		decode_find_sync(bs);

		parse_syncinfo(&syncinfo,bs);

		parse_bsi(&bsi,bs);

		for(i=0; i < 6; i++)
		{
			//FIXME remove debugging stuff
		start = gethrtime();

			/* Extract most of the audblk info from the bitstream
			 * (minus the mantissas */
			bits_per_audblk = bs->total_bits_read;

			parse_audblk(&bsi,&audblk,bs);
		decode_sanity_check();

			/* Take the differential exponent data and turn it into
			 * absolute exponents */
			exponent_unpack(&bsi,&audblk,&stream_coeffs); 
		decode_sanity_check();
		fprintf(stderr,"ba - %lld ns ",gethrtime() - start);
			/* Figure out how many bits per mantissa */
			bit_allocate(syncinfo.fscod,&bsi,&audblk);
		decode_sanity_check();

		fprintf(stderr,"mant - %lld ns ",gethrtime() - start);
			/* Extract the mantissas from the data stream */
			mantissa_unpack(&bsi,&audblk,bs);
		decode_sanity_check();

			/* Uncouple the coupling channel if it exists and
			 * convert the mantissa and exponents to IEEE floating
			 * point format */
			uncouple(&bsi,&audblk,&stream_coeffs);
		decode_sanity_check();

#if 0
			/* Perform dynamic range compensation */
			dynamic_range(&bsi,&audblk,&stream_coeffs); 

			/* Downmix channels appropriately in the frequency domain */
			downmix(&bsi,&audblk,&stream_coeffs); 
#endif

		fprintf(stderr,"imdct - %lld ns ",gethrtime() - start);
			/* Convert the frequency data into time samples */
			imdct(&bsi,&audblk,&stream_coeffs,&stream_samples);
		decode_sanity_check();

		fprintf(stderr,"output - %lld ns ",gethrtime() - start);
			/* Send the samples to the output device */
			output_play(&stream_samples);
		fprintf(stderr,"end - %lld ns\n",gethrtime() - start);

			//FIXME remove
			//fprintf(stderr,"%ld bits for this audblk\n",
			//		bs->total_bits_read - bits_per_audblk );

		}
		parse_auxdata(bs);

		if(!crc_validate())
		{
			dprintf("(crc) CRC check failed\n");
		}
		else
		{
			dprintf("(crc) CRC check passed\n");
		}

		//FIXME remove
		//fprintf(stderr,"      %ld bits (%ld words) read\n",
		//		bs->total_bits_read,bs->total_bits_read/16);
		decode_sanity_check();
	}

	return 0;
}



static 
void decode_find_sync(bitstream_t *bs)
{
	uint_16 sync_word;
	uint_32 i = 0;

	sync_word = bitstream_get(bs,16);

	/* Make sure we sync'ed */
	while(1)
	{
		if(sync_word == 0x0b77)
			break;
		sync_word = sync_word * 2;
		sync_word |= bitstream_get(bs,1);
		i++;
	}
	dprintf("(sync) %ld bits skipped to synchronize\n",i);
	dprintf("(sync) begin frame %ld\n",frame_count);
	frame_count++;

	bs->total_bits_read = 16;
	crc_init();
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
		fprintf(stderr,"\n** Sanity check failed -- syncinfo magic number **");
	
	if(bsi.magic != DECODE_MAGIC_NUMBER)
		fprintf(stderr,"\n** Sanity check failed -- bsi magic number **");

	if(audblk.magic1 != DECODE_MAGIC_NUMBER)
		fprintf(stderr,"\n** Sanity check failed -- audblk magic number 1 **"); 

	if(audblk.magic2 != DECODE_MAGIC_NUMBER)
		fprintf(stderr,"\n** Sanity check failed -- audblk magic number 2 **"); 

	if(audblk.magic3 != DECODE_MAGIC_NUMBER)
		fprintf(stderr,"\n** Sanity check failed -- audblk magic number 3 **"); 

	for(i = 0;i < 5 ; i++)
	{
		if (audblk.fbw_exp[i][255] !=0 || audblk.fbw_exp[i][254] !=0 || 
				audblk.fbw_exp[i][253] !=0)
			fprintf(stderr,"\n** Sanity check failed -- fbw_exp out of bounds **"); 

		if (audblk.fbw_bap[i][255] !=0 || audblk.fbw_bap[i][254] !=0 || 
				audblk.fbw_bap[i][253] !=0)
			fprintf(stderr,"\n** Sanity check failed -- fbw_bap out of bounds **"); 

		if (audblk.chmant[i][255] !=0 || audblk.chmant[i][254] !=0 || 
				audblk.chmant[i][253] !=0)
			fprintf(stderr,"\n** Sanity check failed -- chmant out of bounds **"); 
	}

	if (audblk.cpl_exp[255] !=0 || audblk.cpl_exp[254] !=0 || 
			audblk.cpl_exp[253] !=0)
		fprintf(stderr,"\n** Sanity check failed -- cpl_exp out of bounds **"); 

	if (audblk.cpl_bap[255] !=0 || audblk.cpl_bap[254] !=0 || 
			audblk.cpl_bap[253] !=0)
		fprintf(stderr,"\n** Sanity check failed -- cpl_bap out of bounds **"); 

	if (audblk.cplmant[255] !=0 || audblk.cplmant[254] !=0 || 
			audblk.cplmant[253] !=0)
		fprintf(stderr,"\n** Sanity check failed -- cpl_mant out of bounds **"); 

	if ((audblk.cplinu == 1) && (audblk.cplbegf > (audblk.cplendf+2)))
		fprintf(stderr,"\n** Sanity check failed -- cpl params inconsistent **"); 

	for(i=0; i < bsi.nfchans; i++)
	{
		if((audblk.chincpl[i] == 0) && (audblk.chbwcod[i] > 60))
			fprintf(stderr,"\n** Sanity check failed -- chbwcod too big **"); 
	}

	return;
}	

