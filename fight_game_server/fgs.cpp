#include "fgs.h"
#include <unistd.h>
#include "udp_server.h"
#include "message_handler.h"
#include "simple_logger.h"

namespace fgs
{
    class client_server_handle : public sj::udp_server_handle
    {
    public:
        void OnRecv(sj::udp_server* server, unid_t sid, char* buf, size_t len)
        {
            ARRAY2PB(buf, len, proto_buf, pb)
            pb.set_from_id(sid);
            LOG_DEBUG("recv type : ", pb.pb_type(), " from ", sid, " by thread ", pthread_self());
            fgs::message_handler::GetInstance().AddMessage(pb);
            Send2Client(sid, pb);
        }

        void OnSent(sj::udp_server* server, unid_t sid, char* buf, size_t len)
        {
            LOG_DEBUG("sent to ", sid, " success");
        }
    };

    static sj::udp_server _client_server;
    static client_server_handle _client_server_handle;

    bool Init(const char * config_file)
    {
        sj::udp_server_config test_config;
        test_config._port = 1019;
        test_config._thread_num = 4;
        test_config._name = "test_server";

        _client_server.Init(test_config);
        _client_server.SetHandle(&_client_server_handle);
        _client_server.StartUp();
        return true;
    }

    bool Update()
    {
        if (!message_handler::GetInstance().HandleMessage())
        {
            usleep(1);
        }
        return true;
    }

    bool Close()
    {
        _client_server.Close();
        return true;
    }

    bool Send2Client(unid_t sid, PB::proto_buf& pb)
    {
        PB2STR(pb, send_str, return false)
        return _client_server.Send(sid, send_str.c_str(), send_str.size()) == 0;
    }
}
