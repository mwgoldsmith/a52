#ifndef __IMDCT_C_H__
#define __IMDCT_C_H__

#include "cmplx.h"

int imdct_init_c (void);

void fft_128p_c (complex_t *);
void fft_64p_c (complex_t *);

void imdct512_pre_ifft_twiddle_c (const int *pmt, complex_t *buf, float *data, float *xcos_sin_sse);
void imdct512_post_ifft_twiddle_c (complex_t *buf, float *xcos_sin_sse);
void imdct512_window_delay_c (complex_t *buf, float *data_ptr, float *window_prt, float *delay_prt);
void imdct512_window_delay_nol_c (complex_t *buf, float *data_ptr, float *window_prt, float *delay_prt);

#endif
