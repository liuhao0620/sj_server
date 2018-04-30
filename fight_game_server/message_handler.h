#pragma once
#include <deque>
#include "lock.h"
#include "proto_buf.h"

namespace PB
{
    class proto_buf;
}

namespace fgs
{
    class message_handler
    {
    public:
        static message_handler& GetInstance()
        {
            static message_handler _instance;
            return _instance;
        }

    public:
        void AddMessage(PB::proto_buf& pb);
        bool HandleMessage();

    private:
        std::deque<PB::proto_buf> _msg_queue;
        sj::mutex_lock _mq_lock;
    };
}
