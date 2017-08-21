/*  Declarations of the server prototype

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
    
#ifndef SERVER_H_
#define SERVER_H_

/* Defines the parameter needed by the server to start correctly */
struct serverparams
  {
    int port;
    int domain;
    int type;
    int protocol;
    int queue;
    void (*request_handler)(int);
  };

/* Create a new server and start listening. Return negative int if the server
   cannot be started */
extern int create_new_server(struct serverparams *__params);

/* Create a new server socket descriptor and returns it. Return negative int
   if cannot create a socket */
extern int open_server_socket(int __port, int __domain, int __type, int __protocol);

/* Listen and accept new connection, must have an opened SOCKET */
extern void listen_and_accept(int __socket, int __queue, void (*__handler)(int));

/* Set the SIGTERM handler. This is optional and the client can choose to
   handle the signal. */
extern void set_sigterm_handler(int  __socket);

/* enable the log from the server. 0 means disabled, other value means enabled */
extern void enable_log(int __enabled);

#endif
