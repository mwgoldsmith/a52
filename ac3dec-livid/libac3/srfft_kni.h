#ifndef __SRFF_KNI__
#define __SRFF_KNI__

#include "cmplx.h"

void fft_4_kni (complex_t *a);
void fft_8_kni (complex_t *a);
void fft_asmb_kni (int, complex_t*, complex_t *, complex_t *, complex_t*);

#endif
