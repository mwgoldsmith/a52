#ifndef __MM_ACCEL_H__
#define __MM_ACCEL_H__

#include <inttypes.h>

// generic accelerations
#define MM_ACCEL_MLIB		0x00000001

// x86 accelerations
#define MM_ACCEL_X86_MMX	0x80000000
#define MM_ACCEL_X86_3DNOW	0x40000000
#define MM_ACCEL_X86_MMXEXT	0x20000000

uint32_t mm_accel (void);

#endif
