#include "chat_room.h"
#include "chat_client.h"
#include "chat_message.h"
void chat_room::join(chat_session::pointer session) {
    std::shared_ptr<chat_client> client = session->get_client()->shared_from_this();
    clients_.insert(client);
    session_to_client_[session.get()] = client.get();
    client_to_session_[client.get()] = session.get();
    for (auto msg : recent_msgs_) {
        session->deliver(msg);
    }
}

void chat_room::leave(chat_client::pointer client) {
    clients_.erase(client);
    auto it = std::find_if(session_to_client_.begin(), session_to_client_.end(),
                           [client](const auto& item) { return item.second == client.get(); });
    if (it != session_to_client_.end()) {
        session_to_client_.erase(it);
    }
}

void chat_room::deliver(const chat_message& msg, chat_client::pointer sender) {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs) {
        recent_msgs_.pop_front();
    }
    for (auto& client : clients_) { // 注意这里需要使用引用类型
        if (client != sender) {
            auto it = client_to_session_.find(client.get());
            if (it != client_to_session_.end()) {
                it->second->deliver(msg);
            }
        }
    }
}


