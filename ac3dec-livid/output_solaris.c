/*
 * Copyright (C) 1998 Angus Mackay. All rights reserved.
 * See the file LICENSE for details.
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef HAVE_SYS_IOCTL_H
#  error ioctl not found
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/audioio.h>
#include <sys/ioctl.h>


#include "play.h"
#include "sinus.h"

/* Global to keep track of old state */
audio_info_t info;

/*
 * open the audio device for writing to
 */
int open_audio(char *dev, int bits, int rate, int channels)
{
  int fd = -1;

  /*
   * Open the device driver:
   */

	if(dev)
	{
		fd=open(dev,O_WRONLY);
		printf("Opening audio device \"%s\"\n",dev);
	}
	else
	{
		fd=open("/dev/audio",O_WRONLY);
		printf("Opening audio device \"/dev/audio\"\n",dev);
	}

  if(fd < 0) 
  {
    printf("%s: Opening audio device %s\n",
        strerror(errno), dev ? dev : "/dev/audio");
    goto ERR;
  }

	/* Setup our parameters */
	AUDIO_INITINFO(&info);

	info.play.sample_rate = rate;
	info.play.precision = bits;
	info.play.channels = channels;
	info.play.encoding = AUDIO_ENCODING_LINEAR;
	info.play.port = AUDIO_SPEAKER;

	/* Write our configuration */
	/* An implicit GETINFO is also performed so we can get
	 * the buffer_size */

  if(ioctl(fd, AUDIO_SETINFO, &info) < 0)
  {
    fprintf(stderr, "%s: Writing audio config block\n",strerror(errno));
    goto ERR;
  }

	printf("buffer_size = %d\n",info.play.buffer_size);

  return(fd);

ERR:
  if(fd >= 0) { close(fd); }
  return(-1);
}

/*
 * play the sample to the already opened file descriptor
 */
void play(int fd, void *data, int size)
{
  int i;
  char *p;
  int n;
	uint_t bufsize = info.play.buffer_size;


  p = data;

  dprintf((stderr, "p: %p -> %p\n", p, &(p[size])));
  dprintf((stderr, "size: %06X\n", size));
  dprintf((stderr, "bufsize: %06X\n", bufsize));

  for(i=0; i<size; i+=bufsize)
  {
    n = (size - i < bufsize) ? (size - i) : bufsize;
    /* dprintf((stderr, "&(p[%06X]): %p, n: %X\n", i, &(p[i]), n)); */
    if(write(fd, &(p[i]), n) != n)
    {
      dprintf((stderr, "write on %d: %s\n", fd, strerror(errno)));
      fprintf(stderr, "write on %d: %s\n", fd, strerror(errno));
      exit(1);
    }
  }
}


void
close_audio(int fd)
{
	/* Reset the saved parameters */

  if(ioctl(fd, AUDIO_SETINFO, &info) < 0)
  {
    fprintf(stderr, "%s: Writing audio config block\n",strerror(errno));
  }

	close(fd);

}
