/*  Prototype for the internal logger for the libnpmnetwork lib.
    This prototype defines a client that can connect to a server.

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
#include <stdarg.h>
#include "internlog.h"

int g_logEnabled = 0;

void print_log(FILE* f, char* format, va_list args)
{
    if (g_logEnabled != 0) 
    {
        vfprintf(f, format, args);
        fprintf(f, "\n");
    }  
}

void print_info(char* format, ...)
{
    va_list args;
    va_start(args, format);
    print_log(stdout, format, args);
    va_end(args);
} 

void print_error(char* format, ...)
{
    va_list args;
    va_start(args, format);
    print_log(stderr, format, args);
    va_end(args);
} 

void enable_log(int enabled) 
{
    g_logEnabled = enabled;
}
