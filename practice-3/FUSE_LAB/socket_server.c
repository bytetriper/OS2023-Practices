// this file contains neccessary functios for maintaining a socket connection to
// a certain PORT as a server and send and receive messages from the client

// the server receive and send file in non-blocking mode, so the client should
// also be non-blocking
#include "socket_server.h"
void init_server() {
  // 创建服务器套接字
  server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr(IP_ADDRESS);
  server_address.sin_port = htons(PORT);

  server_len = sizeof(server_address);

  // 命名服务器套接字
  int bind_ret =
      bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
  if (bind_ret == -1) {
    perror("bind error");
    exit(1);
  }
  // 创建监听队列
  listen(server_sockfd, 5);

  printf("服务器正在监听端口 %d...\n", PORT);
  client_len = sizeof(client_address);

  // 等待客户端连接请求
  client_sockfd =
      accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
  // set to unblocking mode
  int ret = SetSocketBlockingEnabled(client_sockfd,
                                     0); // 0 for unblocking, 1 for blocking
  if (ret == 0) {
    perror("set socket server unblocking error");
    exit(1);
  }
}

void close_server() {
  close(client_sockfd);
  close(server_sockfd);
}

// if receive a message, return 1, else return 0

int receive_message(char *buffer) {
  int n = read(client_sockfd, buffer, BUFFER_SIZE);
  if (n > 0) {
    //if the buffer ends with '\n', delete '\n'
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

int send_message(char *buffer) {
  //add 255 to the end of the buffer
  int l=strlen(buffer);
  buffer[l] = (char)255;
  buffer[l + 1] = '\0';
  int n = write(client_sockfd, buffer, strlen(buffer));
  if (n < 0) {
    printf("发送消息失败\n");
    return -1;
  }
  printf("向客户端发送消息： %s\n", buffer);
  return 0;
}