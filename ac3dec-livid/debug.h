/*
 *
 * debug.h
 *
 * Aaron Holtzman - May 1999
 *
 *
 */

int debug_is_on(void);

#define dprintf(format,args...)\
{\
	if (debug_is_on())\
	{\
		fprintf(stderr,format,## args);\
	}\
}

