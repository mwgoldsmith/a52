/*
 *
 * debug.c
 *
 * Aaron Holtzman - May 1999
 *
 *
 */

#include <stdlib.h>
#include "debug.h"

static int debug_level = -1;

// Determine is debug output is required.
// We could potentially have multiple levels of debug info
int debug_is_on(void)
{
	char *env_var;
	
	if(debug_level < 0)
	{
	  env_var = getenv("AC3_DEBUG");

		if (env_var)
		{
			debug_level = 1;
		}
		else
			debug_level = 0;
	}
	
	return debug_level;
}

//If you don't have gcc, then ya don't get debug output
#ifndef __GNUC__
void dprintf(char fmt[],...)
{
	int foo = 0;
}
#endif
