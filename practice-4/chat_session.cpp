#include "chat_session.h"
using boost::asio::ip::tcp;
chat_session::chat_session(tcp::socket socket, chat_room& room)
    : socket_(std::move(socket)), room_(room), client_proxy_(nullptr) {
  // 获取持有 socket 的 chat_client 智能指针
  auto client_ptr = room_.join(std::make_shared<chat_client>(std::move(socket_), room_));

  // 将 chat_client 智能指针转换为指向 chat_client 对象的原始指针，存储到 client_proxy_ 成员变量中
  client_proxy_ = client_ptr.get();
}


void chat_session::start() { do_read_header(); }

void chat_session::deliver(const chat_message& msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
        do_write();
    }
}

void chat_session::do_read_header() {
    auto self(shared_from_this());
    boost::asio::async_read(
        socket_, boost::asio::buffer(read_msg_.data(), chat_message::header_length),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec && read_msg_.decode_header()) {
                do_read_body();
            } else {
                room_.leave(client_proxy_.get_client());
            }
        });
}

void chat_session::do_read_body() {
  auto self(shared_from_this());
  boost::asio::async_read(
      socket_, boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
          room_.deliver(read_msg_, client_proxy_); // 使用 client_proxy_ 调用 chat_client 的 deliver() 函数
          do_read_header();
        } else {
          room_.leave(client_proxy_); // 使用 client_proxy_ 调用 chat_room 的 leave() 函数
        }
      });
}

void chat_session::do_write() {
  auto self(shared_from_this());
  boost::asio::async_write(
      socket_, boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
          write_msgs_.pop_front();
          if (!write_msgs_.empty()) {
            do_write();
          }
        } else {
          room_.leave(client_proxy_); // 使用 client_proxy_ 调用 chat_room 的 leave() 函数
        }
      });
}

