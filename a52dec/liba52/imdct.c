/*
 * imdct.c
 * Copyright (C) 2000-2001 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of a52dec, a free ATSC A-52 stream decoder.
 * See http://liba52.sourceforge.net/ for updates.
 *
 * a52dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * a52dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <math.h>
#include <stdio.h>
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795029
#endif
#include <inttypes.h>

#include "a52.h"
#include "a52_internal.h"
#include "mm_accel.h"

void (* a52_imdct_256) (sample_t data[], sample_t delay[], sample_t bias);
void (* a52_imdct_512) (sample_t data[], sample_t delay[], sample_t bias);

typedef struct complex_s {
    sample_t real;
    sample_t imag;
} complex_t;

static complex_t buf[128];

static uint8_t fftorder_128[] = {
      0, 64, 32, 96, 16, 80,112, 48,  8, 72, 40,104,120, 56, 24, 88,
      4, 68, 36,100, 20, 84,116, 52,124, 60, 28, 92, 12, 76,108, 44,
      2, 66, 34, 98, 18, 82,114, 50, 10, 74, 42,106,122, 58, 26, 90,
    126, 62, 30, 94, 14, 78,110, 46,  6, 70, 38,102,118, 54, 22, 86,
      1, 65, 33, 97, 17, 81,113, 49,  9, 73, 41,105,121, 57, 25, 89,
      5, 69, 37,101, 21, 85,117, 53,125, 61, 29, 93, 13, 77,109, 45,
    127, 63, 31, 95, 15, 79,111, 47,  7, 71, 39,103,119, 55, 23, 87,
      3, 67, 35, 99, 19, 83,115, 51,123, 59, 27, 91, 11, 75,107, 43
};

static uint8_t fftorder_64[] = {
      0, 32, 16, 48,  8, 40, 56, 24,  4, 36, 20, 52, 60, 28, 12, 44,
      2, 34, 18, 50, 10, 42, 58, 26, 62, 30, 14, 46,  6, 38, 54, 22,
      1, 33, 17, 49,  9, 41, 57, 25,  5, 37, 21, 53, 61, 29, 13, 45,
     63, 31, 15, 47,  7, 39, 55, 23,  3, 35, 19, 51, 59, 27, 11, 43
};

/* Twiddle factors for IMDCT */
static sample_t xcos1[128];
static sample_t xsin1[128];
static sample_t xcos2[64];
static sample_t xsin2[64];

static sample_t roots16[] = { 0.9238795325112867561281831893967882868224,
			      0.7071067811865475244008443621048490392848,
			      0.3826834323650897717284599840303988667613 };
static sample_t roots32[] = { 0.9807852804032304491261822361342390369739,
			      0.9238795325112867561281831893967882868224,
			      0.8314696123025452370787883776179057567385,
			      0.7071067811865475244008443621048490392848,
			      0.5555702330196022247428308139485328743749,
			      0.3826834323650897717284599840303988667613,
			      0.1950903220161282678482848684770222409276 };
static sample_t roots64[] = { 0.9951847266721968862448369531094799215754,
			      0.9807852804032304491261822361342390369739,
			      0.9569403357322088649357978869802699694828,
			      0.9238795325112867561281831893967882868224,
			      0.8819212643483550297127568636603883495084,
			      0.8314696123025452370787883776179057567385,
			      0.7730104533627369608109066097584698009710,
			      0.7071067811865475244008443621048490392848,
			      0.6343932841636454982151716132254933706757,
			      0.5555702330196022247428308139485328743749,
			      0.4713967368259976485563876259052543776574,
			      0.3826834323650897717284599840303988667613,
			      0.2902846772544623676361923758173952746914,
			      0.1950903220161282678482848684770222409276,
			      0.0980171403295606019941955638886418458611 };
static sample_t roots128[] = {0.9987954562051723927147716047591006944432,
			      0.9951847266721968862448369531094799215754,
			      0.9891765099647809734516737380162430639837,
			      0.9807852804032304491261822361342390369739,
			      0.9700312531945439926039842072861002514568,
			      0.9569403357322088649357978869802699694828,
			      0.9415440651830207784125094025995023571856,
			      0.9238795325112867561281831893967882868224,
			      0.9039892931234433315862002972305370487101,
			      0.8819212643483550297127568636603883495084,
			      0.8577286100002720699022699842847701370425,
			      0.8314696123025452370787883776179057567385,
			      0.8032075314806449098066765129631419238795,
			      0.7730104533627369608109066097584698009710,
			      0.7409511253549590911756168974951627297289,
			      0.7071067811865475244008443621048490392848,
			      0.6715589548470184006253768504274218032287,
			      0.6343932841636454982151716132254933706757,
			      0.5956993044924333434670365288299698895119,
			      0.5555702330196022247428308139485328743749,
			      0.5141027441932217265936938389688157726080,
			      0.4713967368259976485563876259052543776574,
			      0.4275550934302820943209668568887985343045,
			      0.3826834323650897717284599840303988667613,
			      0.3368898533922200506892532126191475704777,
			      0.2902846772544623676361923758173952746914,
			      0.2429801799032638899482741620774711183209,
			      0.1950903220161282678482848684770222409276,
			      0.1467304744553617516588501296467178197062,
			      0.0980171403295606019941955638886418458611,
			      0.0490676743274180142549549769426826583147 };

/* Windowing function for Modified DCT - Thank you acroread */
sample_t a52_imdct_window[] = {
    0.00014, 0.00024, 0.00037, 0.00051, 0.00067, 0.00086, 0.00107, 0.00130,
    0.00157, 0.00187, 0.00220, 0.00256, 0.00297, 0.00341, 0.00390, 0.00443,
    0.00501, 0.00564, 0.00632, 0.00706, 0.00785, 0.00871, 0.00962, 0.01061,
    0.01166, 0.01279, 0.01399, 0.01526, 0.01662, 0.01806, 0.01959, 0.02121,
    0.02292, 0.02472, 0.02662, 0.02863, 0.03073, 0.03294, 0.03527, 0.03770,
    0.04025, 0.04292, 0.04571, 0.04862, 0.05165, 0.05481, 0.05810, 0.06153,
    0.06508, 0.06878, 0.07261, 0.07658, 0.08069, 0.08495, 0.08935, 0.09389,
    0.09859, 0.10343, 0.10842, 0.11356, 0.11885, 0.12429, 0.12988, 0.13563,
    0.14152, 0.14757, 0.15376, 0.16011, 0.16661, 0.17325, 0.18005, 0.18699,
    0.19407, 0.20130, 0.20867, 0.21618, 0.22382, 0.23161, 0.23952, 0.24757,
    0.25574, 0.26404, 0.27246, 0.28100, 0.28965, 0.29841, 0.30729, 0.31626,
    0.32533, 0.33450, 0.34376, 0.35311, 0.36253, 0.37204, 0.38161, 0.39126,
    0.40096, 0.41072, 0.42054, 0.43040, 0.44030, 0.45023, 0.46020, 0.47019,
    0.48020, 0.49022, 0.50025, 0.51028, 0.52031, 0.53033, 0.54033, 0.55031,
    0.56026, 0.57019, 0.58007, 0.58991, 0.59970, 0.60944, 0.61912, 0.62873,
    0.63827, 0.64774, 0.65713, 0.66643, 0.67564, 0.68476, 0.69377, 0.70269,
    0.71150, 0.72019, 0.72877, 0.73723, 0.74557, 0.75378, 0.76186, 0.76981,
    0.77762, 0.78530, 0.79283, 0.80022, 0.80747, 0.81457, 0.82151, 0.82831,
    0.83496, 0.84145, 0.84779, 0.85398, 0.86001, 0.86588, 0.87160, 0.87716,
    0.88257, 0.88782, 0.89291, 0.89785, 0.90264, 0.90728, 0.91176, 0.91610,
    0.92028, 0.92432, 0.92822, 0.93197, 0.93558, 0.93906, 0.94240, 0.94560,
    0.94867, 0.95162, 0.95444, 0.95713, 0.95971, 0.96217, 0.96451, 0.96674,
    0.96887, 0.97089, 0.97281, 0.97463, 0.97635, 0.97799, 0.97953, 0.98099,
    0.98236, 0.98366, 0.98488, 0.98602, 0.98710, 0.98811, 0.98905, 0.98994,
    0.99076, 0.99153, 0.99225, 0.99291, 0.99353, 0.99411, 0.99464, 0.99513,
    0.99558, 0.99600, 0.99639, 0.99674, 0.99706, 0.99736, 0.99763, 0.99788,
    0.99811, 0.99831, 0.99850, 0.99867, 0.99882, 0.99895, 0.99908, 0.99919,
    0.99929, 0.99938, 0.99946, 0.99953, 0.99959, 0.99965, 0.99969, 0.99974,
    0.99978, 0.99981, 0.99984, 0.99986, 0.99988, 0.99990, 0.99992, 0.99993,
    0.99994, 0.99995, 0.99996, 0.99997, 0.99998, 0.99998, 0.99998, 0.99999,
    0.99999, 0.99999, 0.99999, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000,
    1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000
};

static inline void ifft2 (complex_t * buf)
{
    double r, i;

    r = buf[0].real;
    i = buf[0].imag;
    buf[0].real += buf[1].real;
    buf[0].imag += buf[1].imag;
    buf[1].real = r - buf[1].real;
    buf[1].imag = i - buf[1].imag;
}

static inline void ifft4 (complex_t * buf)
{
    double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;

    tmp1 = buf[0].real + buf[1].real;
    tmp2 = buf[3].real + buf[2].real;
    tmp3 = buf[0].imag + buf[1].imag;
    tmp4 = buf[2].imag + buf[3].imag;
    tmp5 = buf[0].real - buf[1].real;
    tmp6 = buf[0].imag - buf[1].imag;
    tmp7 = buf[2].imag - buf[3].imag;
    tmp8 = buf[3].real - buf[2].real;

    buf[0].real = tmp1 + tmp2;
    buf[0].imag = tmp3 + tmp4;
    buf[2].real = tmp1 - tmp2;
    buf[2].imag = tmp3 - tmp4;
    buf[1].real = tmp5 + tmp7;
    buf[1].imag = tmp6 + tmp8;
    buf[3].real = tmp5 - tmp7;
    buf[3].imag = tmp6 - tmp8;
}

/* the basic split-radix ifft butterfly */

#define BUTTERFLY(a0,a1,a2,a3,wr,wi) do {	\
    tmp5 = a2.real * wr + a2.imag * wi;		\
    tmp6 = a2.imag * wr - a2.real * wi;		\
    tmp7 = a3.real * wr - a3.imag * wi;		\
    tmp8 = a3.imag * wr + a3.real * wi;		\
    tmp1 = tmp5 + tmp7;				\
    tmp2 = tmp6 + tmp8;				\
    tmp3 = tmp6 - tmp8;				\
    tmp4 = tmp7 - tmp5;				\
    a2.real = a0.real - tmp1;			\
    a2.imag = a0.imag - tmp2;			\
    a3.real = a1.real - tmp3;			\
    a3.imag = a1.imag - tmp4;			\
    a0.real += tmp1;				\
    a0.imag += tmp2;				\
    a1.real += tmp3;				\
    a1.imag += tmp4;				\
} while (0)

/* split-radix ifft butterfly, specialized for wr=1 wi=0 */

#define BUTTERFLY_ZERO(a0,a1,a2,a3) do {	\
    tmp1 = a2.real + a3.real;			\
    tmp2 = a2.imag + a3.imag;			\
    tmp3 = a2.imag - a3.imag;			\
    tmp4 = a3.real - a2.real;			\
    a2.real = a0.real - tmp1;			\
    a2.imag = a0.imag - tmp2;			\
    a3.real = a1.real - tmp3;			\
    a3.imag = a1.imag - tmp4;			\
    a0.real += tmp1;				\
    a0.imag += tmp2;				\
    a1.real += tmp3;				\
    a1.imag += tmp4;				\
} while (0)

/* split-radix ifft butterfly, specialized for wr=wi */

#define BUTTERFLY_HALF(a0,a1,a2,a3,w) do {	\
    tmp5 = (a2.real + a2.imag) * w;		\
    tmp6 = (a2.imag - a2.real) * w;		\
    tmp7 = (a3.real - a3.imag) * w;		\
    tmp8 = (a3.imag + a3.real) * w;		\
    tmp1 = tmp5 + tmp7;				\
    tmp2 = tmp6 + tmp8;				\
    tmp3 = tmp6 - tmp8;				\
    tmp4 = tmp7 - tmp5;				\
    a2.real = a0.real - tmp1;			\
    a2.imag = a0.imag - tmp2;			\
    a3.real = a1.real - tmp3;			\
    a3.imag = a1.imag - tmp4;			\
    a0.real += tmp1;				\
    a0.imag += tmp2;				\
    a1.real += tmp3;				\
    a1.imag += tmp4;				\
} while (0)

static inline void ifft8 (complex_t * buf)
{
    double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;

    ifft4 (buf);
    ifft2 (buf + 4);
    ifft2 (buf + 6);
    BUTTERFLY_ZERO (buf[0], buf[2], buf[4], buf[6]);
    BUTTERFLY_HALF (buf[1], buf[3], buf[5], buf[7], roots16[1]);
}

/* buf[0...4n-1]; weight[n...2n-2]; n >= 2 */
static void ifft_pass (complex_t * buf, sample_t * weight, int n)
{
    complex_t * buf1;
    complex_t * buf2;
    complex_t * buf3;
    double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    int i;

    buf++;
    buf1 = buf + n;
    buf2 = buf + 2 * n;
    buf3 = buf + 3 * n;

    BUTTERFLY_ZERO (buf[-1], buf1[-1], buf2[-1], buf3[-1]);

    i = n - 1;

    do {
	BUTTERFLY (buf[0], buf1[0], buf2[0], buf3[0], weight[n], weight[2*i]);
	buf++;
	buf1++;
	buf2++;
	buf3++;
	weight++;
    } while (--i);
}

static void ifft16 (complex_t * buf)
{
    ifft8 (buf);
    ifft4 (buf + 8);
    ifft4 (buf + 12);
    ifft_pass (buf, roots16 - 4, 4);
}

static void ifft32 (complex_t * buf)
{
    ifft16 (buf);
    ifft8 (buf + 16);
    ifft8 (buf + 24);
    ifft_pass (buf, roots32 - 8, 8);
}

static void ifft64 (complex_t * buf)
{
    ifft32 (buf);
    ifft16 (buf + 32);
    ifft16 (buf + 48);
    ifft_pass (buf, roots64 - 16, 16);
}

static void ifft128 (complex_t * buf)
{
    ifft32 (buf);
    ifft16 (buf + 32);
    ifft16 (buf + 48);
    ifft_pass (buf, roots64 - 16, 16);

    ifft32 (buf + 64);
    ifft32 (buf + 96);
    ifft_pass (buf, roots128 - 32, 32);
}

static void
imdct_do_512(sample_t data[],sample_t delay[], sample_t bias)
{
    int i,k;

    sample_t tmp_a_i;
    sample_t tmp_a_r;

    sample_t *data_ptr;
    sample_t *delay_ptr;
    sample_t *window_ptr;
	
    /* 512 IMDCT with source and dest data in 'data' */
	
    /* Pre IFFT complex multiply plus IFFT cmplx conjugate plus shuffling */
    for( i=0; i < 128; i++) {
	k = fftorder_128[i];
	/* z[i] = (X[256-2*k-1] + j * X[2*k]) * (xcos1[k] + j * xsin1[k]) ; */
	buf[i].real =         (data[256-2*k-1] * xcos1[k])  -  (data[2*k]       * xsin1[k]);
	buf[i].imag = -1.0 * ((data[2*k]       * xcos1[k])  +  (data[256-2*k-1] * xsin1[k]));
    }

    ifft128 (buf);

    /* Post IFFT complex multiply  plus IFFT complex conjugate*/
    for( i=0; i < 128; i++) {
	/* y[n] = z[n] * (xcos1[n] + j * xsin1[n]) ; */
	tmp_a_r =        buf[i].real;
	tmp_a_i = -1.0 * buf[i].imag;
	buf[i].real =(tmp_a_r * xcos1[i])  -  (tmp_a_i  * xsin1[i]);
	buf[i].imag =(tmp_a_r * xsin1[i])  +  (tmp_a_i  * xcos1[i]);
    }
	
    data_ptr = data;
    delay_ptr = delay;
    window_ptr = a52_imdct_window;

    /* Window and convert to real valued signal */
    for(i=0; i< 64; i++) { 
	*data_ptr++   = -buf[64+i].imag   * *window_ptr++ + *delay_ptr++ + bias; 
	*data_ptr++   =  buf[64-i-1].real * *window_ptr++ + *delay_ptr++ + bias; 
    }

    for(i=0; i< 64; i++) { 
	*data_ptr++  = -buf[i].real       * *window_ptr++ + *delay_ptr++ + bias; 
	*data_ptr++  =  buf[128-i-1].imag * *window_ptr++ + *delay_ptr++ + bias; 
    }

    /* The trailing edge of the window goes into the delay line */
    delay_ptr = delay;

    for(i=0; i< 64; i++) { 
	*delay_ptr++  = -buf[64+i].real   * *--window_ptr; 
	*delay_ptr++  =  buf[64-i-1].imag * *--window_ptr; 
    }

    for(i=0; i<64; i++) {
	*delay_ptr++  =  buf[i].imag       * *--window_ptr; 
	*delay_ptr++  = -buf[128-i-1].real * *--window_ptr; 
    }
}

static void
imdct_do_256(sample_t data[],sample_t delay[],sample_t bias)
{
    int i,k, p, q;

    sample_t tmp_a_i;
    sample_t tmp_a_r;

    sample_t *data_ptr;
    sample_t *delay_ptr;
    sample_t *window_ptr;

    complex_t *buf_1, *buf_2;

    buf_1 = &buf[0];
    buf_2 = &buf[64];

    /* Pre IFFT complex multiply plus IFFT cmplx conjugate */
    for( i=0; i < 64; i++) {
	k = fftorder_64[i];
	p = 2 * (128-2*k-1);
	q = 2 * (2 * k);
	/* z1[i] = (X1[128-2*k-1] + j * X1[2*k]) * (xcos2[k] + j * xsin2[k]); */
	buf_1[i].real =         (data[p] * xcos2[k])  -  (data[q] * xsin2[k]);
	buf_1[i].imag = -1.0 * ((data[q] * xcos2[k])  +  (data[p] * xsin2[k]));
	buf_2[i].real =         (data[p+1] * xcos2[k])  -  (data[q+1] * xsin2[k]);
	buf_2[i].imag = -1.0 * ((data[q+1] * xcos2[k])  +  (data[p+1] * xsin2[k]));
    }

    ifft64 (buf_1);
    ifft64 (buf_2);

    /* Post IFFT complex multiply */
    for( i=0; i < 64; i++) {
	/* y1[n] = z1[n] * (xcos2[n] + j * xs in2[n]) ; */ 
	tmp_a_r =  buf_1[i].real;
	tmp_a_i = -buf_1[i].imag;
	buf_1[i].real =(tmp_a_r * xcos2[i])  -  (tmp_a_i  * xsin2[i]);
	buf_1[i].imag =(tmp_a_r * xsin2[i])  +  (tmp_a_i  * xcos2[i]);
	/* y2[n] = z2[n] * (xcos2[n] + j * xsin2[n]) ; */ 
	tmp_a_r =  buf_2[i].real;
	tmp_a_i = -buf_2[i].imag;
	buf_2[i].real =(tmp_a_r * xcos2[i])  -  (tmp_a_i  * xsin2[i]);
	buf_2[i].imag =(tmp_a_r * xsin2[i])  +  (tmp_a_i  * xcos2[i]);
    }
	
    data_ptr = data;
    delay_ptr = delay;
    window_ptr = a52_imdct_window;

    /* Window and convert to real valued signal */
    for(i=0; i< 64; i++) { 
	*data_ptr++  = -buf_1[i].imag      * *window_ptr++ + *delay_ptr++ + bias;
	*data_ptr++  =  buf_1[64-i-1].real * *window_ptr++ + *delay_ptr++ + bias;
    }

    for(i=0; i< 64; i++) {
	*data_ptr++  = -buf_1[i].real      * *window_ptr++ + *delay_ptr++ + bias;
	*data_ptr++  =  buf_1[64-i-1].imag * *window_ptr++ + *delay_ptr++ + bias;
    }
	
    delay_ptr = delay;

    for(i=0; i< 64; i++) {
	*delay_ptr++ = -buf_2[i].real      * *--window_ptr;
	*delay_ptr++ =  buf_2[64-i-1].imag * *--window_ptr;
    }

    for(i=0; i< 64; i++) {
	*delay_ptr++ =  buf_2[i].imag      * *--window_ptr;
	*delay_ptr++ = -buf_2[64-i-1].real * *--window_ptr;
    }
}

void a52_imdct_init (uint32_t mm_accel)
{
#ifdef LIBA52_MLIB
    if (mm_accel & MM_ACCEL_MLIB) {
        fprintf (stderr, "Using mlib for IMDCT transform\n");
	a52_imdct_512 = a52_imdct_do_512_mlib;
	a52_imdct_256 = a52_imdct_do_256_mlib;
    } else
#endif
    {
	int i;

	fprintf (stderr, "No accelerated IMDCT transform found\n");

	/* Twiddle factors to turn IFFT into IMDCT */
	for (i = 0; i < 128; i++) {
	    xcos1[i] = -cos ((M_PI / 2048) * (8 * i + 1));
	    xsin1[i] = -sin ((M_PI / 2048) * (8 * i + 1));
	}

	/* More twiddle factors to turn IFFT into IMDCT */
	for (i = 0; i < 64; i++) {
	    xcos2[i] = -cos ((M_PI / 1024) * (8 * i + 1));
	    xsin2[i] = -sin ((M_PI / 1024) * (8 * i + 1));
	}

	a52_imdct_512 = imdct_do_512;
	a52_imdct_256 = imdct_do_256;
    }
}
