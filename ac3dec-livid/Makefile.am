# # Process this file with automake to produce Makefile.in.
AUTOMAKE_OPTIONS = 1.3 foreign

CFLAGS = -Wall -Werror -O3 -g 

bin_PROGRAMS = decode

decode_LDFLAGS = -lm

decode_SOURCES = bitstream.c coeffs.c decode.c imdct.c
noinst_HEADERS = bitstream.h coeffs.h decode.h imdct.h ac3.h

SUBDIRS = . test

