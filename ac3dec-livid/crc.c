/* 
 *    crc.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "crc.h"

uint_16 state;

static inline void crc_process_bit(uint_32 bit);

void
crc_init(void)
{
	state = 0;
}

static inline void
crc_process_bit(uint_32 bit)
{
	uint_32 xor_val;

	xor_val = (state >> 15) ^ bit;

	state <<= 1;
	
	if(xor_val)
		state = (state ^ 0x8005);
}


void
crc_process(uint_32 data, uint_32 num_bits)
{
	data <<= 32 - num_bits;

	while(num_bits)
	{
		crc_process_bit(data >> 31);

		data <<= 1;
		num_bits--;
	}
}

int
crc_validate(void)
{
	return(state == 0);
}


