#include "ChatRoom.h"
#include "ChatSession.h"
void ChatRoom::join(std::shared_ptr<ChatSession> session)
{
    m_sessions.insert(session);

    // 检查连接次数是否过多
    session->increaseConnectCount();
    if (session->getConnectCount() > 10)
    {
        std::cerr << "Rejecting connection from " << session->socket().remote_endpoint().address().to_string() << " due to excessive connection attempts." << std::endl;
        session->ban();
        return;
    }
}

void ChatRoom::leave(std::shared_ptr<ChatSession> session)
{
    m_sessions.erase(session);
}

void ChatRoom::deliver(const std::string& message, std::shared_ptr<ChatSession> sender)
{
    for (const auto& session : m_sessions)
    {
        if (session != sender)
        {
            session->deliver(message);
        }
    }
}