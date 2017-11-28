/*  Prototype for file utility functions

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

#ifndef LIBNPMTOOLKIT_FILEUTILS_H_
#define LIBNPMTOOLKIT_FILEUTILS_H_

/* path separator and NULL terminating constant*/
#define FILE_SEPARATOR '/'
#define NULL_TERMINATING '\0'

/* define some error code to be used by the calling function */
enum {
	OK,
	INVALID_ARGUMENT,
	CANNOT_OPEN_FILE,
	CANNOT_WRITE_FILE,
	CANNOT_SEEK_FILE
};

/*
   Build a full path to a file. Output parameters must be large enough to hold the
   full path.
*/
int get_file_path(char *path, char *file, char out[]);

/*
    Append content to the end of the file.
*/
int append_to_file(char *file, char *content);

/*
 * Write the content of the memory to the file. This function is meant
 * to be used with data structure and provides a quick way to save
 * memory content into a file. If the file exists, it will be overwritten
 * with the new content.
 *
 * Avoid using data structure that contains pointers since only the
 * address will be written and not the content. A way to avoid this
 * is to dump the entire data in a memory space and use this one to
 * save the content in the file.
 */
int write_data_to_file(void *data, size_t len, char *file);

/*
 * Load the content of the file into memory. This function is meant
 * to fill a data structure from a file. The data structure should
 * already be allocated and be large enough to hold all the data from
 * the file.
 *
 * Special note that the data must not be a pointer, otherwise the
 * address found in the file will be loaded. It will result in an
 * invalid pointer.
 */
int load_data_from_file(char *file, void *data, size_t len);

#endif /* LIBNPMTOOLKIT_FILEUTILS_H_ */
