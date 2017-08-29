/*  Simple data manipulation program that demonstrate the conversion 
    function from the toolkit.
    
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
#include <string.h>
#include "../../libnpmtoolkit/dist/include/convert.h"
#include "../../libnpmtoolkit/dist/include/logger.h"

/* validate the number of arguments and the size of the input */
void validate_arguments(int argc, char* argv[])
{
    if (argc < 2)
    {
        logmsg(LOGGER_ERROR, "Missing arguments");
        exit(-1);
    }
    
    size_t argLen = strlen(argv[1]);
    if (argLen % 2 != 0)
    {
        logmsg(LOGGER_ERROR, "Not a valid hexadecimal string, missing one character?");
        exit(-2);
    }
}

/* convert a hexadecimal string into a real byte array */
int main(int argc, char* argv[])
{
    validate_arguments(argc, argv);
    
    // everything is ok to succeed
    const char *input = argv[1];
    size_t inputLen = strlen(input);
    size_t outputLen = inputLen / 2;
    
    int result;
    unsigned char output[outputLen];
    if ((result = hexa_str_to_byte_array(input, output)) != 0)
    {
        logmsg(LOGGER_FATAL, "Something happens during conversion: %d", result);
        exit(result); 
    }

    print_byte_array(output, outputLen);
    return 0;
}
