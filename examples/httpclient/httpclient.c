/*  Simple client that send a HTTP GET request to a server and 
    display the response in the console.

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
#include <unistd.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include "../../libnpmnetwork/dist/include/client.h"

int main(int argc, char* argv[])
{   
    // reference to the client socket
    int socket = 0;

    // setup the client params for connection
    struct addrinfo *hostinfo;
    struct clientparams params;
    params.hostname =  argv[1];
    params.port = argv[2];
    params.family = AF_UNSPEC;
    params.type = SOCK_STREAM;
    
    // get the host information via the network layer of the OS
    if (prepare_connection(&params, &hostinfo) < 0)
    {
        printf("Could not get host information: %d\n", errno);
        exit(-1);
    }
    
    // actually connecting to the host, after this operation,
    // read/write possible.
    if ((socket = connect_to_host(hostinfo)) < 0)
    {
        printf("Could not connect to host: %d\n", errno);
        exit(-2);
    } 
    
    // sending a http request
    byte* request = "GET / HTTP/1.1\r\n\r\n";
    size_t requestsize = 22;
    send_data_to_host(socket, request, requestsize);
    
    // reading the response and close the client socket
    byte* content = NULL;
    size_t read = waiting_data_from_host(socket, &content, 0);
    close(socket);
    
    if (read > 0)
    {
        // got the raw byte, want to make a readable string
        unsigned char* stringcontent = (unsigned char*)malloc(read + 1);
        memcpy(stringcontent, content, read);
        memset(stringcontent + read, 0, 1);
        free(content);
        
        printf("%s", stringcontent);  
    }
    
    printf("Byte received: %d\n", read);
}
