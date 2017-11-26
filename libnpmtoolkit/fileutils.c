/*  Implementation for file utility functions

    This prototype provides wrapping function related to files
    operations.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fileutils.h"

/* this implementation uses loop to build a full path instead
   of using strcat or strcpy */
int get_file_path(char *path, char *file, char out[])
{
	int i;
	size_t path_len, file_len, total_len;

	if (path == NULL || file == NULL)
	{
		return -1;
	}

	// need some sizes for appending
	// need to count the separator at the end of the path
	path_len = strlen(path) + 1;
	file_len = strlen(file);
	total_len = path_len + file_len;

	// size 0 is not good
	if (path_len == 0 || file_len == 0)
	{
		return -2;
	}

	// need the NULL terminated character
	for (i = 0; i < path_len; i++)
	{
		out[i] = *path;
		path++;
	}

	out[i++] = FILE_SEPARATOR;

	for (; i < total_len; i++)
	{
		out[i] = *file;
		file++;
	}

	out[i++] = NULL_TERMINATING;
	return 0;
}

int append_to_file(char *file, char *content)
{
	if (file == NULL || content == NULL)
	{
		return -1;
	}

	FILE *file_ptr = fopen(file, "a");
	if (file_ptr == NULL)
	{
		return -2;
	}

	int written = fwrite(content, 1, sizeof(content), file_ptr);
	fclose(file_ptr);

	return written;
}
