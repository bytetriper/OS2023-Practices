#include "chat_server.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: chat_server <port> <ip>\n";
        return 1;
    }

    try {
        boost::asio::io_context io_context;

        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::make_address(argv[2]), std::atoi(argv[1]));

        chat_server server(io_context, endpoint);

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
