/* 
 *    exponent.h
 *
 *	Aaron Holtzman - May 1999
 *
 */

#define UNPACK_FBW  1
#define UNPACK_CPL  2
#define UNPACK_LFE  4

void exponent_unpack( bsi_t *bsi, audblk_t *audblk, stream_coeffs_t *coeffs);