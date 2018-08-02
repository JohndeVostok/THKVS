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
#include <boost/serialization/list.hpp>


namespace as = boost::asio;

enum m_type {
    m_sync = 1,
    m_keepalive = 2,
    m_ack = 3,
    m_text = 4,
    m_share = 5,
    m_plainshare = 6,
    m_op = 7,
    m_opret = 8,
    m_join = 9,
    m_leave = 10,
    m_setflag = 11,
    m_move = 12,
    m_datamove = 13,
    m_addserver = 14,
    m_removeserver = 15,
    // TODO : more type
};

enum m_op_type {
    m_put = 1,
    m_putret = 2,
    m_get = 3,
    m_getret = 4,
    m_setflagret = 5,
    m_moveret = 6,
    m_datamoveret = 7,
    m_addserverret = 8,
    m_removeserverret = 9
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

class OpDataMoveMessage :public Message {
public:
    int id;
    std::string srcip;
    int srcport;
    std::list<std::string> key;
    std::list<std::string> value;
    OpDataMoveMessage() {}
    OpDataMoveMessage(m_type _type, std::string _to_ip, int _port, std::string _srcip, int _srcport,
                      int _id, std::list<std::string>& _key, std::list<std::string>& _value)
            : Message(_type, _to_ip, _port), id(_id), srcip(_srcip), srcport(_srcport),
              key(std::move(_key)), value(std::move(_value)) {}

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & id;
        ar & srcip;
        ar & srcport;
        ar & key;
        ar & value;
    }
};

class OpMoveMessage : public Message {
public:
    int id;
    std::string srcip;
    int srcport;
    std::string remoteSrcip;
    int remoteSrcport;
    std::string remoteDstip;
    int remoteDstport;
    unsigned hashBegin;
    unsigned hashEnd;
    bool remove;
    OpMoveMessage() {}
    OpMoveMessage(m_type _type, std::string _to_ip, int _port, std::string _srcip, int _srcport, int _id,
                  std::string _dstip, int _dstport, unsigned _hashBegin, unsigned _hashEnd, bool _remove)
            : Message(_type, _to_ip, _port), srcip(_srcip), srcport(_srcport), id(_id), remoteDstip(_dstip),
              remoteDstport(_dstport), hashBegin(_hashBegin), hashEnd(_hashEnd),
              remoteSrcip(_to_ip), remoteSrcport(_port), remove(_remove){}
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & id;
        ar & srcip;
        ar & srcport;
        ar & remoteSrcip;
        ar & remoteSrcport;
        ar & remoteDstip;
        ar & remoteDstport;
        ar & hashBegin;
        ar & hashEnd;
        ar & remove;
    }
};

class OpRemoveServerMessage : public Message {
public:
    int id;
    std::string srcip;
    int srcport;
    std::string hostname;
    OpRemoveServerMessage() {}
    OpRemoveServerMessage(m_type _type, std::string _to_ip, int _port, std::string _srcip, int _srcport,
    int _id, std::string _hostname)
    : Message(_type, _to_ip, _port), srcip(_srcip), srcport(_srcport), id(_id), hostname(_hostname) {}

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & id;
        ar & srcip;
        ar & srcport;
        ar & hostname;
    }
};

class OpAddServerMessage : public Message {
public:
    int id;
    std::string srcip;
    int srcport;
    std::string hostname;
    std::string hostip;
    int hostport;
    OpAddServerMessage() {}
    OpAddServerMessage(m_type _type, std::string _to_ip, int _port, std::string _srcip, int _srcport,
                       int _id, std::string _hostname, std::string _hostip, int _hostport)
            : Message(_type, _to_ip, _port), srcip(_srcip), srcport(_srcport), id(_id), hostname(_hostname),
              hostip(_hostip), hostport(_hostport) {}

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & id;
        ar & srcip;
        ar & srcport;
        ar & hostname;
        ar & hostip;
        ar & hostport;
    }
};

class OpEnableFlagMessage : public Message {
public:
    int id;
    bool flag;
    std::string srcip;
    int srcport;
    OpEnableFlagMessage() {}
    OpEnableFlagMessage(m_type _type, std::string _to_ip, int _port, std::string _srcip, int _srcport, int _id, bool _flag)
            : Message(_type, _to_ip, _port), srcip(_srcip), srcport(_srcport), id(_id), flag(_flag) {}

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & id;
        ar & flag;
        ar & srcip;
        ar & srcport;
    }
};

class OpMessage : public Message {
public:
    m_op_type op;
    int id;
    std::string key, value;
    std::string srcip;
    int srcport;
    OpMessage() {}
    OpMessage(m_type _type, std::string _to_ip, int _port, std::string _srcip, int _srcport,
              m_op_type _op, int _id, std::string _key, std::string _value = "")
            : Message(_type, _to_ip, _port), srcip(_srcip), srcport(_srcport), op(_op), id(_id), key(_key), value(_value) {}

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & op;
        ar & id;
        ar & key;
        ar & value;
        ar & srcip;
        ar & srcport;
    }
};

class OpRetMessage : public Message {
public:
    m_op_type op;
    int id, status;
    long long timestamp;
    std::string value;
    OpRetMessage() {}
    OpRetMessage(m_type _type, std::string _to_ip, int _port, m_op_type _op, int _id,
                 int _status = 0, std::string _value = "", long long _timestamp = 0)
                         : Message(_type, _to_ip, _port), op(_op), status(_status), id(_id), value(_value), timestamp(_timestamp) {}

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & op;
        ar & id;
        ar & timestamp;
        ar & status;
        ar & value;
    }
};

#endif //THKVS_MESSAGE_HPP
