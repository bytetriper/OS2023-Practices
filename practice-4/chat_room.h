#ifndef CHAT_ROOM_H_
#define CHAT_ROOM_H_

#include "chat_client.h"
#include "chat_message.h"
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <memory>
#include <set>
#include <unordered_map>
#include "chat_session.h"
class chat_room {
public:
    void join(chat_session::pointer session);

    void leave(chat_client::pointer client);

    void deliver(const chat_message& msg, chat_client::pointer sender);

private:
    std::set<chat_client::pointer> clients_;
    std::unordered_map<chat_session*, chat_client*> session_to_client_;
    std::unordered_map<chat_client*, chat_session*> client_to_session_;
    enum { max_recent_msgs = 100 };
    std::deque<chat_message> recent_msgs_;
};

#endif  // CHAT_ROOM_H_
