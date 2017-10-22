/*
    crypto.h
    Contains macros, type definition, enums that are used through 
    the library.
    
    ** Based on the LibTomCrypt library **
    
    MIT License

    Copyright (c) [2017] [Neilson P. Marcil]

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE. */

#ifndef NPM_LIB_CRYPTO_H_
#define NPM_LIB_CRYPTO_H_

/**
 * ================================================================
 * Types and enum section
 * ================================================================
 */

typedef unsigned ulong32;

/**
 * Contains all the error code that can be thrown by the different
 * function. Best way to verify that execution is correct is to
 * check if the function returns CRYPTO_OK.
 */
enum {
	CRYPTO_OK = 0,

	// AES
	CRYPTO_INVALID_DATA_SIZE,
	CRYPTO_INVALID_KEYSIZE,
	CRYPTO_INVALID_STRUCT
};

/**
 * ================================================================
 * Macros section
 * ================================================================
 */

#define BYTE(x, n) (((x) >> (8 * (n))) & 255)
#define CONDITION_CHECK(a, x) { if (!(a)) { return x; } }

#define LOAD32H(x, y)          \
asm __volatile__ (             \
   "movl (%1),%0\n\t"          \
   "bswapl %0\n\t"             \
   :"=r"(x): "r"(y));

#define STORE32H(x, y)           \
asm __volatile__ (               \
   "bswapl %0     \n\t"          \
   "movl   %0,(%1)\n\t"          \
   "bswapl %0     \n\t"          \
      ::"r"(x), "r"(y));

#define RORc(word,i) ({ \
   ulong32 __RORc_tmp = (word); \
   __asm__ ("rorl %2, %0" : \
            "=r" (__RORc_tmp) : \
            "0" (__RORc_tmp), \
            "I" (i)); \
            __RORc_tmp; \
   })

#endif
