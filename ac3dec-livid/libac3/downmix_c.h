#ifndef __DOWNMIX_C_H__
#define __DOWNMIX_C_H__

void downmix_3f_2r_to_2ch_c(float *samples, dm_par_t * dm_par);
void downmix_3f_1r_to_2ch_c(float *samples, dm_par_t * dm_par);
void downmix_2f_2r_to_2ch_c(float *samples, dm_par_t * dm_par);
void downmix_2f_1r_to_2ch_c(float *samples, dm_par_t * dm_par);
void downmix_3f_0r_to_2ch_c(float *samples, dm_par_t * dm_par);            
void stream_sample_2ch_to_s16_c(int16_t *s16_samples, float *left, float *right);
void stream_sample_1ch_to_s16_c(int16_t *s16_samples, float *center);      

#endif
