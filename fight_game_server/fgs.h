#pragma once
#include "common_def.h"
#include "proto_buf.h"

namespace fgs
{
    bool Init(const char * config_file);
    bool Update();
    bool Close();
    bool Send2Client(unid_t sid, PB::proto_buf& pb);
    template < class PBTYPE >
    bool Send2Client(unid_t sid, PBTYPE& pb)
    {
        PB2STR(pb, data_str, return false)
        PB::proto_buf send_pb;
        send_pb.set_pb_type(pb.type());
        send_pb.set_pb_data(data_str);
        return Send2Client(sid, send_pb);
    }
}
