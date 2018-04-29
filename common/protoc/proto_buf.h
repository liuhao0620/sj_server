#pragma once
#include "gprotoc/internal_protocol.pb.h"
#include "gprotoc/external_protocol.pb.h"

#define MAKE_PB(ptype, pname) \
    PB::ptype pname; \
    pname.set_type(PB::PT_##ptype);

#define PB2STR(pname, sname) \
    std::string sname; \
    try \
    { \
        PB::proto_buf __pb_temp; \
        std::string __pb_data_temp; \
        __pb_temp.set_pb_type(pname.type()); \
        pname.SerializeToString(&__pb_data_temp); \
        __pb_temp.set_pb_data(__pb_data_temp); \
        __pb_temp.SerializeToString(&sname); \
    } \
    catch(...) \
    {}

#define STR2PB(sname, ptype, pname) \
    PB::ptype pname; \
    pname.ParseFromString(sname);

#define ARRAY2PB(adata, alen, ptype, pname) \
    PB::ptype pname; \
    pname.ParseFromArray((const void*)adata, alen);
