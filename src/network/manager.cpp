//
// Created by Blink on 2018/7/24.
//

#include "manager.hpp"

namespace manager {
    MsgQueue<std::shared_ptr<Message> > recvQue;
    MsgQueue<std::shared_ptr<Message> > sendQue;

    std::shared_ptr<Message> recv() {
        std::shared_ptr<Message> msg;
        recvQue.pop(msg);
        return msg;
    }

    bool send(std::shared_ptr<Message> msg) {
        sendQue.push(msg);
    }
}