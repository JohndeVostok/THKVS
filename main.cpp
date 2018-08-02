#include <iostream>
#include "src/network/tcp_server.hpp"
#include "src/network/manager.hpp"
#include "src/network/tcp_connection.hpp"
#include <thread>
#include "src/msg_handler.hpp"
#include "src/data.h"
#include "src/driver.h"
#include "worker.h"
namespace as = boost::asio;



void startIO(std::shared_ptr<as::io_service> io_service) {
    try {
        io_service->run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void send_thread(std::shared_ptr<as::io_service>& io_service) {
    while (true) {
        auto conn = TcpConnection::create(*io_service);
        conn->write_message();
    }
}

int main() {
    std::cout << "THKVS" << std::endl;

    std::shared_ptr<as::io_service> io_service(new as::io_service);
    //TcpServer server(*io_service);
    TcpServer *server = NULL;
    int portNum;
    std::cin >> portNum;
    server = new TcpServer(*io_service, portNum);
    auto thread_server = std::thread(&startIO, std::ref(io_service));
    auto thread_write = std::thread(&send_thread, std::ref(io_service));
    auto thread_data = std::thread(&Data::run, Data::getInstance());
    thread_server.detach();
    thread_data.detach();
    thread_write.detach();

    unsigned concurentThreadsSupported = std::thread::hardware_concurrency() - 10;
    unsigned nthread = concurentThreadsSupported >> 1;

    for (int i = 0; i < nthread; i++) {
		auto thread_handler = std::thread(&msgHandler::run);
		thread_handler.detach();
		auto thread_worker = std::thread(&Worker::run);
		thread_worker.detach();
    }

	Worker::insertPut("sb", "caonima");
	Worker::insertGet("sb");
	Worker::insertGet("sc");
	/*
    while (true) {
        std::string op, key, value;
        std::cin >> op;
        if (op == "put") {
            std::cin >> key >> value;
            Driver::getInstance()->put(key, value);
        } else if (op == "get") {
            std::cin >> key;
            Driver::getInstance()->get(key);
        } else if (op == "set") {
            std::cin >> key;
            if (key == "0") {
                Driver::getInstance()->setEnableFlag(false);
            } else {
                Driver::getInstance()->setEnableFlag(true);
            }
        } else if (op == "add") {
            std::string hostname, ip;
            int port;
            std::cin >> hostname >> ip >> port;
            Driver::getInstance()->addServer(hostname, ip, port);
        } else if (op == "test") {
            Driver::getInstance()->test();
        } else if (op == "remove") {
            std::string hostname;
            std::cin >> hostname;
            Driver::getInstance()->removeServer(hostname);
        }
    }
    std::string key = "sb", value = "caonima";
    Driver::getInstance()->put(key, value);
    Driver::getInstance()->get(key);
	*/
    /*
    test ptest;
    auto thread_test = std::thread(&test::run, &ptest);
    thread_test.join();
    */
     return 0;
}
