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

}

#endif //THKVS_MANAGER_HPP
