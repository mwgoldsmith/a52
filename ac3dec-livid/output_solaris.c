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
#include <stropts.h>
#include <signal.h>

#include "ac3.h"
#include "decode.h"
#include "output.h"
#include "ring_buffer.h"


#define BUFFER_SIZE 1024 

/* Global to keep track of old state */
static char dev[] = "/dev/audio";
static audio_info_t info;
static int fd;
//FIXME remove
//#include "matlab.h"
//static matlab_file_t *foo;
//


/*
 * open the audio device for writing to
 */
int output_open(int bits, int rate, int channels)
{

  /*
   * Open the device driver
   */

	fd=open(dev,O_WRONLY | O_NDELAY);
  if(fd < 0) 
  {
    fprintf(stderr,"%s: Opening audio device %s\n",
        strerror(errno), dev);
    goto ERR;
  }
	fprintf(stderr,"Opened audio device \"%s\"\n",dev);

	fcntl(fd,F_SETFL,O_NONBLOCK);

	/* Setup our parameters */
	AUDIO_INITINFO(&info);

	info.play.sample_rate = rate;
	info.play.precision = bits;
	info.play.channels = channels;
	info.play.buffer_size = 2048;
	info.play.encoding = AUDIO_ENCODING_LINEAR;
	info.play.port = AUDIO_SPEAKER;
	info.play.gain = 110;

	/* Write our configuration */
	/* An implicit GETINFO is also performed so we can get
	 * the buffer_size */

  if(ioctl(fd, AUDIO_SETINFO, &info) < 0)
  {
    fprintf(stderr, "%s: Writing audio config block\n",strerror(errno));
    goto ERR;
  }

	fprintf(stderr,"buffer_size = %d\n",info.play.buffer_size);

	/* Initialize the ring buffer */
	rb_init();

	//FIXME remove
//	foo = matlab_open("foo.m");
	//
	

	return 1;

ERR:
  if(fd >= 0) { close(fd); }
  return 0;
}

static void
output_flush(void)
{
	int i,j = 0;
	sint_16 *out_buf = 0;

	i = 0;

	do
	{
		out_buf = rb_begin_read();
		if(out_buf)
			i = write(fd, out_buf,BUFFER_SIZE);
		else
			break;

		if(i == BUFFER_SIZE)
		{
			rb_end_read();
			j++;
		}
	}
	while(i == BUFFER_SIZE);
	
	//FIXME remove
	//fprintf(stderr,"(output) Flushed %d blocks, wrote %d bytes last frame\n",j,i);
}

/*
 * play the sample to the already opened file descriptor
 */
void output_play(stream_samples_t *samples)
{
  int i;
	float left_sample;
	float right_sample;
	sint_16 *out_buf;

	if(fd < 0)
		return;

	out_buf = rb_begin_write();

	/* Keep trying to dump frames from the ring buffer until we get a 
	 * write slot available */
	while(!out_buf)
	{
		output_flush();
		usleep(1000);
		out_buf = rb_begin_write();
	} 

	//FIXME remove
//	matlab_write(foo,samples->channel[0],512);
	
	/* Take the floating point audio data and convert it into
	 * 16 bit signed PCM data */

	for(i=0; i < 256; i++)
	{
		sint_16 left_pcm;
		sint_16 right_pcm;

		left_sample = samples->channel[0][i];
		right_sample = samples->channel[1][i];

		left_pcm = left_sample * 32000.0;
		out_buf[i * 2 ] = left_pcm;
		right_pcm = right_sample * 32000.0;
		out_buf[i * 2 + 1] = right_pcm;
	}
	rb_end_write();

	output_flush();
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
