/* 
 *   ac3dec.c
 *
 *	Copyright (C) Aaron Holtzman - May 1999
 *
 *  This file is part of ac3dec, a free Dolby AC-3 stream decoder.
 *	
 *  ac3dec is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  ac3dec is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <errno.h>

#include "libac3/ac3.h"
#include "output.h"
#include "output_wav.h"

#define CHUNK_SIZE 2048
uint_8 buf[CHUNK_SIZE];
FILE *in_file;
uint_32 (*ac3dec_output_open)(uint_32,uint_32,uint_32);
void (*ac3dec_output_play)(sint_16*,uint_32);
void (*ac3dec_output_close)(void);
 
void fill_buffer(uint_8 **start,uint_8 **end)
{
	uint_32 bytes_read;

	*start = buf;

	bytes_read = fread(*start,1,CHUNK_SIZE,in_file);

	//FIXME hack...
	if(bytes_read < CHUNK_SIZE)
	{
		ac3dec_output_close();
		exit(0);
	}

	*end= *start + bytes_read;
}

void
output_close_null(void)
{
}

uint_32
output_open_null(uint_32 bits,uint_32 rate,uint_32 channels)
{
	//do nothing
	return 0;
}

void
output_play_null(sint_16 *foo,uint_32 bar)
{
	//do nothing
}

void 
handle_args(int argc,char *argv[])
{
	char c;

	ac3dec_output_open  = output_open;
	ac3dec_output_play = output_play;
	ac3dec_output_close  = output_close;

	while((c = getopt(argc,argv,"nw")) != EOF)
	{
		switch(c)
		{
			case 'n':
				ac3dec_output_open  = output_open_null;
				ac3dec_output_play = output_play_null;
				ac3dec_output_close = output_close_null;
			break;

			case 'w':
				ac3dec_output_open  = output_open_wav;
				ac3dec_output_play = output_play_wav;
				ac3dec_output_close = output_close_wav;
			break;

			default:
				printf("usage:  %s [-n|-w] foo.ac3\n"
						   "        -n         no audio output (for testing)\n"
						   "        -w         .wav output (to output.wav)\n",argv[0]);
				exit(1);
		}
	}

	// If we get an argument then use it as a filename... otherwise use stdin 
	if(optind < argc)
	{
		in_file = fopen(argv[optind],"r");	
		if(!in_file)
		{
			fprintf(stderr,"%s - Couldn't open file %s\n",strerror(errno),argv[1]);
			exit(1);
		}
	}
	else
		in_file = stdin;
}

int main(int argc,char *argv[])
{
	ac3_frame_t *ac3_frame;
	ac3_config_t ac3_config;

	handle_args(argc,argv);

	ac3_config.fill_buffer_callback = fill_buffer;
	ac3_config.num_output_ch = 2;
	ac3_config.flags = 0;

	ac3_init(&ac3_config);
	
	ac3_frame = ac3_decode_frame();
	ac3dec_output_open(16,ac3_frame->sampling_rate,2);

	do
	{
		//Send the samples to the output device 
		ac3dec_output_play(ac3_frame->audio_data, 256 * 6 * 2);
	}
	while((ac3_frame = ac3_decode_frame()));

	ac3dec_output_close();
	fclose(in_file);
	return 0;
}
