noinst_PROGRAMS = bitstream_test imdct_test

LDFLAGS = -lm
bitstream_test_SOURCES = bitstream_test.c
imdct_test_SOURCES = imdct_test.c
bitstream_test_LDADD = ../bitstream.o ../crc.o
imdct_test_LDADD = ../imdct.o

check:
	rm -f bitstream_test.out
	./bitstream_test > bitstream_test.out
	diff bitstream_test.out bitstream_test.out.ref
