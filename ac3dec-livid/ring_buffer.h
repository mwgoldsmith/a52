/*
 *
 * ring_buffer.h
 *
 * Aaron Holtzman - June 1999
 *
 *
 */

void rb_init(void);
sint_16* rb_begin_read(void);
void rb_end_read(void);
sint_16* rb_begin_write(void);
void rb_end_write(void);

