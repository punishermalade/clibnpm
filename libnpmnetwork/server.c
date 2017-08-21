/*  Implementation of the server prototype

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
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.h"

/* internal error code */
const int8_t ERR_CANNOT_BIND_SOCKET    = -1;
const int8_t ERR_CANNOT_CREATE_SOCKET  = -2;

/* catching termination signal to cleanup */
void close_resources(int signum);
int g_serverSocket;

/* console printing utils function */
int8_t g_logEnabled = 0;
void print_log(FILE* f, char* format, va_list args);
void print_info(char* format, ...);
void print_error(char* format, ...);

int create_new_server(struct serverparams *params) 
{
    int socket = 0;
    socket = open_server_socket(params->port,
                                params->domain,
                                params->type,
                                params->protocol);
    
    // socket created, listening the server
    if (socket > 0)
    {
        set_sigterm_handler(socket);                               
        listen_and_accept(socket, params->queue, params->request_handler);
        return 0;
    }   
                            
    // there was an error, throwing it
    return socket;
}

int open_server_socket(int port, int domain, int type, int protocol)
{
    struct sockaddr_in saddr;
    int ssocket = 0;
    
    print_info("Creating a new server socket to listen on port %d", port);
    
    if ((ssocket = socket(domain, type, protocol)) < 0)
    {
        print_error("Could not create a socket: %d", errno);
        return ERR_CANNOT_CREATE_SOCKET;
    }
    
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = domain;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(ssocket, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)
    {
        print_error("Cannot bind to socket, maybe port already in use?");
        return ERR_CANNOT_BIND_SOCKET;
    }
    
    print_info("Server socket [%d] binded on port %d", ssocket, port);
    return ssocket;
}

void listen_and_accept(int socket, int queue, void (*handler)(int))
{
    struct sockaddr_in caddr;
    unsigned int caddrLen = sizeof(caddr);
    int client = 0;
    
    memset(&caddr, 0, caddrLen);
    listen(socket, queue);
        
    print_info("Now accepting incoming connection");

    while ((client = accept(socket, (struct sockaddr*)&caddr, &caddrLen)) != -1)
    {
        print_info("Connection accepted from %d", caddr.sin_addr.s_addr);
        if (fork() == 0) // in child process
        {   
            close(socket);
            handler(client);
        }
        else // in parent process
        {
            close(client);
        }
    }
}

void set_sigterm_handler(int serverSocketDesc)
{
    g_serverSocket = serverSocketDesc;
    
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = close_resources;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
}

void close_resources(int signum)
{
    print_info("Closing socket [%d]", g_serverSocket);
    close(g_serverSocket);
}

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
