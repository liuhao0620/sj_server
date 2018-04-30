#include <iostream>
#include "message_handler.h"
#include "simple_logger.h"

namespace fgs
{
    void message_handler::AddMessage(PB::proto_buf& pb)
    {
        sj::mutex_lock_guard l(_mq_lock);
        _msg_queue.push_back(pb);
    }

    bool message_handler::HandleMessage()
    {
        PB::proto_buf pb;
        {
            sj::mutex_lock_guard l(_mq_lock);
            if (_msg_queue.empty())
            {
                return false;
            }
            pb = _msg_queue.front();
            _msg_queue.pop_front();
        }

        switch (pb.pb_type())
        {
        case PB::PT_ept_test:
        {
            STR2PB(pb.pb_data(), ept_test, cmd)
            LOG_DEBUG("PT_ept_test words : ", cmd.somewords());
            break;
        }
        default:
            LOG_ERROR("error message type : ", pb.pb_type());
            break;
        }
        return true;
    }
}