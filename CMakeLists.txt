cmake_minimum_required(VERSION 3.12)
project(THKVS)

set(CMAKE_CXX_STANDARD 14)

find_package (Boost COMPONENTS system filesystem serialization thread REQUIRED)
include_directories (${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})
set(SOURCE_FILES main.cpp src/network/msg_queue.cpp src/network/msg_queue.hpp src/network/tcp_server.cpp src/network/tcp_server.hpp src/network/message.cpp src/network/message.hpp src/network/tcp_connection.cpp src/network/tcp_connection.hpp)

add_executable(THKVS ${SOURCE_FILES})


target_link_libraries(THKVS PUBLIC ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} cryptopp ${OPENSSL_LIBRARIES})