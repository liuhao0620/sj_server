#include "udp_client.h"
#include <iostream>

class test_uc_handle : public sj::udp_client_handle
{
public:
    virtual void OnRecv(sj::udp_client* client, char* buf, size_t len)
    {
        std::cout << "recv " << buf << " from server" << std::endl;
    }
};

int main(int argc, char ** argv)
{
    sj::udp_client test_client;
    auto test_handle = new test_uc_handle;
    test_client.Init(test_handle);
    test_client.StartUp("127.0.0.1", 1019, 10003);
    // test_client.StartUp("120.27.11.202", 1019, 1020);
    test_client.Send("PING", 4);
    test_client.Close();
    delete test_handle;
    return 0;
}