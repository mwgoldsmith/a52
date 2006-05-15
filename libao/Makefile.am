# # Process this file with automake to produce Makefile.in.
AUTOMAKE_OPTIONS = 1.3 foreign

CFLAGS = -Wall -Werror -O3 -g @LIBAO_CFLAGS@

lib_LTLIBRARIES = libao.la

include_HEADERS = audio_out.h 

libao_la_LIBADD= @LIBAO_CONFIG_LIBS@  
libao_la_DEPENDENCIES= @LIBAO_CONFIG_LIBS@  
libao_la_SOURCES = audio_out.c audio_out_wav.c audio_out_null.c

EXTRA_DIST = audio_out_linux.c audio_out_irix.c audio_out_solaris.c
