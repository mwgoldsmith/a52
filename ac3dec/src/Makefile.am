CFLAGS = @CFLAGS@ @AC3DEC_CFLAGS@

bin_PROGRAMS = ac3dec extract_ac3
ac3dec_SOURCES = ac3dec.c getopt.c
ac3dec_LDADD = $(top_builddir)/libac3/libac3.la $(top_builddir)/libao/libao.la
extract_ac3_SOURCES = extract_ac3.c getopt.c

EXTRA_DIST = configure.incl getopt.h
