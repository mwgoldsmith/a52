#ifndef __IMDCT_KNI_H__
#define __IMDCT_KNI_H__

#include "cmplx.h"

int imdct_init_kni (void);

void fft_128p_kni(complex_t *);
void fft_64p_kni(complex_t *);

void imdct512_pre_ifft_twiddle_kni(const int *pmt, complex_t *buf, float *data, float *xcos_sin_sse);
void imdct512_post_ifft_twiddle_kni(complex_t *buf, float *xcos_sin_sse);
void imdct512_window_delay_kni(complex_t *buf, float *data_ptr, float *window_prt, float *delay_prt);
void imdct512_window_delay_nol_kni(complex_t *buf, float *data_ptr, float *window_prt, float *delay_prt);

#endif
