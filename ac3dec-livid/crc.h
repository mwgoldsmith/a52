/* 
 *    crc.h
 *
 *	Aaron Holtzman - May 1999
 *
 */
 
int crc_validate(void);
void crc_process_bit(uint_32 bit);
void crc_process(uint_32 data, uint_32 num_bits);
void crc_init(void);
