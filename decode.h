/* 
 *  decode.h
 *
 *	Aaron Holtzman - May 1999
 *
 */

typedef struct stream_coeffs_s
{
	float	left[256];
	float	right[256];
	float	centre[256];
	float	left_surround[256];
	float	right_surround[256];
	float	low_frequency[256];
} stream_coeffs_t;

typedef struct stream_samples_s
{
	float	left[512];
	float	right[512];
	float	centre[512];
	float	left_surround[512];
	float	right_surround[512];
	float	low_frequency[512];
} stream_samples_t;
