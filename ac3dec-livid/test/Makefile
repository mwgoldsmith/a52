CC=gcc
CFLAGS = -Wall -g -I..
BITSTREAM_OBJS = bitstream_test.o ../bitstream.o
IMDCT_OBJS = imdct_test.o ../imdct.o
TARGET = bitstream_test imdct_test

all : $(TARGET)
bitstream_test : $(BITSTREAM_OBJS)
imdct_test : $(IMDCT_OBJS)
	gcc -o imdct_test $(IMDCT_OBJS) -lm
