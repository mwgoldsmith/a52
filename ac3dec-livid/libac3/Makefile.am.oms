##
## Process this file with automake to produce Makefile.in
##

INCLUDES += -I$(top_srcdir)/include

libdir = $(PLUGIN_DIR)

## Libs
if ARCH_X86
	LIBAC3DEC_OBJS="downmix_i386.lo"
else
	LIBAC3DEC_OBJS="downmix.lo"
endif

lib_LTLIBRARIES = codec_ac3dec.la

#codec_ac3dec_la_LIBADD = $(LIBAC3DEC_OBJS)
#codec_ac3dec_la_DEPENDENCIES = $(LIBAC3DEC_OBJS)
codec_ac3dec_la_LIBADD = downmix.lo
codec_ac3dec_la_DEPENDENCIES = downmix.lo
codec_ac3dec_la_CFLAGS = @MM_CFLAGS@
codec_ac3dec_la_LDFLAGS = -avoid-version -module Wl,-soname,codec_ac3dec.so -lm @MM_LIBS@
codec_ac3dec_la_SOURCES = bitstream.c codec.c imdct.c coeff.c exponent.c stats.c bit_allocate.c  parse.c crc.c debug.c rematrix.c dither.c sanity_check.c srfft.c

EXTRA_codec_ac3dec_la_SOURCES = downmix.c downmix_i386.S

noinst_HEADERS = ac3_internal.h bitstream.h imdct.h stats.h coeff.h exponent.h bit_allocate.h parse.h crc.h debug.h rematrix.h downmix.h dither.h sanity_check.h ac3.h srfft.h srfftp.h
