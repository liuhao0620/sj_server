#include "udp_server.h"

class test_us_handle : public sj::udp_server_handle
{
public:
    void OnRecv(sj::udp_server* server, unid_t sid, char* buf, size_t len)
    {}
};

int main(int argc, char **argv)
{
    sj::udp_server test_server;
    auto test_handle = new test_us_handle();
    test_server.Init(test_handle);
    test_server.StartUp("0.0.0.0", 1019);
    return 0;
}
