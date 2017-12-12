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

#include <string.h>
#include "padding.h"

/* Padding constant */
#define PADDING_FILL 	0x00
#define PADDING_LIMIT 	0x80

int padding_content(unsigned char *content, size_t content_len, size_t block_size,
					unsigned char **output, size_t *out_len)
{
	// need to check the full size, maybe need to pad
	if (content_len % block_size != 0)
	{
		// get the padded full size
		*out_len = block_size - (content_len % block_size);
		*out_len += content_len;

		// allocate memory and set memory
		*output = (unsigned char*) malloc(sizeof(unsigned char) * (*out_len));
		if (*output == NULL)
		{
			return -1;
		}
		// the end of the content will need 0x00 anyway, set the whole
		// area.
		memset(*output, PADDING_FILL, *out_len);

		// copy and pad
		memcpy(*output, content, content_len);
		memset(*output + content_len, PADDING_LIMIT, 1);
	}
	else
	{
		/*
		 * Need to allocated memory instead of just assigning pointer address.
		 * We don't know what the caller will do with the input after. To
		 * keep everything in order, better to make memory allocation here.
		 */
		*out_len = content_len;
		*output = (unsigned char*) malloc(sizeof(unsigned char) * (*out_len));
		if (*output == NULL)
		{
			return -1;
		}

		// just make a copy and we are done
		memcpy(*output, content, *out_len);
	}

	return 0;
}

int unpadding_content(unsigned char *content, size_t content_len, unsigned char **output, size_t *out_len)
{
	int index = content_len;
	while (index-- > 0)
	{
		unsigned char value = content[index];

		// the padding should have only these two values, if not, then
		// there is no padding!
		if (value != 0x00 && value != 0x80)
		{
			index = 0;
			break;
		}

		// end of padding, get out and finish the unpadding
		if (value == 0x80)
		{
			index--;
			break;
		}
	}

	// adjusting the length for the memcpy
	if (index > 0)
	{
		*out_len = index;
	}
	else
	{
		*out_len = content_len;
	}

	// allocate memory and copy
	*output = (unsigned char *)malloc(sizeof(unsigned char) * (*out_len));
	if (*output == NULL)
	{
		return -1;
	}
	memcpy(*output, content, *out_len);

	return 0;
}
