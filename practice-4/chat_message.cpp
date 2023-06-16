#include "chat_message.h"

chat_message::chat_message() : body_length_(0) {}

chat_message::chat_message(const std::string& msg) : body_length_(msg.length()) {
    std::memcpy(body(), msg.c_str(), body_length_);
    encode_header();
}

const char* chat_message::data() const {
    return data_;
}

char* chat_message::data() {
    return data_;
}

std::size_t chat_message::length() const {
    return header_length + body_length_;
}

const char* chat_message::body() const {
    return data_ + header_length;
}

char* chat_message::body() {
    return data_ + header_length;
}

std::size_t chat_message::body_length() const {
    return body_length_;
}

void chat_message::body_length(std::size_t new_length) {
    body_length_ = new_length;
    if (body_length_ > max_body_length) {
        body_length_ = max_body_length;
    }
}

bool chat_message::decode_header() {
    std::memcpy(&body_length_, data_, header_length);
    if (body_length_ > max_body_length) {
        body_length_ = 0;
        return false;
    }
    return true;
}

void chat_message::encode_header() {
    std::memcpy(data_, &body_length_, header_length);
}

std::vector<std::string> chat_message::messages() const {
    std::vector<std::string> msgs;
    std::string msg(body(), body_length());
    std::size_t pos = 0, last_pos = 0;
    while ((pos = msg.find('\n', last_pos)) != std::string::npos) {
        msgs.push_back(msg.substr(last_pos, pos - last_pos));
        last_pos = pos + 1;
    }
    if (last_pos < msg.length()) {
        msgs.push_back(msg.substr(last_pos));
    }
    return msgs;
}
