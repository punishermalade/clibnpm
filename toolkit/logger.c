/*  Implementation of the logger prototype.
    
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
    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include "logger.h"

/* define to remove the logger entirely from the source code */
#ifdef LOGGER_DISABLE
#define logmsg(...)
#endif

/* default values and constant */
uint32_t    g_bufferSize            = DEFAULT_BUFFER_SIZE;
LogLevel    g_logPriority           = LOGGER_INFO;
const char* TIMESTAMP_FORMAT        = "%Y-%m-%d %H:%M:%S";
const char* LOG_MSG_FORMAT_PREFIX   = "%s [%d] [%d]: ";

void logmsg(LogLevel priority, char* format, ...) 
{
    if (priority <= g_logPriority)
    {
        time_t timer;
        struct tm* tm_info;
        char tsBuffer[26];
        time(&timer);
        tm_info = localtime(&timer);
        strftime(tsBuffer, 26, TIMESTAMP_FORMAT, tm_info);
    
        // print the beginning of the log message
        printf(LOG_MSG_FORMAT_PREFIX, tsBuffer, priority, getpid());
        
        // print the actual message from the caller
        va_list args;
        va_start(args, format);
        printf(format, args);
        va_end(args);
        
        // end of line
        printf("\n");
    }
}

void set_buffer_size(uint32_t size)
{
    if (size <= 0)
    {
        g_bufferSize = DEFAULT_BUFFER_SIZE;
    }
    else 
    {
        g_bufferSize = size;
    }
}

void set_priority(LogLevel priority)
{
    if (priority >= LOGGER_NONE && priority <= LOGGER_VERBOSE) 
    {
        g_logPriority = priority;
    }
}

void print_byte_array(unsigned char *data, size_t len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        printf("%02X", *data);
        data++;
    }
    
    printf("\n");
}


