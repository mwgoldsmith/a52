#ifndef __DOWNMIX_I386_H__
#define __DOWNMIX_I386_H__

void stream_sample_2ch_to_s16_i386(int16_t *s16_samples, float *left, float *right);
void stream_sample_1ch_to_s16_i386(int16_t *s16_samples, float *center);      

#endif
