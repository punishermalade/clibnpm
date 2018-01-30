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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "fileutils.h"

/* buffer size for read operation */
#define DEFAULT_BUFFER 1024

/* this implementation uses loop to build a full path instead
   of using strcat or strcpy */
int get_file_path(char *path, char *file, char **out)
{
	size_t path_len, file_len;

	if (path == NULL || file == NULL)
	{
		return INVALID_ARGUMENT;
	}

	// need some sizes for appending
	// need to count the separator at the end of the path
	path_len = strlen(path);
	file_len = strlen(file);

	// size 0 is not good
	if (path_len == 0 || file_len == 0)
	{
		return INVALID_ARGUMENT;
	}

	*out = (char *) malloc(sizeof(char) * (path_len + file_len + 1));
	if (*out == NULL)
	{
		return MALLOC_FAILED;
	}

	strcat(*out, path);
	strcat(*out, FILE_SEPARATOR);
	strcat(*out, file);

	return OK;
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
		return CANNOT_OPEN_FILE;
	}

	int written = fwrite(content, 1, sizeof(content), file_ptr);
	fclose(file_ptr);

	return written;
}

int write_data_to_file(void *data, size_t len, char *file)
{
	if (data == NULL || len == 0 || file == NULL)
	{
		return INVALID_ARGUMENT;
	}

	FILE *f_ptr = fopen(file, "w");
	if (f_ptr == NULL)
	{
		return CANNOT_OPEN_FILE;
	}

	fwrite(data, 1, len, f_ptr);
	fclose(f_ptr);
	return OK;
}

int load_data_from_file(char *file, void *data, size_t len)
{
	if (file == NULL || data == NULL)
	{
		return INVALID_ARGUMENT;
	}

	FILE *f_ptr = fopen(file, "r");
	if (f_ptr == NULL)
	{
		return CANNOT_OPEN_FILE;
	}

	// read the file
	fread(data, len, 1, f_ptr);
	fclose(f_ptr);

	return OK;
}

int read_file_content(FILE *file_ptr, unsigned char **output, size_t *out_len)
{
	// seeking the file size
	fseek(file_ptr, 0L, SEEK_END);
	*out_len = ftell(file_ptr);
	fseek(file_ptr, 0L, SEEK_SET);

	if (*out_len == 0)
	{
		return -2;
	}

	// dynamic allocation, read and set the cursor back to 0
	*output = (unsigned char*) malloc(sizeof(unsigned char) * (*out_len));
	if (*output == NULL)
	{
		return -1;
	}

	fread(*output, *out_len, 1, file_ptr);
	fseek(file_ptr, 0L, SEEK_SET);

	return 0;
}

int read_fd_content(int fd, unsigned char **output, size_t *out_len)
{
    unsigned char buffer[DEFAULT_BUFFER];
    unsigned char *temp = NULL;
    size_t byte_read, total_byte_read;

    memset(&buffer, 0, DEFAULT_BUFFER);
    byte_read = 0;
    total_byte_read = 0;

    while ((byte_read = read(fd, buffer, DEFAULT_BUFFER)) > 0)
    {
        temp = realloc(*output, total_byte_read + byte_read);
        if (temp == NULL)
        {
        		return -1;
        }

        memcpy(temp + total_byte_read, buffer, byte_read);
        *output = temp;
        total_byte_read += byte_read;
    }

    *out_len = total_byte_read;
    return 0;
}

int read_fd_size(int fd, size_t *out_len)
{
	size_t total_byte_read, byte_read;
	unsigned char buffer[DEFAULT_BUFFER];

	total_byte_read = 0;
	byte_read = 0;
	memset(buffer, 0, DEFAULT_BUFFER);

	while ((byte_read = read(fd, buffer, DEFAULT_BUFFER)) > 0)
	{
		total_byte_read += byte_read;
	}

	*out_len = total_byte_read;
	return 0;
}
