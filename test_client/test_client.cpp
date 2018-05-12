#include <iostream>
#include <unistd.h>
#include "udp_client.h"
#include "proto_buf.h"
#include "simple_logger.h"

class test_uc_handle : public sj::udp_client_handle
{
public:
    virtual void OnRecv(sj::udp_client* client, char* buf, size_t len)
    {
        ARRAY2PB(buf, len, proto_buf, pb)
        LOG_DEBUG("recv ", pb.pb_type(), " from server");
    }

    virtual void OnSent(sj::udp_client* client, char* buf, size_t len)
    {
        LOG_DEBUG("sent to server"); 
    }
};

int main(int argc, char ** argv)
{
    sj::udp_client test_client;
    auto test_handle = new test_uc_handle;
    sj::udp_client_config test_cfg;
    test_cfg._server_ip = "127.0.0.1";
    test_cfg._server_port = 1019;
    test_cfg._port = 10001;
    test_cfg._name = "test_client";
    test_client.Init(test_cfg);
    test_client.SetHandle(test_handle);
    test_client.StartUp();
    while (true)
    {
        //主循环
        std::cout << "please enter your command:" << std::endl;
        std::string str;
        std::cin >> str;
        MAKE_PB(ept_test, cmd)
        cmd.set_somewords(str);
        PB2STR(cmd, data_str, )
        PB::proto_buf send_pb;
        send_pb.set_pb_type(cmd.type());
        send_pb.set_pb_data(data_str);
        PB2STR(send_pb, send_str, )
        std::cout << send_str.size() << " " << send_str << std::endl;
        test_client.Send(send_str.c_str(), send_str.size());
        usleep(1000);
    }
    test_client.Close();
    delete test_handle;
    sleep(5);
    return 0;
}