/*  Declarations of the logger prototype.
    
    This logger is to provide timestamp, process id information to 
    any log message.
    
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

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdint.h>
#include <stdarg.h>

/* Default size of the buffer to hold one message */
#define DEFAULT_BUFFER_SIZE 1024

/* Different log level to be used for each message */
#define LOGGER_NONE    -1   // this level is used to disable the logging during runtime
#define LOGGER_FATAL    0   // critical error, cannot recover
#define LOGGER_ERROR    1   // important error, recovery possible
#define LOGGER_WARNING  2   // error that is recoverable by the program
#define LOGGER_INFO     3   // information about the program
#define LOGGER_DEBUG    4   // debug info
#define LOGGER_VERBOSE  5   // more debug info

/* Represent the LogLevel defined on one byte */
typedef char LogLevel;

/* Set the minimal LogLevel to be display */
extern void set_priority(LogLevel __loglevel);

/* Set the buffer size to hold one message */
extern void set_buffer_size(uint32_t __size);

/* Display a log message in stdout */
extern void logmsg(LogLevel __loglevel, char* __format , ...);

/* Display a byte array in the standard output */
extern void print_byte_array(unsigned char *data, size_t len);

#endif
