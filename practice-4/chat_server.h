#ifndef CHAT_SERVER_H_
#define CHAT_SERVER_H_

#include "chat_acceptor.h"
#include "chat_room.h"
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>

class chat_server {
public:
  chat_server(boost::asio::io_context& io_context, const boost::asio::ip::tcp::endpoint& endpoint);

private:
  void do_accept();

private:
  boost::asio::io_context& io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;

  // 将 chat_acceptor_ 替换为指向 chat_acceptor 对象的智能指针
  std::shared_ptr<chat_acceptor> chat_acceptor_; 

  chat_room room_;
};


#endif  // CHAT_SERVER_H_
