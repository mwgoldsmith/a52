/* 
 *    parse.h
 *
 *	Aaron Holtzman - May 1999
 *
 */

void parse_syncinfo(syncinfo_t *syncinfo,bitstream_t *bs);
void parse_audblk(bsi_t *bsi,audblk_t *audblk,bitstream_t *bs);
void parse_bsi(bsi_t *bsi,bitstream_t *bs);
void parse_auxdata(bitstream_t *bs);

