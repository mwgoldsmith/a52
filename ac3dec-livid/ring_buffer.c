/*
 *
 * ring_buffer.c
 *
 * Aaron Holtzman - June 1999
 *
 * Based on original code by Angus Mackay (amackay@gus.ml.org)
 *
 */

//FIXME separate the typedefs from ac3.h
typedef signed short sint_16;

#include <unistd.h>
#include "ring_buffer.h"

#define BUFFER_SIZE 2048
#define NUM_BUFFERS 64
#define SLEEP_USECS 1000
#define INC_INDEX(x) (((x) + 1) % NUM_BUFFERS)

// The buffer that was just written into
static volatile int write_index;
// The buffer that was just read from
static volatile int read_index;
static sint_16 ring_buf[NUM_BUFFERS][BUFFER_SIZE];

void
rb_init(void)
{
	write_index = 0;
	read_index = 0;
}

sint_16* rb_begin_read(void)
{
	while(read_index == write_index)
		usleep(SLEEP_USECS);
	return ring_buf[INC_INDEX(read_index)];
}

void rb_end_read(void)
{
	read_index = INC_INDEX(read_index);	
}

sint_16* rb_begin_write(void)
{
	while(read_index == INC_INDEX(write_index))
		usleep(SLEEP_USECS);
	return ring_buf[INC_INDEX(write_index)];
}

void rb_end_write(void)
{
	write_index = INC_INDEX(write_index);	
}
