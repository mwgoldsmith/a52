/* vc++/config.h - manually adapted from include/config.h.in */

/* a52dec profiling */
/* #undef A52DEC_GPROF */

/* maximum supported data alignment */
/* #undef ATTRIBUTE_ALIGNED_MAX */

/* Define if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define if you have the `memalign' function. */
/* #undef HAVE_MEMALIGN */

/* Define if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define if you have the <sys/time.h> header file. */
/* #undef HAVE_SYS_TIME_H */

/* a52 sample precision */
/* #undef LIBA52_DOUBLE */

/* liba52 mlib support */
/* #undef LIBA52_MLIB */

/* libao OSS support */
/* #undef LIBAO_OSS */

/* libao solaris support */
/* #undef LIBAO_SOLARIS */

/* Name of package */
#define PACKAGE "a52dec"

/* Define as the return type of signal handlers (`int' or `void'). */
/* #undef RETSIGTYPE */

/* Version number of package */
#define VERSION "0.7.2-cvs"

/* Define if your processor stores words with the most significant byte first
   (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define as `__inline' if that's what the C compiler calls it, or to nothing
   if it is not supported. */
#define inline __inline
