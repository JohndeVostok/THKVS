//
// Created by Blink on 2018/7/23.
//

#include <iostream>
#include "tcp_server.hpp"

void TcpServer::start_accept() {
    std::shared_ptr<TcpConnection> newConn = TcpConnection::create(acceptor_.get_io_service());
    //std::cout << "[DEBUG] started accept socket" << std::endl;
    acceptor_.async_accept(newConn->socket(), boost::bind(&TcpServer::handle_accept, this, newConn, boost::asio::placeholders::error));
}

void TcpServer::handle_accept(std::shared_ptr<TcpConnection> conn, const boost::system::error_code &error) {
    if (!error) {
        conn->init_from_accept();
    } else {
        std::cout << boost::system::system_error(error).what() << std::endl;
    }
    start_accept();
}
