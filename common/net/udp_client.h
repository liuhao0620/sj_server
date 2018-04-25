#pragma once
#include "uv.h"
#include "common_def.h"

namespace sj
{

    class udp_client;
    class udp_client_handle
    {
    public:
        virtual void OnRecv(udp_client* client, char* buf, size_t len) = 0;
    };

    struct udp_client_t_with_handle : public uv_udp_t
    {
        udp_client_handle* _handle;
		udp_client* _client;
    };

    class udp_client
    {
    public:
        int Init(udp_client_handle* uch)
        {
            _client._handle = uch;
            _client._client = this;
            return 0;
        }

        int StartUp(const char * ip, int port)
        {
            int ret_code = uv_ip4_addr(ip, port, &_server_addr);
            ASSERT(ret_code == 0);
            ret_code = uv_udp_init(uv_default_loop(), &_client);
            ASSERT(ret_code == 0);
            return 0;
        }

        int Send(const char* buf, size_t len)
        {
            uv_udp_send_t req;
            uv_buf_t msg = uv_buf_init((char*)buf, len);
            int ret_code = uv_udp_send(&req,
                  &_client,
                  &msg,
                  1,
                  (const sockaddr*) &_server_addr,
                  udp_client::SendCb);
            ASSERT(ret_code == 0);
            ret_code = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
            ASSERT(ret_code == 0);
            return 0;
        }

        int Close()
        {
			uv_close((uv_handle_t *)&_client, udp_client::CloseCb);
            return 0;
        }

    private:
        static void SendCb(uv_udp_send_t* req, int status)
        {

        }

		static void CloseCb(uv_handle_t* handle) 
		{
			uv_is_closing(handle);
		}

    private:
        sockaddr_in _server_addr;
        udp_client_t_with_handle _client;
    };
}
