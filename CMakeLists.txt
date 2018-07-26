cmake_minimum_required(VERSION 3.5)
project(THKVS)

set(CMAKE_CXX_STANDARD 14)

find_package (Boost COMPONENTS system filesystem serialization thread REQUIRED)
include_directories (${Boost_INCLUDE_DIR})
set(SOURCE_FILES main.cpp src/network/msg_queue.cpp src/network/msg_queue.hpp src/network/tcp_server.cpp src/network/tcp_server.hpp src/network/message.cpp src/network/message.hpp src/network/tcp_connection.cpp src/network/tcp_connection.hpp src/network/manager.cpp src/network/manager.hpp src/msg_handler.cpp src/msg_handler.hpp)

add_executable(THKVS ${SOURCE_FILES})


target_link_libraries(THKVS PUBLIC -lpthread ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
