# # Process this file with automake to produce Makefile.in.
AUTOMAKE_OPTIONS = 1.3 foreign

CFLAGS = -Wall -Werror -O3 -g -Ilibac3 -Ilibao

bin_PROGRAMS = ac3dec 

ac3dec_LDADD= libac3/libac3.la libao/libao.la -lm 
ac3dec_LDFLAGS = 

ac3dec_SOURCES = ac3dec.c 
ac3dec_DEPENDENCIES = libac3/libac3.la libao/libao.la

EXTRA_DIST = README Changelog TODO plot_spectrum.m autogen.sh
       

SUBDIRS = libao libac3 tools test .

