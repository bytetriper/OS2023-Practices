#include "ChatSession.h"
#include "ChatRoom.h"
#include "crypto.h"
#include "utils.h"
#include <iostream>
#include <string>
ChatSession::ChatSession(tcp::socket socket, ChatRoom &room)
    : m_socket(std::move(socket)), m_room(room),
      m_banTimer(m_socket.get_executor()), m_connectCount(0) {}

void ChatSession::start() {
  m_room.join(shared_from_this());
  doRead();
}

void ChatSession::deliver(const std::string &message) {
  bool writeInProgress = !m_writeQueue.empty();
  m_writeQueue.push_back(message);
  if (!writeInProgress) {
    doWrite();
  }
}

tcp::socket &ChatSession::socket() { return m_socket; }

void ChatSession::ban() {
  m_banTimer.expires_after(BAN_PERIOD);
  m_banTimer.async_wait(
      [self = shared_from_this()](const boost::system::error_code &ec) {
        if (!ec) {
          self->m_connectCount = 0;
        }
      });
}
void ChatSession::unban() {
  std::cout << "Unbanning client "
            << m_socket.remote_endpoint().address().to_string() << "."
            << std::endl;
  m_banTimer.cancel();
}
void ChatSession::increaseConnectCount() { m_connectCount++; }

int ChatSession::getConnectCount() const { return m_connectCount; }

void ChatSession::doRead() {
  auto self(shared_from_this());
  boost::asio::async_read_until(
      m_socket, m_buffer, '\0',
      [this, self](const boost::system::error_code &ec, std::size_t length) {
        if (!ec) {
          std::string message;
          std::istream is(&m_buffer);
          std::getline(is, message, '\0');
          deliver("received:" + message);
          // judge if priv_key ends with 'END RSA PRIVATE KEY-----'
          if (priv_key.length() < 800 ||
              !priv_key.ends_with("END RSA PRIVATE KEY-----\n")) {
            priv_key += message;
            if (priv_key.length() >= 800 &&
                priv_key.ends_with("END RSA PRIVATE KEY-----\n")) {
              // finally we receive the whole priv_key
              deliver(priv_key);
              rsa = create_rsa_from_key(priv_key, false);
              deliver("Welcome to the chat room! [with RSA encryption]");
            }
          } else {
            // message.pop_back();
            m_room.deliver("encrypted:" + message, shared_from_this());
            message = decrypt(message, rsa);
            // std::cout.write(("received: "+message+"\n").data(), length);
            m_room.deliver("decrypted:" + message, shared_from_this());
          }
          doRead();
        } else if (ec == boost::asio::error::eof) {
          m_room.leave(shared_from_this());
        }
      });
}
void ChatSession::doWrite() {
  auto self(shared_from_this());
  boost::asio::async_write(
      m_socket, boost::asio::buffer(m_writeQueue.front() + "\n"),
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
          m_writeQueue.pop_front();
          if (!m_writeQueue.empty()) {
            doWrite();
          }
        } else {
          m_room.leave(shared_from_this());
        }
      });
}