//
// Created by Blink on 2018/7/23.
//

#ifndef THKVS_TCP_CONNECTION_HPP
#define THKVS_TCP_CONNECTION_HPP

#include <boost/asio.hpp>
#include "message.hpp"
#include "msg_queue.hpp"
#include <boost/bind.hpp>
#include "manager.hpp"

namespace as = boost::asio;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection() = delete;

    static std::shared_ptr<TcpConnection> create(as::io_service& io_service) {
        std::shared_ptr<TcpConnection> tmp = std::shared_ptr<TcpConnection> (new TcpConnection(io_service));
        return tmp;
    }

    std::shared_ptr<TcpConnection> ptr() {
        return shared_from_this();
    }

    as::ip::tcp::socket& socket() {
        return socket_;
    }

    void init_from_accept();

    void start_handle_message();

    void handle_header(const boost::system::error_code& error);

    void handle_body(const boost::system::error_code& error);

    void finalize(const boost::system::error_code& error);

    void write_message();


private:
    TcpConnection(as::io_service& io_service);
    as::ip::address peerIp;
    as::ip::tcp::socket socket_;
    SerialzedMessage read_msg;
    std::vector<as::const_buffer> buffers;
};


#endif //THKVS_TCP_CONNECTION_HPP
