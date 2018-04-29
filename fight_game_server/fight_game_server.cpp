#include <iostream>
#include <unistd.h>
#include "udp_server.h"
#include "proto_buf.h"

class test_us_handle : public sj::udp_server_handle
{
public:
    void OnRecv(sj::udp_server* server, unid_t sid, char* buf, size_t len)
    {
        ARRAY2PB(buf, len, proto_buf, cmd)
        std::cout << "recv type : " << cmd.pb_type() << " from " << sid << std::endl;
        server->Send(sid, "PONG", 4);
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
        sleep(5);
    }
    test_server.Stop();
    delete test_handle;
    return 0;
}
