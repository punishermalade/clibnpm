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

/* constant used for IO */
#define FILE_SEPARATOR '/'
#define NULL_TERMINATING '\0'

/* build a full path to a file. Output parameters must be large enough to hold the
   full path. */
int get_file_path(char *path, char *file, char out[]);

/* append content to the end of the file
   @param file: the filename to open and write content
   @param content: the content to write
   @return: the number of byte written
   	   	    -1 if any of the argument are null
   	   	   	-2 if the file cannot be opened in append mode
*/
int append_to_file(char *file, char *content);


#endif /* LIBNPMTOOLKIT_FILEUTILS_H_ */
