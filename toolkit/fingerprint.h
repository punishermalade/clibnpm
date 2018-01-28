/*
    fingerprint.h: prototype to build a fingerprint buffer.

    MIT License

    Copyright (c) [2018] [Neilson P. Marcil]

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

#ifndef LIBNPM_FINGERPRINT_H_
#define LIBNPM_FINGERPRINT_H_

#include <stdlib.h>

/* error constant */
enum {
	FP_SUCCESS = 0,
	FP_CANNOT_ADD_ITEM,
	FP_STRUCT_NULL,
	FP_MEMORY_ERROR
};

/* structure to hold part of a fingerprint */
typedef struct fingerprint_item_struct {
	unsigned char *fitem;
	size_t len;
} fingerprint_item;

/* create a new fingerprint item.
 * @param fi: data structure, must not be NULL
 * @param data: the data to include in the fingerprint
 * @param len: the data length
 * @return: FP_SUCCESS, otherwise error number
 */
int create_fingerprint_item(fingerprint_item *fi, unsigned char *data, size_t len);

/* create a continuous buffer from a list of fingerprint_item.
 * @param: output buffer; initialized in the function
 * @param: length buffer; contains the size of the output buffer
 * @param: len; the number of fingerprint item that are used in the function
 * @param: ...; all the fingerprint item to be used.
 */
int create_fingerprint_buffer(unsigned char **output, size_t *outlen, size_t len, ...);

#endif /* LIBNPM_FINGERPRINT_H_ */
