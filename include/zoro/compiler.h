/**
 * @file compiler.h
 * @author Andrea Pepe
 * @copyright Copyright (c) 2024
 *
 * @brief Internal compiler directives and pre-processing helpers.
 */
#pragma once
#ifndef __ZORO_COMPILER_H__
#define __ZORO_COMPILER_H__

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

/* Extracted from linux/include/linux/compiler.h in kernel 5.16.11 */

#define likely(x)     __builtin_expect(!!(x), 1)
#define unlikely(x)   __builtin_expect(!!(x), 0)

/* Extracted from linux/include/linux/stringify.h in kernel 5.16.11 */

/* Indirect stringification.  Doing two levels allows the parameter to be a
 * macro itself.  For example, compile with -DFOO=bar, __stringify(FOO)
 * converts to "bar".
 */

#define __stringify_1(x...)	#x
#define __stringify(x...)	__stringify_1(x)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* Optimization barrier */
#ifndef barrier
/* The "volatile" is due to gcc bugs */
# define barrier() __asm__ __volatile__("": : :"memory")
#endif
#define smp_mb()			barrier()
#define smp_wmb()			barrier()
#define smp_read_barrier_depends()	barrier()

#if defined(__STDC__)
# if defined(__STDC_VERSION__)
#  if (__STDC_VERSION__ >= 199901L)
#   include <stdbool.h>
#  else
#   define bool uint8_t
#   define true  1
#   define false 0
#  endif
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Function taken from GNU libc source code.
 * Repo  : https://sourceware.org/git/glibc.git
 * Commit: 7e08db3359c8
 * Path  : malloc/reallocarray.c
 */
#if !defined(__GLIBC__) || !__GLIBC_PREREQ(2, 26)
static inline void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
	size_t bytes;
	if (__builtin_mul_overflow(nmemb, size, &bytes)) {
		errno = ENOMEM;
		return NULL;
	}
	return realloc(ptr, bytes);
}
#endif

/* Extracted from linux/include/linux/compiler_types.h in kernel 5.16.11 */
#define ___PASTE(a,b) a##b
#define __PASTE(a,b) ___PASTE(a,b)

/* Extracted from include/linux/compiler-gcc.h in kernel 5.16.11 */
/* GCC and CLANG has the same builtin */
#define __UNIQUE_ID(prefix) __PASTE(__PASTE(__UNIQUE_ID_, prefix), __COUNTER__)

/* Extracted from linux/include/asm-generic/unaligned.h in kernel 5.16.11 */
#define __get_unaligned_t(type, ptr) ({					\
	const struct { type x; } __attribute__((__packed__))		\
		*__pptr = (typeof(__pptr))(ptr);			\
	__pptr->x;							\
})

/* Extracted from linux/include/asm-generic/unaligned.h in kernel 5.16.11 */
#define get_unaligned(ptr)	__get_unaligned_t(typeof(*(ptr)), (ptr))


#ifdef __cplusplus
}
#endif

#endif
