/*  Simple implmentation that just display a message when a new connection
    is received.

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
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "../../libnpmtoolkit/dist/include/logger.h"
#include "../../libnpmnetwork/dist/include/server.h"

/* building the params from the command line */
void set_options(int argc, char* argv[], struct serverparams *params, int *logenabled);
void validate_options(struct serverparams *params);

/* Request handler function */
void my_function(int i)
{
    logmsg(LOGGER_INFO, "New connection handled, client socket: %d", i);
}

/* main program */
int main(int argc, char* argv[])
{ 
    int logenabled = 0;
    struct serverparams params = { 8080,        // default port
                                   AF_INET,     // using IPv4 or  IPv6
                                   SOCK_STREAM, // TCP
                                   0,           // non-blocking 
                                   100,         // 100 request max 
                                   &my_function // handler
                                 };
    
    // can override the default options with command line
    set_options(argc, argv, &params, &logenabled);
    validate_options(&params);
    enable_log(logenabled);
    
    // creating a new server, this will start listening
    if (create_new_server(&params) < 0)
    {
        logmsg(LOGGER_FATAL, "Could not start server: %d", errno);
        exit(-1);
    }
    
    return 0;
}

void set_options(int argc, char* argv[], struct serverparams *params, int *logenabled)
{
    int c;
    while ((c = getopt(argc, argv, "p:q:d")) != -1)
    {
        switch (c)
        {
          case 'p':
            params->port = atoi(optarg);
            break;
          case 'q':
            params->queue = atoi(optarg);
            break;
          case 'd':
            *logenabled = 1;
            break;
          default:
            abort();
        }
    }
}

void validate_options(struct serverparams *params)
{
    if (params->port < 1024 || params->port > 49151) 
    {
        logmsg(LOGGER_FATAL, "Server port out of range (1024-49151): %d", params->port);
        exit(-1);
    }
    
    if (params->queue < 1) 
    {
        logmsg(LOGGER_FATAL, "Request queue size too small: %d", params->queue);
        exit(-2);
    }
}
