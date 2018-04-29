#include <iostream>
#include <unistd.h>
#include "udp_client.h"
#include "proto_buf.h"

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
    test_client.StartUp("127.0.0.1", 1019, 10001);
    // test_client.StartUp("120.27.11.202", 1019, 10001);
    while (true)
    {
        //主循环
        std::string str;
        std::cin >> str;
        MAKE_PB(ept_test, cmd)
        cmd.set_somewords(str);
        PB2STR(cmd, send_str)
        std::cout << send_str.size() << " " << send_str << std::endl;
        test_client.Send(send_str.c_str(), send_str.size());
        sleep(5);
    }
    test_client.Close();
    delete test_handle;
    return 0;
}