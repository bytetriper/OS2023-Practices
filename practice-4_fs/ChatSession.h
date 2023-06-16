#ifndef CHATSESSION_H
#define CHATSESSION_H
#include <iostream>
#include <openssl/ossl_typ.h>
#include <set>
#include <memory>
#include <utility>
#include <deque>
#include <chrono>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;
class ChatRoom;
class ChatSession : public std::enable_shared_from_this<ChatSession>{
    public:
    ChatSession(tcp::socket socket, ChatRoom& room);
    void start();
    void deliver(const std::string& message);
    tcp::socket& socket();
    void ban();
    void unban();
    void increaseConnectCount();
    int getConnectCount() const;
    private:
    void doRead();
    void doWrite();
    private:
    tcp::socket m_socket;
    ChatRoom& m_room;
    boost::asio::streambuf m_buffer;
    std::deque<std::string> m_writeQueue;
    boost::asio::steady_timer m_banTimer;
    int m_connectCount;
    std::string priv_key;
    RSA* rsa;
};
#endif