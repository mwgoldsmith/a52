/* 
 *    unpack.h
 *
 *	Aaron Holtzman - May 1999
 *
 */

#define UNPACK_FBW  1
#define UNPACK_CPL  1
#define UNPACK_LFE  1


void unpack_exponents( bsi_t *bsi, audblk_t *audblk, stream_coeffs_t *coeffs);
void unpack_reset(void);
uint_16 unpack_mantissa(bitstream_t *bs, uint_16 bap);
