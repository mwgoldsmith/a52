CFLAGS = @CFLAGS@ @LIBAC3_CFLAGS@

lib_LTLIBRARIES = libac3.la

libac3_la_SOURCES = bitstream.c imdct.c imdct_mlib.c bit_allocate.c parse.c \
		    downmix.c dither.c
libac3_la_LIBADD = @LIBAC3_LIBS@ -lm

EXTRA_DIST = ac3_internal.h bitstream.h imdct.h ac3.h \
	     bit_allocate.h downmix.h dither.h
