//
// Created by Blink on 2018/7/23.
//

#ifndef THKVS_TCP_CONNECTION_HPP
#define THKVS_TCP_CONNECTION_HPP

#include <boost/asio.hpp>

namespace as = boost::asio;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection() = delete;

    static std::shared_ptr<TcpConnection> create(as::io_service& io_service, as::ip::address ip) {
        std::shared_ptr<TcpConnection> tmp = std::shared_ptr<TcpConnection> (new TcpConnection(io_service, ip));
        return tmp;
    }

    std::shared_ptr<TcpConnection> ptr() {
        return shared_from_this();
    }

    as::ip::tcp::socket& socket() {
        return socket_;
    }

private:
    TcpConnection(as::io_service& io_service, as::ip::address ip);
    as::ip::address peerIp;
    as::ip::tcp::socket socket_;
};


#endif //THKVS_TCP_CONNECTION_HPP
