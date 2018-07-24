//
// Created by Blink on 2018/7/23.
//

#ifndef THKVS_MESSAGE_HPP
#define THKVS_MESSAGE_HPP


#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <vector>
#include <map>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace as = boost::asio;

enum m_type {
    m_sync = 1,
    m_keepalive = 2,
    m_ack = 3,
    m_text = 4,
    m_share = 5,
    m_plainshare = 6
    // TODO : more type
};

class SerialzedMessage {
public:

    enum { HEADER_SIZE = 5 };
    enum { MAX_BODY_SIZE = 20480 };
    SerialzedMessage() {
        memset(data_, 0, HEADER_SIZE + MAX_BODY_SIZE);
    }

    const char* data() const {
        return data_;
    }

    char* data() {
        return data_;
    }
    size_t length() const {
        return HEADER_SIZE + MAX_BODY_SIZE;
    }

    const char* body() const {
        return data_ + HEADER_SIZE;
    }

    char* body() {
        return data_ + HEADER_SIZE;
    }

    size_t body_length() const {
        return body_size_;
    }

    void body_length(size_t new_length) {
        body_size_ = new_length;
        if (body_size_ > MAX_BODY_SIZE) {
            body_size_ = MAX_BODY_SIZE;
        }
    }

    m_type message_type() {
        return message_type_;
    }

    // TODO : decode type
    bool decode_header() {
        char header[HEADER_SIZE + 1] = "";
        memcpy(header, data_, HEADER_SIZE);
        body_size_ = *((int*)header);
        message_type_ = static_cast<m_type>(*((char*)header + 4));
        // TODO : exception
        if (body_size_ > MAX_BODY_SIZE) {
            body_size_ = 0;
            return false;
        }
        return true;
    }

    void encode_header(m_type type) {
        char header[HEADER_SIZE + 1] = "";
        memcpy(data_, header, HEADER_SIZE);
    }
private:
    char data_[HEADER_SIZE + MAX_BODY_SIZE];
    size_t body_size_;
    m_type message_type_;
};

class Message {
public:
    m_type type;
    int port;
    std::string to_ip;
    Message(m_type _type, std::string _to_ip, int _port)
            : type(_type), port(_port), to_ip(_to_ip) {}
    Message() {}
    virtual ~Message() {}
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & type;
        ar & port;
        ar & to_ip;
    }

};

class TextMessage : public Message {
public:
    std::string msg;
    TextMessage() {}
    TextMessage(std::string _msg): msg(_msg) {}
    TextMessage(m_type _type, std::string _to_ip, int _port,  std::string _msg)
            : Message(_type, _to_ip, _port), msg(_msg) {}

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & msg;
    }
};




#endif //THKVS_MESSAGE_HPP
