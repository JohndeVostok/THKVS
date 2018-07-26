//
// Created by Blink on 2018/7/26.
//

#ifndef THKVS_MSG_HANDLER_HPP
#define THKVS_MSG_HANDLER_HPP


#include <string>
#include "network/manager.hpp"

namespace msgHandler {

    void handleGet(std::shared_ptr<OpMessage>& opm);

    void handlePut(std::shared_ptr<OpMessage>& opm);

    void handleGetRet(std::shared_ptr<OpRetMessage>& opm);
    void handlePutRet(std::shared_ptr<OpRetMessage>& opm);

    // while loop recv msg
    void run();


    void sendPut(int id, std::string ip, int port, std::string& key, std::string& value);

    void sendGet(int id, std::string ip, int port, std::string& key);
}


#endif //THKVS_MSG_HANDLER_HPP
