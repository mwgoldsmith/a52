##
## Process this file with automake to produce Makefile.in
##

CFLAGS = -Wall -Werror -O2 -g -I../libao -idirafter ../compatibility

lib_LTLIBRARIES = libac3.la

include_HEADERS = ac3.h

libac3_la_LDFLAGS = -version-info 1:6:1 -export-symbols libac3.sym -lm

libac3_la_SOURCES =\
	bit_allocate.c\
	bitstream.c\
	coeff.c\
	cpu_accel.c\
	crc.c\
	debug.c\
	decode.c\
	dither.c\
	downmix.c\
	downmix_c.c\
	downmix_i386.S\
	downmix_kni.S\
	exponent.c\
	imdct.c\
	imdct_c.c\
	imdct_kni.c\
	imdct512_kni.S\
	parse.c\
	rematrix.c\
	sanity_check.c\
	srfft.c\
	srfft_kni_c.c\
	srfft_kni.S\
	stats.c


noinst_HEADERS =\
	ac3_internal.h\
	bit_allocate.h\
	bitstream.h\
	bswap.h\
	coeff.h\
	crc.h\
	debug.h\
	dither.h\
	downmix.h\
	downmix_c.h\
	downmix_i386.h\
	downmix_kni.h\
	exponent.h\
	decode.h\
	imdct.h\
	imdct_c.h\
	imdct_kni.h\
	mm_accel.h\
	parse.h\
	rematrix.h\
	sanity_check.h\
	srfft.h\
	srfftp.h\
	stats.h

