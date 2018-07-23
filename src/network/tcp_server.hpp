//
// Created by Blink on 2018/7/23.
//

#ifndef THKVS_TCP_SERVER_HPP
#define THKVS_TCP_SERVER_HPP

#include <boost/asio.hpp>

namespace as = boost::asio;

class TcpServer {
public:
    explicit TcpServer(as::io_service& io_service, int portNum)
            : acceptor_(io_service, as::ip::tcp::endpoint(as::ip::tcp::v4(), portNum)) {
        start_accept();
    }
private:
    void start_accept();
    as::ip::tcp::acceptor acceptor_;
};


#endif //THKVS_TCP_SERVER_HPP
