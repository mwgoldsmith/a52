/* 
 *  bitstream_test.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "bitstream.h"
#include "decode.h"
#include "imdct.h"

static stream_samples_t samples;
static stream_coeffs_t coeffs;

int main(void)
{
	int i;

	coeffs.left[80] = 1.0;

	imdct_init();

	imdct(&coeffs,&samples);

	for(i=0;i<512;i++)
		printf("%f\n",samples.left[i]);
	
	return 0;

}
