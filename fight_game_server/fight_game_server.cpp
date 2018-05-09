#include <iostream>
#include <unistd.h>
#include "udp_server.h"
#include "message_handler.h"
#include "simple_logger.h"

class test_us_handle : public sj::udp_server_handle
{
public:
    void OnRecv(sj::udp_server* server, unid_t sid, char* buf, size_t len)
    {
        ARRAY2PB(buf, len, proto_buf, pb)
        pb.set_from_id(sid);
        LOG_DEBUG("recv type : ", pb.pb_type(), " from ", sid, " by thread ", pthread_self());
        fgs::message_handler::GetInstance().AddMessage(pb);
        server->Send(pb.from_id(), "PONG", 4);
        // server->Send(pb.from_id(), "BANG", 4);
    }

    void OnSent(sj::udp_server* server, unid_t sid, char* buf, size_t len)
    {
        LOG_DEBUG("sent to ", sid, " success");
    }
};

int main(int argc, char **argv)
{
    sj::udp_server_config test_config;
    test_config._port = 1019;
    test_config._thread_num = 4;
    test_config._name = "test_server";
    sj::udp_server test_server;
    auto test_handle = new test_us_handle();
    test_server.Init(test_config);
    test_server.SetHandle(test_handle);
    test_server.StartUp();
    while (true)
    {
        //主循环
        if (!fgs::message_handler::GetInstance().HandleMessage())
        {
            usleep(5);
        }
    }
    test_server.Close();
    delete test_handle;
    return 0;
}
