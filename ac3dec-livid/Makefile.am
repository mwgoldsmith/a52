# # Process this file with automake to produce Makefile.in.
AUTOMAKE_OPTIONS = 1.3 foreign

CFLAGS = -Wall -Werror -O3 -g 

bin_PROGRAMS = decode

decode_LDFLAGS = -lm

decode_SOURCES = bitstream.c decode.c imdct.c unpack.c stats.c
noinst_HEADERS = bitstream.h decode.h imdct.h ac3.h unpack.h stats.h


SUBDIRS = . test

