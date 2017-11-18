/*  Prototype for a socket client
    
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

#ifndef CLIENT_H_
#define CLIENT_H_

#include <netinet/in.h>
#include "internlog.h"

/* defining a byte on unsigned int on 8 bits */
typedef u_int8_t byte;

/* client params to connect to host */
struct clientparams
  {
    char* hostname;
    char* port;
    int family;
    int type;
  };
  
/* Create a socket client and resolve the hostname supplied by the __params.
   The __addrinfo will contain the necessary data to make a connection.
   The return value is 0 for success, otherwise negative int, errno will
   be set  */
extern int prepare_connection(struct clientparams* __param, 
                              struct addrinfo** __addrinfo);

/* Connect to hostname specified in the __addrinfo data structures.
   Will return the socket file descriptor on success, otherwise negative
   int, errno will be set.  */
extern int connect_to_host(struct addrinfo* __addrinfo);

/* Send data to the host. Return 0 if all the byte are sent, otherwise will
   return a negative value. errno may not be set */
extern int send_data_to_host(int __socket, byte* __data, size_t __length);

/* Wait and read any data sent from the SOCKET. Memory will be allocated 
   and the data copied in CONTENT. */
extern size_t waiting_data_from_host(int socket, byte** content, int block);

#endif
