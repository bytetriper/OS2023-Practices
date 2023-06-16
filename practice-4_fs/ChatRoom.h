#ifndef CHATROOM_H
#define CHATROOM_H
#include <iostream>
#include <set>
#include <memory>
#include <utility>
#include <deque>
#include <chrono>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;

const int MAX_MESSAGE_LENGTH = 1024;  // 最大消息长度（字节）
const std::chrono::seconds BAN_PERIOD(30);  // 禁止访问的时间段（秒）
//extern a ChatSession;
class ChatSession;
class ChatRoom
{
public:
    void join(std::shared_ptr<ChatSession> session);

    void leave(std::shared_ptr<ChatSession> session);

    void deliver(const std::string& message, std::shared_ptr<ChatSession> sender);

private:
    std::set<std::shared_ptr<ChatSession>> m_sessions;
};
#endif