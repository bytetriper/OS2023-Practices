// ChatClient.cpp

#include "crypto.h"
#include <boost/asio.hpp>
#include <iostream>
#include <openssl/ossl_typ.h>
#include <thread>
#include "utils.h"
using boost::asio::ip::tcp;

class ChatClient {
public:
  void write(const std::string &message, bool with_name = true,bool encrypt_opt = false) {
    std::string fullMessage;
    if (with_name)
      fullMessage = m_name + ": " + message;
    else
      fullMessage = message;
    if (encrypt_opt)
      fullMessage = encrypt(fullMessage, this->rsa);
    std::cout<<("sending a message:\n");
    print_with_escape(fullMessage);
    auto buffer=boost::asio::buffer(fullMessage+'\0');
    boost::asio::write(m_socket, buffer);
    auto decrypted=decrypt(fullMessage, this->rsa);
    std::cout<<("decrypted message:\n");
    print_with_escape(decrypted);
  }
  ChatClient(boost::asio::io_context &io_context, const std::string &host,
             const std::string &port, const std::string &name)
      : m_socket(io_context), m_name(name) {
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(host, port);

    boost::asio::connect(m_socket, endpoints);

    std::cout << "Connected to server at "
              << m_socket.remote_endpoint().address().to_string() << ":"
              << m_socket.remote_endpoint().port() << std::endl;

    std::thread readThread([this]() { doRead(); });
    readThread.detach();
    this->rsa = NULL;
    generate_key(this->rsa, pub_key, priv_key);
    std::cout << "Private key: "<< std::endl;
    print_with_escape(priv_key);
    auto priv_body=priv_key;
    //priv_body.erase(0,32);
    //priv_body.erase(priv_body.size()-30,30);
    write(priv_body ,false, false);
  }

private:
  void doRead() {
    try {
      while (true) {
        char data[2048];
        std::size_t length = m_socket.read_some(boost::asio::buffer(data));
        std::string message(data, length);
        //decrypt(message, this->rsa);
        std::cout.write(message.data(), length);
      }
    } catch (std::exception &e) {
      std::cerr << "Exception in thread: " << e.what() << std::endl;
    }
  }

private:
  tcp::socket m_socket;
  std::string m_name;
  std::string pub_key, priv_key;
  RSA *rsa;
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 4) {
      std::cerr << "Usage: ChatClient <host> <port> <name>" << std::endl;
      return 1;
    }

    boost::asio::io_context io_context;
    ChatClient client(io_context, argv[1], argv[2], argv[3]);

    while (true) {
      std::string message;
      std::getline(std::cin, message);
      client.write(message);
    }
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
