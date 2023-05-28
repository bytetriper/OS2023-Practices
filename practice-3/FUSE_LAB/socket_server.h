//this file exposes the functions that are used in the socket_server_support.c file

#ifndef SOCKET_SERVER_SUPPORT_H

#define SOCKET_SERVER_SUPPORT_H
#include "socket_support.h"

void init_server();

void close_server();

int send_message(char *message);

int receive_message(char *buffer);
#endif
