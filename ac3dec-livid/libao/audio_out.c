/*
 *
 *  audio_out.c 
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
 *  This file is based on output_linux.c by Aaron Holtzman.
 *  All .wav modifications were done by Jorgen Lundman <lundman@lundman.net>
 *  Any .wav bugs and errors should be reported to him.
 *
 *
 */

#include <stdlib.h>
#include "audio_out.h"

extern ao_functions_t audio_out_norm;
extern ao_functions_t audio_out_wav;
extern ao_functions_t audio_out_null;

ao_functions_t* audio_out_drivers[] = 
{
	&audio_out_norm,
	&audio_out_wav,
	&audio_out_null,
	NULL
};
