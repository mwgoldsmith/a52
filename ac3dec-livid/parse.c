/* 
 *    parse.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "bitstream.h"
#include "parse.h"

void
parse_auxdata(bitstream_t *bs)
{
	int i;
	int skip_length;
	uint_16 crc;
	uint_16 auxdatae;

	//FIXME use proper frame size
	skip_length = (768 * 16)  - bs->total_bits_read - 17 - 1;
	printf("Skipping %d auxbits\n",skip_length);

	for(i=0; i <  skip_length; i++)
		//printf("Skipped bit %i\n",(uint_16)bitstream_get(bs,1));
		bitstream_get(bs,1);

	//get the auxdata exists bit
	auxdatae = bitstream_get(bs,1);	
  printf("auxdatae =  %i\n",auxdatae);

	//Skip the CRC reserved bit
	bitstream_get(bs,1);

	crc = bitstream_get(bs,16);
	printf("crc = %i\n",crc);
}


