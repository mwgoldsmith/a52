##
## Process this file with automake to produce Makefile.in
##

INCLUDES += -I$(top_srcdir)/include
libdir = $(PLUGIN_DIR)

SOURCE_KNI = srfft_kni_c.c srfft_kni.S downmix_kni.S imdct512_kni.S imdct_kni.c
HEADER_KNI = imdct_kni.h downmix_kni.h srfft_kni.h
SOURCE_X86 = downmix_i386.S
HEADER_X86 = downmix_i386.h

## Libs
noinst_LTLIBRARIES = libac3.la

AM_CFLAGS = @MM_CFLAGS@
libac3_la_LDFLAGS = -version-info 1:6:1 -lm @MM_LIBS@
libac3_la_SOURCES = bitstream.c bit_allocate.c decode.c coeff.c exponent.c stats.c parse.c crc.c debug.c rematrix.c dither.c sanity_check.c srfft.c imdct.c imdct_c.c downmix.c downmix_c.c ${SOURCE_X86}

noinst_HEADERS = ac3_internal.h bitstream.h imdct.h imdct_c.h stats.h coeff.h exponent.h bit_allocate.h parse.h crc.h debug.h rematrix.h downmix.h downmix_c.h dither.h sanity_check.h ac3.h srfft.h srfftp.h mm_accel.h cmplx.h decode.h bswap.h ${HEADER_X86}
