#include "chat_client.h"
#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: chat_client <port> <ip> <name>\n";
        return 1;
    }

    try {
        boost::asio::io_context io_context;

        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::make_address(argv[2]), std::atoi(argv[1]));

        chat_client::pointer client = chat_client::create(io_context, argv[3]);
        client->connect(endpoint);

        std::thread t([&io_context]() { io_context.run(); });

        char line[chat_message::max_body_length + 1];
        while (std::cin.getline(line, chat_message::max_body_length + 1)) {
            chat_message message;
            std::string input_str(line);
            message.body_length(input_str.length());
            std::memcpy(message.body(), input_str.c_str(), message.body_length());
            message.encode_header();
            client->write(message);
        }
        client->close();
        t.join();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
