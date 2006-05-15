/* 
 *  decode.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include "bitstream.h"


static stream_coeffs_t stream_coeffs;
static stream_samples_t stream_samples;
static audblk_t audblk;
static bsi_t bsi;
static syncinfo_t syncinfo;

int main(int argc,char argv[])
{
	int i;

	bitstream_open
	while(1)
	{
		decode_fill_syncinfo();
		decode_fill_bsi();
		for(i=0; i < 6; i++)
		{
			decode_fill_audblk();
			/* Take audblk info and turn it into floating point
			 * frequency coefficients for all streams */
			coeff_fill(&audblk,&stream_coeffs);
			/* FIXME Perform dynamic range compensation */
			/* Convert the frequency data into time samples */
			imdct(&stream_coeffs,&stream_samples);
			/* FIXME downmix all channels into the left / right */
			/* Send the samples to the output device */
			output_samples(&stream_samples);
		}
	}

}

void;
decode_fill_syncinfo()
{
	bitstream_get	

}
