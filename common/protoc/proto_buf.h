#pragma once
#include "gprotoc/internal_protocol.pb.h"
#include "gprotoc/external_protocol.pb.h"

#define MAKE_PB(ptype, pname) \
    PB::ptype pname; \
    pname.set_type(PB::PT_##ptype);

#define PB2STR(pname, sname, err_op) \
    std::string sname; \
    try \
    { \
        pname.SerializeToString(&sname); \
    } \
    catch(...) \
    { \
        err_op; \
    }

#define STR2PB(sname, ptype, pname) \
    PB::ptype pname; \
    pname.ParseFromString(sname);

#define ARRAY2PB(adata, alen, ptype, pname) \
    PB::ptype pname; \
    pname.ParseFromArray((const void*)adata, alen);
