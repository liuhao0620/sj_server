#include <iostream>
#include <unistd.h>
#include "udp_server.h"
#include "message_handler.h"

class test_us_handle : public sj::udp_server_handle
{
public:
    void OnRecv(sj::udp_server* server, unid_t sid, char* buf, size_t len)
    {
        ARRAY2PB(buf, len, proto_buf, pb)
        pb.set_from_id(sid);
        std::cout << "recv type : " << pb.pb_type() << " from " << sid << std::endl;
        fgs::message_handler::GetInstance().AddMessage(pb);
        server->Send(pb.from_id(), "PONG", 4);
        // server->Send(pb.from_id(), "BANG", 4);
    }
};

int main(int argc, char **argv)
{
    sj::udp_server test_server;
    auto test_handle = new test_us_handle();
    test_server.Init(test_handle);
    test_server.StartUp("0.0.0.0", 1019);
    while (true)
    {
        //主循环
        if (!fgs::message_handler::GetInstance().HandleMessage())
        {
            usleep(5);
        }
    }
    test_server.Stop();
    delete test_handle;
    return 0;
}
