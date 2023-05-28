// this file implements necessary functions for creating a socket client
//  the client receive and send file in non-blocking mode

#include "socket_client.h"
#include <stdio.h>
// the PORT,BUFFER_SIZE is already defined in socket_const.h

void init_client() {
  // 创建客户端套接字
  client_sockfd = socket(AF_INET, SOCK_STREAM, 0);

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr(IP_ADDRESS);
  server_address.sin_port = htons(PORT);

  server_len = sizeof(server_address);

  // 连接服务器
  int connect_ret =
      connect(client_sockfd, (struct sockaddr *)&server_address, server_len);
  if (connect_ret == -1) {
    perror("connect error");
    exit(1);
  }
  printf("客户端已连接到服务器, IP: %s, PORT: %d\n", IP_ADDRESS, PORT);
  // set to unblocking mode

  int ret = SetSocketBlockingEnabled(client_sockfd,
                                     0); // 0 for unblocking, 1 for blocking
  if (ret == 0) {
    perror("set socket client unblocking error");
    exit(1);
  }
}

void close_client() { close(client_sockfd); }

// if receive a message, return 1, else return 0

int receive_message_c(char *buffer) {
  int n = read(client_sockfd, buffer, BUFFER_SIZE);
  if (n > 0) {
    // if the buffer ends with '\n', delete '\n'
    int l = strlen(buffer);
    printf("l = %d\n", l);
    printf("buffer: %s\n", buffer);
    if (buffer[l - 1] == (char)255) {
      buffer[l - 1] = '\0';
    }
    char *last_message = strrchr(buffer, 255);
    if (last_message != NULL) {
      printf("收到来自客户端的消息： %s\n", last_message);
      // strcpy
      strcpy(buffer, last_message+1);
    } else {
      printf("收到来自客户端的消息： %s\n", buffer);
    }
    return 0;
  } else {
    return -1;
  }
}

int send_message_c(char *buffer) {
  // add 255 to the end of the buffer
  int l = strlen(buffer);
  buffer[l] = (char)255;
  buffer[l + 1] = '\0';
  int n = write(client_sockfd, buffer, strlen(buffer));
  if (n < 0) {
    printf("发送消息失败\n");
    return -1;
  }
  printf("向服务器发送消息： %s\n", buffer);
  return 0;
}
