//this file exposes the functions that are used in the socket_client.c file
//
#ifndef SOCKET_CLIENT_SUPPORT_H
#define SOCKET_CLIENT_SUPPORT_H
#include "socket_support.h"

void init_client();

void close_client();

int send_message_c(char *message);

int receive_message_c(char *buffer);

#endif