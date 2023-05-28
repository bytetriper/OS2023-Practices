#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
  int server_sockfd, client_sockfd, server_len, client_len;
  struct sockaddr_in server_address, client_address;

  // 创建服务器套接字
  server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(PORT);

  server_len = sizeof(server_address);

  // 命名服务器套接字
  bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

  // 创建监听队列
  listen(server_sockfd, 5);

  printf("服务器正在监听端口 %d...\n", PORT);

  char buffer[BUFFER_SIZE] = {0};

  client_len = sizeof(client_address);

  // 等待客户端连接请求
  client_sockfd =
      accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);

  pid_t pid = fork(); // 创建一个子进程处理客户端请求

  if (pid == -1) {
    perror("fork error");
    exit(1);
  } else if (pid == 0) { // 子进程处理客户端请求

    printf("进程 [%d] 正在处理客户端请求...\n", getpid());

    // 从客户端读取消息
    read(client_sockfd, buffer, BUFFER_SIZE);

    // 处理客户端消息
    printf("收到来自客户端的消息： %s\n", buffer);
    sprintf(buffer, "Hello from server");

    // 向客户端发送消息
    write(client_sockfd, buffer, strlen(buffer));

    printf("已向客户端发送消息： %s\n", buffer);

    close(client_sockfd); // 关闭客户端套接字
    exit(0);              // 子进程退出
  }

  close(client_sockfd); // 父进程关闭客户端套接字

  return 0;
}
