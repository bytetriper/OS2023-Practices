// ChatServer.cpp

#include <iostream>
#include <set>
#include <memory>
#include <utility>
#include <deque>
#include <chrono>
#include <boost/asio.hpp>
#include "ChatRoom.h"
#include "ChatSession.h"

class ChatServer
{
public:
   ChatServer(boost::asio::io_context& io_context, const std::string& address, const std::string& port)
    : m_acceptor(io_context)
    , m_room()
{
    tcp::resolver resolver(io_context);
    tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
    //print a message to the console to let the user know the server is running
    std::cout << "Server is running at " << endpoint.address().to_string() << ":" << endpoint.port() << std::endl;
    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(tcp::acceptor::reuse_address(true));
    m_acceptor.bind(endpoint);
    m_acceptor.listen();
    doAccept();
}


private:
    void doAccept()
    {
        m_acceptor.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec)
                {
                    auto session = std::make_shared<ChatSession>(std::move(socket), m_room);
                    m_room.join(session);
                    session->start();
                }

                doAccept();
            });
    }

private:
    tcp::acceptor m_acceptor;
    ChatRoom m_room;
};
int main(int argc, char* argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: ChatServer <address> <port>" << std::endl;
            return 1;
        }

        boost::asio::io_context io_context;
        ChatServer server(io_context, argv[1], argv[2]);

        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
