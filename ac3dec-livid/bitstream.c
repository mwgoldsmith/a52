/* 
 *  bitstream.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "decode.h"
#include "crc.h"
#include "bitstream.h"


//My vego-matic endian swapping routine
#define SWAP_ENDIAN32(x)  ((((uint_8*)&x)[0] << 24) |  \
                         (((uint_8*)&x)[1] << 16) |  \
                         (((uint_8*)&x)[2] << 8) |   \
                         ((uint_8*)&x)[3])           


static void bitstream_load(bitstream_t *bs);

/* Fetches 1-32 bits from the file opened in bitstream_open */
uint_32
bitstream_get(bitstream_t *bs,uint_32 num_bits)
{
	uint_32 result;
	uint_32 bits_read;
	uint_32 bits_to_go;


	if(num_bits == 0)
		return 0;

	bits_read = num_bits > bs->bits_left ? bs->bits_left : num_bits; 

	result = bs->current_word  >> (32 - bits_read);
	bs->current_word <<= bits_read;
	bs->bits_left -= bits_read;

	if(bs->bits_left == 0)
		bitstream_load(bs);

	if (bits_read < num_bits)
	{
		bits_to_go = num_bits - bits_read;
		result <<= bits_to_go;
		result |= bs->current_word  >> (32 - bits_to_go);
		bs->current_word <<= bits_to_go;
		bs->bits_left -= bits_to_go;
	}
	
	bs->total_bits_read += num_bits;
	crc_process(result,num_bits);
	return result;
}

static void
bitstream_load(bitstream_t *bs)
{
	int bytes_read;

	bytes_read = fread(&bs->current_word,1,4,bs->file);
	bs->current_word = SWAP_ENDIAN32(bs->current_word);
	bs->bits_left = bytes_read * 8;

	if (bytes_read < 4)
	{
		printf("done!");
		bs->done = 1;
	}
}

/* Opens a bitstream for use in bitstream_get */
bitstream_t*
bitstream_open(FILE *file)
{
	bitstream_t *bs;
	
	if(!file)
		return 0;

	bs = malloc(sizeof(bitstream_t));
	if(!bs)
		return 0;

	bs->done = 0;

	/* Read in the first 32 bit word and initialize the structure */
	bs->file = file;
	bitstream_load(bs);

	return bs;
}

int bitstream_done(bitstream_t *bs)
{
	return (bs->done);
}

void 
bitstream_close(bitstream_t *bs)
{
	if(bs->file)
		fclose(bs->file);

	free(bs);
}
