/* 
 *  decode.h
 *
 *	Aaron Holtzman - May 1999
 *
 */

typedef struct stream_coeffs_s
{
	float fbw[5][256];
	float cpl[256];
	float lfe[256];
} stream_coeffs_t;

typedef struct stream_samples_s
{
	float channel[6][512];
} stream_samples_t;
