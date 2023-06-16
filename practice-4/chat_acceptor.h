#ifndef CHAT_ACCEPTOR_H_
#define CHAT_ACCEPTOR_H_

#include "chat_client.h"
#include "chat_room.h"
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
using boost::asio::ip::tcp;
class chat_acceptor : public std::enable_shared_from_this<chat_acceptor> {
public:
  using pointer = std::shared_ptr<chat_acceptor>;

  chat_acceptor(boost::asio::io_context& io_context, const tcp::endpoint& endpoint, chat_room& room);

  void start();
  void close(); // 声明 close() 函数

  boost::asio::ip::tcp::socket& socket(); // 声明访问 socket_ 的函数

private:
  void start_accept();

  void handle_accept(chat_client::pointer new_client, const boost::system::error_code& error);

private:
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::ip::tcp::socket socket_;
  chat_room& room_;
};



#endif  // CHAT_ACCEPTOR_H_
