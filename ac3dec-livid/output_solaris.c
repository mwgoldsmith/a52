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

	fd=open(dev,O_WRONLY );
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
	info.play.buffer_size = 2048;
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
	int bytes_left = 0;
	int bytes_written = 0;

	if(fd < 0)
		return;

	/* Take the floating point audio data and convert it into
	 * 16 bit signed PCM data */


	for(i=0; i < 512; i++)
	{
		sint_16 left_pcm;
		sint_16 right_pcm;

		left_sample = samples->channel[0][i];
		right_sample = samples->channel[1][i];
		max_left = left_sample > max_left ? left_sample : max_left;
		max_right = right_sample > max_right ? right_sample : max_right;

		//FIXME gain too high
		left_pcm = left_sample * 60000.0;
		out_buf[i * 2 ] = left_pcm;
		right_pcm = right_sample * 60000.0;
		out_buf[i * 2 + 1] = right_pcm;

		//fprintf(stderr,"lsample = %1.6e rsample = %1.6e\n",left_sample,right_sample);
	}

	//FIXME remove
	//printf("max_left = %f max_right = %f\n",max_left,max_right);

	bytes_left = 2048;

	bytes_written = write(fd, out_buf,bytes_left);
#if 0
	while (bytes_left)
	{
		bytes_written = write(fd, out_buf,bytes_left);
		bytes_left -= bytes_written;
		if(bytes_left > 0)
			usleep(5);
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
