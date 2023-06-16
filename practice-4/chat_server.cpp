#include "chat_server.h"

chat_server::chat_server(boost::asio::io_context& io_context, const boost::asio::ip::tcp::endpoint& endpoint)
    : io_context_(io_context),
      acceptor_(io_context, endpoint),
      chat_acceptor_(std::make_shared<chat_acceptor>(acceptor_.get_executor().context(), endpoint, room_)) {
  do_accept();
}

void chat_server::do_accept() {
  chat_acceptor_->start();
}