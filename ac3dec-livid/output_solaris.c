/*
 *
 * output.c
 *
 * Aaron Holtzman - May 1999
 *
 * Based on original code by Angus Mackay (amackay@gus.ml.org)
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/audioio.h>
#include <sys/ioctl.h>


#include "ac3.h"
#include "decode.h"
#include "output.h"

/* Global to keep track of old state */
static char dev[] = "/dev/audio";
static audio_info_t info;
static int fd;
static sint_16 out_buf[1024];

/*
 * open the audio device for writing to
 */
int output_open(int bits, int rate, int channels)
{

  /*
   * Open the device driver:
   */

	fd=open(dev,O_WRONLY | O_NDELAY);
  if(fd < 0) 
  {
    printf("%s: Opening audio device %s\n",
        strerror(errno), dev);
    goto ERR;
  }
	printf("Opened audio device \"%s\"\n",dev);


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

  return 1;

ERR:
  if(fd >= 0) { close(fd); }
  return 0;
}

/*
 * play the sample to the already opened file descriptor
 */
void output_play(stream_samples_t *samples)
{
  int i;
	float max_left = 0.0;
	float max_right =  0.0;
	float left_sample;
	float right_sample;

	if(fd < 0)
		return;

	/* Take the floating point audio data and convert it into
	 * 16 bit signed LE data */

	for(i=0; i < 512; i++)
	{
		left_sample = samples->channel[0][i];
		right_sample = samples->channel[1][i];
		max_left = left_sample > max_left ? left_sample : max_left;
		max_right = right_sample > max_right ? right_sample : max_right;

		//FIXME gain is high
		out_buf[i * 2] = left_sample * 65536.0 * 1000.0;
		out_buf[i * 2 + 1] = right_sample * 65536.0 * 1000.0;

		//fprintf(stderr,"lsample = %1.6e rsample = %1.6e\n",left_sample,right_sample);
	}

	//FIXME remove
	//printf("max_left = %f max_right = %f\n",max_left,max_right);

	if(write(fd, out_buf,1024) != 1024)
	{
		fprintf(stderr, "write on %d: %s\n", fd, strerror(errno));
		exit(1);
	}

#if 0
  char *p;
	uint_t bufsize = info.play.buffer_size;
  p = out_buf;

  for(i=0; i<1024; i+=bufsize)
  {
    n = (size - i < bufsize) ? (size - i) : bufsize;

  }
#endif
}


void
output_close(void)
{
	/* Reset the saved parameters */

  if(ioctl(fd, AUDIO_SETINFO, &info) < 0)
  {
    fprintf(stderr, "%s: Writing audio config block\n",strerror(errno));
  }

	close(fd);

}
