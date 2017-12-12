/* Provides functionality to pad and unpad various content.

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

#ifndef LIBNPMTOOLKIT_PADDING_H_
#define LIBNPMTOOLKIT_PADDING_H_

#include <stdlib.h>

/* Pad the content with the ISO/IEC 7816 algo. The block_size is used to determine
   how many bytes are needed for the padding. The output parameter will be allocated
   the necessary memory.
   @return: 0 if successful, otherwise -1 if memory could not be allocated
    */
int padding_content(unsigned char *content, size_t content_len, size_t block_size,
					unsigned char **output, size_t *out_len);

/* Unpad the content with the ISO/IEC 7816 algo. If no padding is detected, the
   output parameter will have the same value as the input content. The output
   parameter will be allocated with the necessary memory.
   @return: 0 if successful, otherwise -1 if memory could not be allocated */
int unpadding_content(unsigned char *content, size_t content_len,
					  unsigned char **output, size_t *out_len);

#endif /* LIBNPMTOOLKIT_PADDING_H_ */
