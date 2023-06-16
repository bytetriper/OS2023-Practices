#ifndef CHAT_SESSION_H_
#define CHAT_SESSION_H_

#include "chat_client.h"
#include "chat_message.h"
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <memory>
#include <boost/asio/ip/tcp.hpp>
class chat_client;
using boost::asio::ip::tcp;
class chat_session : public std::enable_shared_from_this<chat_session> {
public:
  chat_session(tcp::socket socket, chat_room& room);

  void start();

private:
  void do_read_header();
  void do_read_body();
  void do_write();

private:
  tcp::socket socket_;
  chat_room& room_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;

  // 添加一个指向 chat_client 对象的原始指针
  chat_client* client_proxy_; 
};

#endif  // CHAT_SESSION_H_
