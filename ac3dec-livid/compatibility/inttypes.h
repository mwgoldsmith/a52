#ifndef _INTTYPES_H
#define _INTTYPES_H

/* default inttypes.h for people who do not have it on their system */

#if (!defined __int8_t_defined) && (!defined __BIT_TYPES_DEFINED__)
#define __int8_t_defined
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
#ifdef __i386__
typedef signed long long int64_t;
#endif
#endif
#if (!defined _LINUX_TYPES_H)
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#ifdef __i386__
typedef unsigned long long uint64_t;
#endif
#endif

/* 
 * check that sizeofs are what we expect.
 * If they are, we will not generate any data.
 * If they are not, we will get a compile-time error here.
 * It is then up to the user to fix the typedefs above.
 */

#define __CHECK__(type,size)				\
static int __check1__##type [sizeof(type)-size];	\
static int __check2__##type [size-sizeof(type)];

__CHECK__ (int8_t, 1);
__CHECK__ (uint8_t, 1);
__CHECK__ (int16_t, 2);
__CHECK__ (uint16_t, 2);
__CHECK__ (int32_t, 4);
__CHECK__ (uint32_t, 4);
#ifdef __i386__
__CHECK__ (int64_t, 8);
__CHECK__ (uint64_t, 8);
#endif

#undef __CHECK__

#endif
