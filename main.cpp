#include <iostream>
#include "src/network/tcp_server.hpp"
#include "src/network/manager.hpp"
#include "src/network/tcp_connection.hpp"
#include <thread>
namespace as = boost::asio;



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

    return 0;
}
