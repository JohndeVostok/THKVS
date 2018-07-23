//
// Created by Blink on 2018/7/23.
//

#include "tcp_connection.hpp"

TcpConnection::TcpConnection(as::io_service &io_service, as::ip::address ip): peerIp(ip), socket_(io_service) {}
