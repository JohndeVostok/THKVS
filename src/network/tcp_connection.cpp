//
// Created by Blink on 2018/7/23.
//

#include <iostream>
#include "tcp_connection.hpp"

TcpConnection::TcpConnection(as::io_service &io_service): socket_(io_service) {}

void TcpConnection::init_from_accept() {
    peerIp = socket_.remote_endpoint().address();
    start_handle_message();
}

void TcpConnection::start_handle_message() {
    std::cout << "[DEBUG] started reading header" << std::endl;
    as::async_read(socket_, as::buffer(read_msg.data(), SerialzedMessage::HEADER_SIZE),
                   boost::bind(&TcpConnection::handle_header, shared_from_this(), as::placeholders::error));
}

void TcpConnection::handle_header(const boost::system::error_code& error) {
    std::cout << "[DEBUG] started handle header and read body" << std::endl;
    if (!error & read_msg.decode_header()) {
        std::cout << "[DEBUG] decoded header: body_size: " << read_msg.body_length() << std::endl;
        std::cout << "[DEBUG] decoded header: msg_type: " << read_msg.message_type() << std::endl;
        as::async_read(socket_, as::buffer(read_msg.body(), read_msg.body_length()),
                       boost::bind(&TcpConnection::handle_body, shared_from_this(), as::placeholders::error));

    } else {
        std::cout << boost::system::system_error(error).what() << std::endl;
    }

    return ;
}

void TcpConnection::handle_body(const boost::system::error_code& error) {
    if (error) {
        std::cout << boost::system::system_error(error).what() << std::endl;
        return ;
    }
    std::cout << "[DEBUG] started handle body" << std::endl;
    std::shared_ptr<Message> msgToRecv;
    std::istringstream istream(read_msg.body());
    std::string tmp = std::string(read_msg.body());
    boost::archive::text_iarchive ia(istream);
    
}




