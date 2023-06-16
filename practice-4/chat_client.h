#ifndef CHAT_CLIENT_H_
#define CHAT_CLIENT_H_

#include "chat_message.h"
#include <boost/asio.hpp>
#include <deque>
#include <iostream>
#include <memory>
#include <set>

class chat_room;

class chat_client : public std::enable_shared_from_this<chat_client> {
public:
    using pointer = std::shared_ptr<chat_client>;

    chat_client(boost::asio::io_context& io_context, const boost::asio::ip::tcp::resolver::results_type& endpoints,
                chat_room& room);

    void write(const chat_message& msg);

    void close();

    void connect(const std::string& client_id);

    boost::asio::ip::tcp::socket& socket();

private:
    void do_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints);

    void do_read_header();

    void do_read_body();

    void do_write();

private:
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::socket socket_;
    chat_room& room_;
    chat_message read_msg_;
    std::deque<chat_message> write_msgs_;
    std::string client_id_;
};

#endif  // CHAT_CLIENT_H_
