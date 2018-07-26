//
// Created by Blink on 2018/7/26.
//

#ifndef THKVS_MSG_HANDLER_HPP
#define THKVS_MSG_HANDLER_HPP


#include <string>
#include "network/manager.hpp"

namespace msgHandler {

    void handleGet(std::shared_ptr<OpMessage>& opm) {
        //TODO: data::get()
    }

    void handlePut(std::shared_ptr<OpMessage>& opm) {
        //TODO: data::put();
    }

    void handleGetRet(std::shared_ptr<OpRetMessage>& opm) {

        //TODO: driver::getret();
    }

    void handlePutRet(std::shared_ptr<OpRetMessage>& opm) {
        //TODO: driver::putret();
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
                            break;
                        }
                        case m_putret: {
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
}


#endif //THKVS_MSG_HANDLER_HPP
