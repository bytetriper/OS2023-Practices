//this file defines the constants that are used socket_client.c and socket_server.c

#ifndef SOCKET_CONST_H
#define SOCKET_CONST_H
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#define PORT 6666
#define MAX 80
#define SA struct sockaddr
#define BUFFER_SIZE 1024
#define IP_ADDRESS "127.0.0.1"
int server_sockfd, client_sockfd, server_len, client_len;

struct sockaddr_in server_address, client_address;



int SetSocketBlockingEnabled(int fd, int blocking);
#endif
