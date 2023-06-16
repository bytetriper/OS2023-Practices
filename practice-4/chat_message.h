#ifndef CHAT_MESSAGE_H
#define CHAT_MESSAGE_H

#include <cstring>
#include <vector>
#include <algorithm>

class chat_message {
public:
    enum { header_length = 4 };
    enum { max_body_length = 512 };

    chat_message();

    explicit chat_message(const std::string& msg);

    const char* data() const;

    char* data();

    std::size_t length() const;

    const char* body() const;

    char* body();

    std::size_t body_length() const;

    void body_length(std::size_t new_length);

    bool decode_header();

    void encode_header();

    std::vector<std::string> messages() const;

private:
    char data_[header_length + max_body_length];
    std::size_t body_length_;
};

#endif // CHAT_MESSAGE_H
