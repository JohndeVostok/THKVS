//
// Created by Blink on 2018/7/24.
//

#ifndef THKVS_MANAGER_HPP
#define THKVS_MANAGER_HPP

#include "msg_queue.hpp"
#include "message.hpp"

namespace manager {
    extern MsgQueue<std::shared_ptr<Message> > recvQue;
    extern MsgQueue<std::shared_ptr<Message> > sendQue;

    std::shared_ptr<Message> recv();

    bool send(std::shared_ptr<Message> msg);

    void get_writable_msg(std::shared_ptr<Message>& msg);

}

#endif //THKVS_MANAGER_HPP
