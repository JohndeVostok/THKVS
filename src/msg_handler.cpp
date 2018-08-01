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
        std::string ip = opm->srcip;
        int port = opm->srcport;
        Data::getInstance()->get(id,  ip, port, key);
    }

    void handlePut(std::shared_ptr<OpMessage>& opm) {
        int id = opm->id;
        std::string key = opm->key;
        std::string value = opm->value;
        std::string ip = opm->srcip;
        int port = opm->srcport;
        Data::getInstance()->put(id, ip, port, key, value);
    }

    void handleSetFlag(std::shared_ptr<OpEnableFlagMessage>& oefm) {
        Driver::getInstance()->actSetEnableFlag(oefm->flag);
        int id = oefm->id;
        std::string ip = oefm->srcip;
        int port = oefm->srcport;
        int status = 0;
        sendSetEnableFlagRet(id, ip, port, status);
    }

    void handleAddServer(std::shared_ptr<OpAddServerMessage> &oasm) {
        std::string hostname = oasm->hostname;
        std::string hostip = oasm->hostip;
        int hostport = oasm->hostport;
        Driver::getInstance()->actAddServer(hostname, hostip, hostport);
        int id = oasm->id;
        std::string ip = oasm->srcip;
        int port = oasm->srcport;
        int status = 0;
        sendAddServerRet(id, ip, port, status);
    }

    void handleRemoveServer(std::shared_ptr<OpRemoveServerMessage> &orsm) {
        std::string hostname = orsm->hostname;
        Driver::getInstance()->actRemoveServer(hostname);
        int id = orsm->id;
        std::string ip = orsm->srcip;
        int port = orsm->srcport;
        int status = 0;
        sendRemoveServerRet(id, ip, port, status);
    }

    void handleGetRet(std::shared_ptr<OpRetMessage>& opm) {
        int id = opm->id;
        int status = opm->status;
        long long timestamp = opm->timestamp;
        std::string value = opm->value;
        std::cout << "[DEBUG Handler] After Received GetRet status: " << status << std::endl;
        Driver::getInstance()->getReturn(id, status, timestamp, value);
        return ;
    }

    void handlePutRet(std::shared_ptr<OpRetMessage>& opm) {
        int id = opm->id;
        int status = opm->status;
        std::cout << "[DEBUG Handler] After Received PutRet status: " << status << std::endl;
        Driver::getInstance()->putReturn(id, status);
        return ;
    }

    void handleSetFlagRet(std::shared_ptr<OpRetMessage>& opm) {
        int id = opm->id;
        int status = opm->status;
        Driver::getInstance()->setEnableFlagReturn(id, status);
        return ;
    }

    void handleAddServerRet(std::shared_ptr<OpRetMessage> &opm) {
        int id = opm->id;
        int status = opm->status;
        Driver::getInstance()->addServerReturn(id, status);
        return ;
    }

    void handleRemoveServerRet(std::shared_ptr<OpRetMessage> &opm) {
        int id = opm->id;
        int status = opm->status;
        Driver::getInstance()->removeServerReturn(id, status);
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
                        case m_setflagret: {
                            handleSetFlagRet(oprm);
                            break;
                        }
                        case m_addserverret: {
                            handleAddServerRet(oprm);
                            break;
                        }
                        case m_removeserverret: {
                            handleRemoveServerRet(oprm);
                            break;
                        }
                    }
                    break;
                }
                case m_setflag: {
                    std::shared_ptr<OpEnableFlagMessage> oefm = std::dynamic_pointer_cast<OpEnableFlagMessage>(m);
                    handleSetFlag(oefm);
                    break;
                }
                case m_addserver: {
                    std::shared_ptr<OpAddServerMessage> oasm = std::dynamic_pointer_cast<OpAddServerMessage>(m);
                    handleAddServer(oasm);
                    break;
                }
                case m_removeserver: {
                    std::shared_ptr<OpRemoveServerMessage> orsm = std::dynamic_pointer_cast<OpRemoveServerMessage>(m);
                    handleRemoveServer(orsm);
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }


    void sendPut(int id, std::string localip, int localport, std::string ip, int port, std::string& key, std::string& value) {
        OpMessage opm(m_op, ip, port, localip, localport, m_put, id, key, value);
        manager::send(std::make_shared<OpMessage>(opm));
        return ;
    }

    void sendGet(int id, std::string localip, int localport, std::string ip, int port, std::string& key) {
        OpMessage opm(m_op, ip, port, localip, localport, m_get, id, key);
        manager::send(std::make_shared<OpMessage>(opm));
    }

    void sendPutRet(int id, std::string ip, int port, int status) {
        std::cout << "[DEBUG Handler] Before sendPutRet status: " << status << std::endl;
        OpRetMessage oprm(m_opret, ip, port, m_putret, status);
        manager::send(std::make_shared<OpRetMessage>(oprm));
    }

    void sendGetRet(int id, std::string ip, int port, int status, std::string value, long long time_stamp) {
        std::cout << "[DEBUG Handler] Before sendGetRet status: " << status << std::endl;
        OpRetMessage oprm(m_opret, ip, port, m_getret, id, status, value, time_stamp);
        manager::send(std::make_shared<OpRetMessage>(oprm));
    }

    void sendSetEnableFlag(int id, std::string localip, int localport, std::string ip, int port, bool flag) {
        OpEnableFlagMessage oefm(m_setflag, ip, port, localip, localport, id, flag);
        manager::send(std::make_shared<OpEnableFlagMessage>(oefm));
        return ;
    }

    void sendSetEnableFlagRet(int id, std::string ip, int port, int status) {
        OpRetMessage oprm(m_opret, ip, port, m_setflagret, id, status);
        manager::send(std::make_shared<OpRetMessage>(oprm));
        return ;
    }

    void sendAddServer(int id, std::string localip, int localport, std::string ip, int port,
                       std::string hostname, std::string hostip, int hostport) {
        OpAddServerMessage oasm(m_addserver, ip, port, localip, localport, id, hostname, hostip, hostport);
        manager::send(std::make_shared<OpAddServerMessage>(oasm));
        return ;
    }

    void sendAddServerRet(int id, std::string ip, int port, int status) {
        OpRetMessage oprm(m_opret, ip, port, m_addserverret, id, status);
        manager::send(std::make_shared<OpRetMessage>(oprm));
        return ;
    }

    void sendRemoveServer(int id, std::string localip, int localport, std::string ip, int port,
                          std::string hostname) {
        OpRemoveServerMessage orsm(m_removeserver, ip, port, localip, localport, id, hostname);
        manager::send(std::make_shared<OpRemoveServerMessage>(orsm));
        return ;
    }

    void sendRemoveServerRet(int id, std::string ip, int port, int status) {
        OpRetMessage oprm(m_opret, ip, port, m_removeserverret, id, status);
        manager::send(std::make_shared<OpRetMessage>(oprm));
        return ;
    }

}