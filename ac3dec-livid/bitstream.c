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
#include "bitstream.h"


static uint_32 bit_mask[] = { 
	0x80000000, 0xC0000000, 0xE0000000,0xF0000000,
	0xF8000000, 0xFC000000, 0xFE000000,0xFF000000,
	0xFF800000, 0xFFC00000, 0xFFE00000,0xFFF00000,
	0xFFF80000, 0xFFFC0000, 0xFFFE0000,0xFFFF0000,
	0xFFFF8000, 0xFFFFC000, 0xFFFFE000,0xFFFFF000,
	0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00,0xFFFFFF00,
	0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0,0xFFFFFFF0,
	0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE,0xFFFFFFFF};

/* Fetches 1-32 bits from the file opened in bitstream_open */
uint_32
bitstream_get(bitstream_t *bs,uint_32 num_bits)
{
	uint_32 result;

	if (num_bits < bs->bits_left)
	{
		result = (bs->current_word & bit_mask[num_bits]) >> (32 - num_bits);
		/*printf("case 1 current_word = %lx result = %lx\n",bs->current_word,result);*/
		bs->current_word <<= num_bits;
		bs->bits_left -= num_bits;
	}
	else
	{
		/* The bit request overlaps two words */
		result = (bs->current_word & bit_mask[bs->bits_left]) >> 
			(32 - bs->bits_left);
		num_bits -= bs->bits_left;
		result <<= num_bits;
		fread(&bs->current_word,4,1,bs->file);
		result |= (bs->current_word & bit_mask[num_bits]) >> (32 - num_bits);
		bs->current_word <<= num_bits;
		bs->bits_left = 32 - num_bits;
	}
	return result;
}

/* Opens a bitstream for use in bitstream_get */
bitstream_t*
bitstream_open(char file_name[])
{
	bitstream_t *bs;
	
	bs = malloc(sizeof(bitstream_t));
	if(!bs)
		return 0;

	/* Read in the first 32 bit word and initialize the structure */
	bs->file = fopen(file_name,"r");
	if(!bs->file)
	{
		free(bs);
		return 0;
	}

	fread(&bs->current_word,4,1,bs->file);
	bs->bits_left = 32;

	return bs;
}


void 
bitstream_close(bitstream_t *bs)
{
	if(!bs)
		return;

	if(bs->file)
		fclose(bs->file);

	free(bs);
}
