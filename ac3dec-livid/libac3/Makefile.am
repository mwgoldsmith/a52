##
## Process this file with automake to produce Makefile.in
##

CFLAGS = -Wall -Werror -O2 -g -I../libao -idirafter ../compatibility

lib_LTLIBRARIES = libac3.la

include_HEADERS = ac3.h

libac3_la_LDFLAGS = -version-info 1:6:1 -export-symbols libac3.sym -lm

libac3_la_SOURCES = bitstream.c decode.c imdct.c coeff.c exponent.c stats.c\
	bit_allocate.c  parse.c crc.c debug.c rematrix.c downmix.c dither.c\
	sanity_check.c srfft.c

noinst_HEADERS = ac3_internal.h bitstream.h decode.h imdct.h stats.h coeff.h\
	exponent.h bit_allocate.h parse.h crc.h debug.h rematrix.h downmix.h\
	dither.h sanity_check.h srfft.h srfftp.h
