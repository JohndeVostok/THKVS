#include <iostream>
#include "src/network/tcp_server.hpp"
#include "src/network/manager.hpp"
#include "src/network/tcp_connection.hpp"
#include <thread>
#include "src/msg_handler.hpp"
#include "src/data.h"
#include "src/driver.h"
#include "src/worker.h"
#include "cstdlib"

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

    thread_data.detach();
    thread_write.detach();

    unsigned concurentThreadsSupported = std::thread::hardware_concurrency() - 2;
    unsigned nthread = concurentThreadsSupported >> 1;

    for (int i = 0; i < nthread; i++) {
		auto thread_handler = std::thread(&msgHandler::run);
		thread_handler.detach();
		auto thread_worker = std::thread(&Worker::run);
		thread_worker.detach();
    }

	
	std::string op;
	std::cin >> op;

	if (op == "test") {
		std::string key, value;
		
		key = "a";
		value = "a";
		std::cout << "[TEST] throughput(put:get 1:0, length=1, record=1000)" << std::endl;
		Worker::testCnt.store(1000);
		auto starttime = std::chrono::system_clock::now();
		for (int i = 0; i < 1000; i++) {
			//std::cout << "[DEBUG MAIN] put: " << i << " times" << std::endl;
			Worker::insertPut(key, value);
		}
		{
        	unique_lock <mutex> lck(Worker::testMu);
        	Worker::testCond.wait(lck, []() {
        		return Worker::testCnt.load() == 0;
        	});
		}
		auto endtime = std::chrono::system_clock::now();
		std::chrono::duration <double> d = endtime - starttime;
		cout << "[TEST] time: " << d.count() << endl;

		Worker::insertPut("a", "a");
		std::cout << "[TEST] throughput(put:get 0:1) started.";
		Worker::testCnt.store(10000);
		starttime = std::chrono::system_clock::now();
		for (int i = 0; i < 10000; i++) {
			std::cout << "[DEBUG MAIN] get: " << i << " times" << std::endl;
			Worker::insertGet("a");
		}
		{
        	unique_lock <mutex> lck(Worker::testMu);
        	Worker::testCond.wait(lck, []() {
        		return Worker::testCnt.load() == 0;
        	});
		}
		endtime = std::chrono::system_clock::now();
		d = endtime - starttime;
		cout << "[TEST] test result: " << d.count() << endl;
	
	} else if (op == "display") {
	    while (true) {
            std::string op, key, value;
            std::cin >> op;
            if (op == "put") {
                std::cin >> key >> value;
                Worker::insertPut(key, value);
            } else if (op == "get") {
                std::cin >> key;
                Worker::insertGet(key);
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
	}
    thread_server.join();
	
	//Worker::insertPut("sb", "caonima");
	//Worker::insertGet("sb");
	//Worker::insertGet("sc");
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
