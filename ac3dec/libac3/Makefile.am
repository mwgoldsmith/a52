CFLAGS = @CFLAGS@ @LIBAC3_CFLAGS@

lib_LTLIBRARIES = libac3.la

libac3_la_SOURCES = bitstream.c decode.c imdct.c imdct_mlib.c \
		    stats.c bit_allocate.c  parse.c debug.c rematrix.c \
		    downmix.c dither.c sanity_check.c
libac3_la_LIBADD = @LIBAC3_LIBS@ -lm

EXTRA_DIST = ac3_internal.h bitstream.h decode.h imdct.h ac3.h stats.h \
	     bit_allocate.h parse.h debug.h rematrix.h downmix.h \
	     dither.h sanity_check.h
