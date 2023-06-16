#include "chat_client.h"
#include "chat_room.h"
#include "chat_session.h"
chat_client::chat_client(
    boost::asio::io_context &io_context,
    const boost::asio::ip::tcp::resolver::results_type &endpoints,
    chat_room &room)
    : io_context_(io_context), socket_(io_context_), room_(room) {
  do_connect(endpoints);
}

void chat_client::write(const chat_message &msg) {
  io_context_.post([this, msg]() {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
      do_write();
    }
  });
}

void chat_client::close() {
  boost::asio::post(io_context_, [this]() { socket_.close(); });
}

void chat_client::do_connect(
    const boost::asio::ip::tcp::resolver::results_type &endpoints) {
  boost::asio::async_connect(
      socket_, endpoints,
      [this](boost::system::error_code ec,
             const boost::asio::ip::tcp::endpoint & /*endpoint*/) {
        if (!ec) {
          std::cout << "Enter your client ID: ";
          std::getline(std::cin, client_id_);
          chat_message msg;
          msg.body_length(client_id_.length());
          std::memcpy(msg.body(), client_id_.c_str(), msg.body_length());
          msg.encode_header();
          write(msg);
          do_read_header();
        }
      });
}

void chat_client::do_read_header() {
  auto self(shared_from_this());
  boost::asio::async_read(
      socket_,
      boost::asio::buffer(read_msg_.data(), chat_message::header_length),
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec && read_msg_.decode_header()) {
          do_read_body();
        } else {
          room_.leave(shared_from_this());
        }
      });
}

boost::asio::ip::tcp::socket& chat_client::socket() { return socket_; }

void chat_client::do_read_body() {
  auto self(shared_from_this());
  boost::asio::async_read(
      socket_, boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
          room_.deliver(read_msg_, shared_from_this());
          do_read_header();
        } else {
          room_.leave(shared_from_this());
        }
      });
}
chat_client *chat_session::get_client() { return client_; }
void chat_client::do_write() {
  auto self(shared_from_this());
  boost::asio::async_write(
      socket_,
      boost::asio::buffer(write_msgs_.front().data(),
                          write_msgs_.front().length()),
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
          write_msgs_.pop_front();
          if (!write_msgs_.empty()) {
            do_write();
          }
        } else {
          room_.leave(shared_from_this());
        }
      });
}
