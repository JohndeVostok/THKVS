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
    std::cout << "[DEBUG] serialized message: " << read_msg.body() << std::endl;
    boost::archive::text_iarchive ia(istream);
    switch (read_msg.message_type()) {
        case m_text: {
            TextMessage tM;
            ia >> tM;
            msgToRecv = std::make_shared<TextMessage>(tM);
            break;
        }
        case m_op: {
            OpMessage opM;
            ia >> opM;
            msgToRecv = std::make_shared<OpMessage>(opM);
            break;
        }
        case m_opret: {
            OpRetMessage oprM;
            ia >> oprM;
            msgToRecv = std::make_shared<OpRetMessage>(oprM);
            break;
        }
        default: {
            break;
        }
    }
    manager::recvQue.push(msgToRecv);
    finalize(boost::system::errc::make_error_code(boost::system::errc::success));
}

void TcpConnection::finalize(const boost::system::error_code& error) {
    if (error) {
        std::cout << boost::system::system_error(error).what() << std::endl;
    }
    else {
        boost::system::error_code ec;
        socket_.shutdown(as::ip::tcp::socket::shutdown_both, ec);
    }
}


void TcpConnection::write_message() {

    std::shared_ptr<Message> msgToSend;
    manager::get_writable_msg(msgToSend);

    auto ip = as::ip::address::from_string(msgToSend->to_ip);
    as::ip::tcp::endpoint endpoint(ip, msgToSend->port);
    boost::system::error_code error;
    socket_.connect(endpoint, error);
    if (error) {
        std::cout << boost::system::system_error(error).what() << std::endl;
        return ;
    }

    std::ostringstream archive_stream;
    boost::archive::text_oarchive archive(archive_stream);

    std::shared_ptr<TextMessage> tm;
    std::shared_ptr<OpMessage> opm;
    std::shared_ptr<OpRetMessage> oprm;
    switch (msgToSend->type) {
        case m_text: {
            tm = std::dynamic_pointer_cast<TextMessage>(msgToSend);
            archive << (*tm);
            break;
        }
        case m_op : {
            opm = std::dynamic_pointer_cast<OpMessage>(msgToSend);
            archive << (*opm);
            break;
        }
        case m_opret: {
            oprm = std::dynamic_pointer_cast<OpRetMessage>(msgToSend);
            archive << (*oprm);
            break;
        }
        default: {
            break;
        }
    }


    size_t write_body_size = archive_stream.str().length();
    char write_msg_type = (char)msgToSend->type;
    std::string write_serialized_msg = archive_stream.str();
    buffers.push_back(as::const_buffer(&write_body_size, 4));
    buffers.push_back(as::const_buffer(&write_msg_type, 1));
    // TODO: may not copy
    std::cout << "[DEBUG] serialized message: " << write_serialized_msg << std::endl;
    buffers.push_back(as::const_buffer(write_serialized_msg.c_str(), write_body_size));

    as::async_write(socket_, buffers, boost::bind(&TcpConnection::finalize, shared_from_this(), boost::asio::placeholders::error));
}






