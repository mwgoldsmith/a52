/* 
 *  imdct.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ac3.h"
#include "decode.h"
#include "imdct.h"

void imdct_do(float x[],float y[]);

typedef struct complex_s
{
	float real;
	float imag;
} complex_t;


#define N 512

static complex_t pre_trans[N/4];
static complex_t post_trans[N/4];
static complex_t pre_window[N/4];
/* Twiddle factors for IMDCT */
static float xcos1[N/4];
static float xsin1[N/4];
/* Windowing function for Modified DCT - Thank you acroread */
static float w[] = {
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
	1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000 };

void imdct_init(void)
{
	int i;

	for( i=0; i < N/4; i++)
	{
		xcos1[i] = -cos(2 * M_PI * (8*i+1)/(8*N)) ; 
		xsin1[i] = -sin(2 * M_PI * (8*i+1)/(8*N)) ;
	}

}

void 
imdct(stream_coeffs_t *coeffs, stream_samples_t *samples)
{
	/* FIXME only do the transform for channels that are enabled */
	imdct_do(coeffs->left,samples->left);
	imdct_do(coeffs->right,samples->right);
	imdct_do(coeffs->centre,samples->centre);
	imdct_do(coeffs->left_surround,samples->left_surround);
	imdct_do(coeffs->right_surround,samples->right_surround);
	imdct_do(coeffs->low_frequency,samples->low_frequency);
}

void
imdct_do(float x[],float y[])
{
	int i,k;

	/* Pre IFFT complex multiply */
	for( i=0; i < N/4; i++)
	{
		/* z[i] = (X[N/2-2*i-1] + j * X[2*i]) * (xcos1[i] + j * xsin1[i]) ; */ 
		pre_trans[i].real =(x[N/2-2*i-1] * xcos1[i])  -  (x[2*i]       * xsin1[i]);
	  pre_trans[i].imag =(x[2*i]       * xcos1[i])  +  (x[N/2-2*i-1] * xsin1[i]);
	}

	/* BIGASS FIXME We do complex IDFT because we're lazy */
	for(i=0; i<N/4; i++) { 
		post_trans[i].real = 0.0; 
		post_trans[i].imag = 0.0; 

		for(k=0; k<N/4; k++) 
		{ 
			post_trans[i].real += (pre_trans[k].real * cos(8*M_PI*k*i/N)) - (pre_trans[k].imag * sin(8*M_PI*k*i/N));
			post_trans[i].imag += (pre_trans[k].real * sin(8*M_PI*k*i/N)) + (pre_trans[k].imag * cos(8*M_PI*k*i/N));
		} 
	}

	/* Post IFFT complex multiply */
	for( i=0; i < N/4; i++)
	{
		/* z[i] = (X[N/2-2*i-1] + j * X[2*i]) * (xcos1[i] + j * xsin1[i]) ; */ 
		pre_window[i].real =(post_trans[i].real * xcos1[i])  -  (post_trans[i].imag  * xsin1[i]);
	  pre_window[i].imag =(post_trans[i].real * xsin1[i])  +  (post_trans[i].imag  * xcos1[i]);
	}
	
	/* Window and convert to real valued signal */

	for(i=0; i<N/8; i++) { 
		y[2*i]         = -pre_window[N/8+i].imag   * w[2*i]; 
		y[2*i+1]       =  pre_window[N/8-i-1].real * w[2*i+1]; 
		y[N/4+2*i]     = -pre_window[i].real       * w[N/4+2*i]; 
		y[N/4+2*i+1]   =  pre_window[N/4-i-1].imag * w[N/4+2*i+1]; 
		y[N/2+2*i]     = -pre_window[N/8+i].real   * w[N/2-2*i-1];
		y[N/2+2*i+1]   =  pre_window[N/8-i-1].imag * w[N/2-2*i-2];
		y[3*N/4+2*i]   =  pre_window[i].imag       * w[N/4-2*i-1];
		y[3*N/4+2*i+1] = -pre_window[N/4-i-1].real * w[N/4-2*i-2];
	}
}
