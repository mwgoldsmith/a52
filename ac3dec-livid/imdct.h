/* 
 *  imdct.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

void imdct(bsi_t *bsi, stream_coeffs_t *coeffs, stream_samples_t *samples);
void imdct_init(void);
//FIXME testing only
void imdct_do(float x[],float y[]);
