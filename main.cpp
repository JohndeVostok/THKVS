#include <iostream>
#include "src/network/tcp_server.hpp"
#include "src/network/manager.hpp"
#include "src/network/tcp_connection.hpp"
#include <thread>
namespace as = boost::asio;


void recv_and_print() {
    std::shared_ptr<Message> m;
    while (true) {
        m = manager::recv();
        switch (m->type) {
            case m_text :  {
                std::shared_ptr<TextMessage> tm = std::dynamic_pointer_cast<TextMessage>(m);

                break;
            }
            case m_op : {
                std::shared_ptr<OpMessage> opm = std::dynamic_pointer_cast<OpMessage>(m);

                break;
            }
        }
        if (m->type == m_text) {
            std::shared_ptr<TextMessage> tm = std::dynamic_pointer_cast<TextMessage>(m);
            std::cout << tm->msg << std::endl;
        }
    }
}

void send_thread(std::shared_ptr<as::io_service>& io_service) {
    while (true) {
        auto conn = TcpConnection::create(*io_service);
        conn->write_message();
    }
}
void test() {
    std::string ip, msg, key, value;
    int port, op, id;

    while (true) {
        std::cin >> ip;
        std::cin >> port;
        std::cin >> msg;
        TextMessage tm(m_text, ip, port, msg);
        OpMessage opm(m_op, ip, port, op, id, key, value);
        manager::send(std::make_shared<OpMessage>(opm));
        manager::send(std::make_shared<TextMessage>(tm));
    }


    return ;
}


void startIO(std::shared_ptr<as::io_service> io_service) {
    try {
        io_service->run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Hello, World!" << std::endl;

    std::shared_ptr<as::io_service> io_service(new as::io_service);
    //TcpServer server(*io_service);
    TcpServer *server = NULL;
    int portNum;
    std::cin >> portNum;
    server = new TcpServer(*io_service, portNum);
    auto thread_server = std::thread(&startIO, std::ref(io_service));
    auto thread_write = std::thread(&send_thread, std::ref(io_service));
    auto thread_read = std::thread(&recv_and_print);
    thread_server.detach();
    thread_write.detach();
    thread_read.detach();

    test();
    return 0;
}
