# # Process this file with automake to produce Makefile.in.
AUTOMAKE_OPTIONS = 1.3 foreign

CFLAGS = -Wall -Werror -O3 -g

bin_PROGRAMS = ac3dec 

ac3dec_LDADD= ./libac3/.libs/libac3.a -lm 
ac3dec_LDFLAGS = 

noinst_HEADERS = output.h output_wav.h
ac3dec_SOURCES = ac3dec.c output.c output_wav.c
ac3dec_DEPENDENCIES = libac3/.libs/libac3.so


EXTRA_DIST = README Changelog TODO plot_spectrum.m autogen.sh\
       output_linux.c output_irix.c output_solaris.c

SUBDIRS = libac3 tools test .

