/*  Implementation of the client request handler

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
#include <unistd.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "client.h"

/* error code */
const int ERR_CANNOT_GET_ADDR_INFO          = -1;
const int ERR_CANNOT_CREATE_SOCKET_TO_HOST  = -2;
const int ERR_CANNOT_CONNECT_TO_HOST        = -3;
const int ERR_CANNOT_SEND_TO_HOST           = -4;

/* the reading buffer size in bytes */
const u_int16_t READ_BUFFER_SIZE            = 1024;

extern int prepare_connection(struct clientparams* params, struct addrinfo** hostinfo)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = params->family;
    hints.ai_socktype = params->type;
    
    if (getaddrinfo(params->hostname, params->port, &hints, hostinfo) < 0)
    {
        print_error("Error while getting address info");
        return ERR_CANNOT_GET_ADDR_INFO;
    }
    
    return 0;
}

extern int connect_to_host(struct addrinfo* hostinfo) 
{
    int remoteSocket = 0;
    
    if ((remoteSocket = socket(hostinfo->ai_family, 
                               hostinfo->ai_socktype, 
                               hostinfo->ai_protocol)) < 0)
    {
        print_error("Error creating a socket to host");
        return ERR_CANNOT_CREATE_SOCKET_TO_HOST;
    }
    
    if (connect(remoteSocket, hostinfo->ai_addr, hostinfo->ai_addrlen) < 0)
    {
       print_error("Could not connect to host");
       return ERR_CANNOT_CONNECT_TO_HOST;
    }
    
    return remoteSocket;
}

int send_data_to_host(int socket, byte* content, size_t len)
{
    int byteSent = 0;
    
    if ((byteSent = send(socket, content, len, 0)) < 0)
    {
        print_error("Cannot send data to host");
        return ERR_CANNOT_SEND_TO_HOST;
    }

    return 0;
}

size_t waiting_data_from_host(int socket, byte** content, int block)
{
    byte buffer[READ_BUFFER_SIZE];
    byte* temp = NULL;
    size_t byteRead, totalByteRead;
    
    memset(&buffer, 0, READ_BUFFER_SIZE);
    byteRead = 0;
    totalByteRead = 0;
    
    while ((byteRead = recv(socket, buffer, READ_BUFFER_SIZE, block)) > 0)
    {
        temp = realloc(*content, totalByteRead + byteRead);
        memcpy(temp + totalByteRead, buffer, byteRead);
        *content = temp;
        totalByteRead += byteRead;
        
    }
    
    return totalByteRead;
}
