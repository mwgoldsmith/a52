/*
 *
 * debug.h
 *
 * Aaron Holtzman - May 1999
 *
 *
 */

int debug_is_on(void);

#ifdef __GNUC__
#define dprintf(format,args...)\
{\
	if (debug_is_on())\
	{\
		fprintf(stderr,format,## args);\
	}\
}
#endif

void dprintf(char fmt[],...);
