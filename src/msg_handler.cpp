//
// Created by Blink on 2018/7/26.
//

#include "msg_handler.hpp"
#include "driver.h"
#include "data.h"
namespace msgHandler {
    void handleGet(std::shared_ptr<OpMessage>& opm) {
        int id = opm->id;
        std::string key = opm->key;
        std::string ip = opm->to_ip;
        int port = opm->port;
        Data::getInstance()->get(id,  ip, port, key);
    }

    void handlePut(std::shared_ptr<OpMessage>& opm) {
        int id = opm->id;
        std::string key = opm->key;
        std::string value = opm->value;
        std::string ip = opm->to_ip;
        int port = opm->port;
        Data::getInstance()->put(id, ip, port, key, value);
    }

    void handleGetRet(std::shared_ptr<OpRetMessage>& opm) {
        int id = opm->id;
        int status = opm->status;
        long long timestamp = opm->timestamp;
        std::string value = opm->value;
        Driver::getInstance()->getReturn(id, status, timestamp, value);
        return ;
    }

    void handlePutRet(std::shared_ptr<OpRetMessage>& opm) {
        int id = opm->id;
        int status = opm->status;
        Driver::getInstance()->putReturn(id, status);
        return ;
    }

    // while loop recv msg
    void run() {
        std::shared_ptr<Message> m;
        while (true) {
            m = manager::recv();
            switch (m->type) {
                case m_op: {
                    std::shared_ptr<OpMessage> opm = std::dynamic_pointer_cast<OpMessage>(m);
                    switch (opm->op) {
                        case m_get: {
                            handleGet(opm);
                            break;
                        }
                        case m_put: {
                            handlePut(opm);
                            break;
                        }
                    }
                    break;
                }
                case m_opret: {
                    std::shared_ptr<OpRetMessage> oprm = std::dynamic_pointer_cast<OpRetMessage>(m);
                    switch (oprm->op) {
                        case m_getret: {
                            handleGetRet(oprm);
                            break;
                        }
                        case m_putret: {
                            handlePutRet(oprm);
                            break;
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }


    void sendPut(int id, std::string ip, int port, std::string& key, std::string& value) {
        OpMessage opm(m_op, ip, port, m_put, id, key, value);
        manager::send(std::make_shared<OpMessage>(opm));
        return ;
    }

    void sendGet(int id, std::string ip, int port, std::string& key) {
        OpMessage opm(m_op, ip, port, m_get, id, key);
        manager::send(std::make_shared<OpMessage>(opm));
    }

    void sendPutRet(int id, std::string ip, int port) {

    }

    void sendGetRet(int id, std::string ip, int port) {

    }
}