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
//#include "libao/audio_out.h"

#define BLOCK_SIZE 2048
uint_8 buf[BLOCK_SIZE];

FILE *in_file;

ao_functions_t *audio_out;

void
print_usage(char *argv[])
{
	uint_32 i = 0;

	fprintf(stderr,"usage:  %s [-o mode] foo.ac3\n"
				 "\t-o\taudio output mode\n",argv[0]);

	while (audio_out_drivers[i] != NULL)
	{              
		const ao_info_t *info;

		info = audio_out_drivers[i++]->get_info();

		fprintf(stderr, "\t\t\t%s\t%s\n", info->short_name,
		info->name);
	}

	exit(1);
}


void 
handle_args(int argc,char *argv[])
{
	char c;
	uint_32 i;

	//default to the first driver in the list
	audio_out = audio_out_drivers[0];

	while((c = getopt(argc,argv,"o:")) != EOF)
	{
		switch(c)
		{
			case 'o':
			
				for (i=0; audio_out_drivers[i] != NULL; i++)
				{
					const ao_info_t *info = audio_out_drivers[i]->get_info();

					if (strcmp(info->short_name,optarg) == 0)
						audio_out = audio_out_drivers[i];
				}

				if (audio_out_drivers[i] == NULL)
				{
					fprintf(stderr,"Invalid audio driver: %s\n", optarg);
					print_usage(argv);
				}

			break;

			default:
				print_usage(argv);
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

	ac3_init(&ac3_config, audio_out);

	while((bytes_read = fread(buf,1,BLOCK_SIZE,in_file)) == BLOCK_SIZE)
		ac3_decode_data(buf,buf + BLOCK_SIZE);

	fclose(in_file);
	return 0;
}
