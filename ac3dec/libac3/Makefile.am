CFLAGS = @CFLAGS@ @LIBAC3_CFLAGS@

lib_LTLIBRARIES = libac3.la

libac3_la_SOURCES = bitstream.c imdct.c imdct_mlib.c bit_allocate.c parse.c \
		    downmix.c
libac3_la_LIBADD = @LIBAC3_LIBS@ -lm

EXTRA_DIST = configure.incl ac3_internal.h bitstream.h tables.h
