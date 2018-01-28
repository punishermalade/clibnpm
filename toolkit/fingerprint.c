/*
    fingerprint.c: implementation to create a fingerprint buffer

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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "fingerprint.h"

int create_fingerprint_item(fingerprint_item *fi, unsigned char data[], size_t len)
{
	if (fi == NULL)
	{
		return FP_STRUCT_NULL;
	}

	fi->fitem = (unsigned char *)malloc(sizeof(unsigned char) * len);
	if (fi->fitem == NULL)
	{
		return FP_MEMORY_ERROR;
	}

	memcpy(fi->fitem, data, len);
	fi->len = len;

	return FP_SUCCESS;
}

int create_fingerprint_buffer(unsigned char **output, size_t *outlen, size_t len, ...)
{
	int i;
	va_list list, list2;
	size_t total = 0;

	// calculating the total size of the buffer
	va_start(list, len);
	for (i = 0; i < len; i++)
	{
		fingerprint_item fpi = va_arg(list, fingerprint_item);
		total += fpi.len;
	}
	va_end(list);

	// creating the output buffer
	*output = (unsigned char *)malloc(sizeof(unsigned char) * total);
	if (*output == NULL)
	{
		return FP_MEMORY_ERROR;
	}

	// copying the data into the output buffer
	va_start(list2, len);
	for (i = 0, total = 0; i < len; i++)
	{
		fingerprint_item fpi = va_arg(list2, fingerprint_item);
		memcpy(*output, fpi.fitem, fpi.len);

		// move ptr to n byte for next copy
		*output += fpi.len;
		total += fpi.len;
	}
	va_end(list2);

	// return the buffer size
	*outlen = total;

	// rewind to beginning
	*output -= total;

	return FP_SUCCESS;
}

