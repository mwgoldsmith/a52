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

#define BLOCK_SIZE 2048
uint_8 buf[BLOCK_SIZE];

FILE *in_file;

ao_functions_t ac3_output;

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

ao_functions_t output_null =
{
	output_open_null,
	output_play_null,
	output_close_null
};


void 
handle_args(int argc,char *argv[])
{
	char c;

	ac3_output = output_norm;

	while((c = getopt(argc,argv,"nw")) != EOF)
	{
		switch(c)
		{
			case 'n':
				ac3_output = output_null;
			break;

			case 'w':
				ac3_output= output_wav;
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
	ac3_config_t ac3_config;
	uint_32 bytes_read;

	handle_args(argc,argv);

	ac3_config.num_output_ch = 2;
	ac3_config.flags = 0;

	ac3_init(&ac3_config, &ac3_output);

	while((bytes_read = fread(buf,1,BLOCK_SIZE,in_file)) == BLOCK_SIZE)
		ac3_decode_data(buf,buf + BLOCK_SIZE);

	fclose(in_file);
	return 0;
}
