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
    void handleSetFlag(std::shared_ptr<OpEnableFlagMessage>& oefm);

    void handleGetRet(std::shared_ptr<OpRetMessage>& opm);
    void handlePutRet(std::shared_ptr<OpRetMessage>& opm);
    void handleSetFlagRet(std::shared_ptr<OpRetMessage>& opm);
    // while loop recv msg
    void run();


    void sendPut(int id, std::string localip, int localport, std::string ip, int port, std::string& key, std::string& value);

    void sendGet(int id, std::string localip, int localport, std::string ip, int port, std::string& key);

    void sendPutRet(int id, std::string ip, int port, int status);

    void sendGetRet(int id, std::string ip, int port, int status, std::string value, long long time_stamp);

    void sendSetEnableFlag(int id, std::string localip, int localport, std::string ip, int port, bool flag);

    void sendSetEnableFlagRet(int id, std::string ip, int port, int status);
}


#endif //THKVS_MSG_HANDLER_HPP
