/* 
 *  bitstream.h
 *
 *	Aaron Holtzman - May 1999
 *
 */

typedef struct bitstream_s
{
	FILE *file;
	uint_32 current_word;
	uint_32 bits_left;
	uint_32 total_bits_read;
} bitstream_t;

uint_32 bitstream_get(bitstream_t *bs,uint_32 num_bits);
bitstream_t* bitstream_open(char file_name[]);
void bitstream_close(bitstream_t *bs);
