
/*
 *
 * output.c
 *
 * Aaron Holtzman - May 1999
 *
 * Based on original code by Angus Mackay (amackay@gus.ml.org)
 *
 */

int output_open(int bits, int rate, int channels);
void output_play(stream_samples_t *samples);
void output_close(void);
