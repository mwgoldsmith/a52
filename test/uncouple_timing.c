/* 
 *  uncouple_timing.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#define HAVE_TSC
#include <stdio.h>
#include "ac3.h"
#include "decode.h"
#include "bitstream.h"
#include "imdct.h"
#include "timing.h"

void convert_to_float(uint_16 exp, uint_16 mant, uint_32 *dest);

uint_64 correction;

void main(int argc,char *argv[])
{
	int i;
	float foo;
	double time;
	double time_acc;
	uint_64 start,end,elapsed,correction;	
	uint_32 iters = 100000;
	uint_16 exp;
	uint_16 mant;
	double mean = 0;
	double variance = 0;

	correction = timing_init();

	printf("\nCorrection Factor = %lld\n",correction);
	printf("Timing convert_to_float %d times\n",iters);
	for (i = 0; i < iters; i++)
	{
		mant = 	rand();
		exp = rand() % 24;
		start = get_time();
		convert_to_float(exp ,mant,&foo);
		end = get_time();
		//printf("Iteration %d - %lld nsec\n",i,end - start);
		if(i>0)
		{
			elapsed = end - start - correction;
			mean += elapsed;
			variance += elapsed * elapsed;
			
		}

	}
	
	mean /= iters;
	variance /= iters;
	variance -= mean * mean;

	printf("mean = %f\n",mean);
	printf("variance= %f\n",variance);

}
