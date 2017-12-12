/*  Implementation for conversion utility function

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
#include "convert.h"

int hexa_str_to_byte_array(const char *input, unsigned char *output)
{
    size_t inputLen = 0;
    size_t outputLen = 0;
    int i;
    
    // checking the buffers
    if (input == NULL || output == NULL)
    {
        return -2;
    }
    
    // checking the size of the input buffer
    inputLen = strlen(input);
    
    if (inputLen <= 0 || inputLen % 2 != 0)
    {
        return -1;
    }
    
    // doing the conversion
    outputLen = inputLen / 2;
    for (i = 0; i < outputLen; i++)
    {
        sscanf(input + (i * 2), "%2hhx", &output[i]);
    }
    
    return 0;
}
