/* 
 *  bitstream_test.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdio.h>
#include "timing.h"
#include "ac3.h"
#include "decode.h"
#include "bitstream.h"
#include "imdct.h"

float i_buf[256];
float o_buf[512];

void main(int argc,char *argv[])
{
	i_buf[80] = 1.0;
	imdct_init();
	timing_test_2(imdct_do,i_buf,o_buf,"imdct_do");
}
