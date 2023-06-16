#include"chat_acceptor.h"
chat_acceptor::chat_acceptor(boost::asio::io_context& io_context, const tcp::endpoint& endpoint, chat_room& room)
    : acceptor_(io_context, endpoint), socket_(io_context), room_(room) {}

void chat_acceptor::start_accept() {
  chat_client::pointer new_client = std::make_shared<chat_client>(socket_.get_executor().context(), room_);
  acceptor_.async_accept(new_client->socket(),
                         std::bind(&chat_acceptor::handle_accept, shared_from_this(), new_client, std::placeholders::_1));
}


void chat_acceptor::handle_accept(chat_client::pointer new_client, const boost::system::error_code& error) {
    if (!error) {
        auto new_session = std::make_shared<chat_session>(std::move(socket_), room_);
        new_session->client_ = new_client.get(); // 新增：将客户端指针保存到会话对象中
        new_session->start();
        room_.join(new_session);
    }
    start_accept();
}
void chat_acceptor::start() {
  start_accept();
}
void chat_acceptor::close() {
  acceptor_.close();
  socket_.close();
}
boost::asio::ip::tcp::socket& chat_acceptor::socket() {
  return socket_;
}

