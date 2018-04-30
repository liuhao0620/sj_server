#pragma once
#include "uv.h"
#include "common_def.h"
#include "simple_logger.h"

namespace sj
{
    class udp_client;
    class udp_client_handle
    {
    public:
        virtual void OnRecv(udp_client* client, char* buf, size_t len) = 0;
        // virtual void OnSent(udp_client* client, char* buf, size_t len) = 0;
    };

    struct udp_client_t_with_handle : public uv_udp_t
    {
        udp_client_handle* _handle;
		udp_client* _client;
    };

    struct udp_client_send_t_with_handle : public uv_udp_send_t
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

        int StartUp(const char * ip, int port, int send_port)
        {
            int ret_code = uv_ip4_addr(ip, port, &_server_addr);
            ASSERT(ret_code == 0);
			if (ret_code != 0) { return ret_code; }
            ret_code = uv_ip4_addr("0.0.0.0", send_port, &_client_addr);
            ASSERT(ret_code == 0);
			if (ret_code != 0) { return ret_code; }
            ret_code = uv_udp_init(uv_default_loop(), &_client);
            ASSERT(ret_code == 0);
			if (ret_code != 0) { return ret_code; }
            ret_code = uv_udp_bind(&_client, (const sockaddr *)&_client_addr, 0);
            ASSERT(ret_code == 0);
			if (ret_code != 0) { return ret_code; }
            ret_code = uv_udp_recv_start(&_client, 
                udp_client::AllocCb,
                udp_client::RecvCb);
            ASSERT(ret_code == 0);
			if (ret_code != 0) { return ret_code; }
            ret_code = uv_queue_work(uv_default_loop(), 
				new uv_work_t, 
				udp_client::Run,
				udp_client::AfterRun);
            ASSERT(ret_code == 0);
			if (ret_code != 0) { return ret_code; }
            return 0;
        }

        int Send(const char* buf, size_t len)
        {
            udp_client_send_t_with_handle* req = new udp_client_send_t_with_handle;
            req->_handle = _client._handle;
            req->_client = this;
            uv_buf_t msg = uv_buf_init((char*)buf, len);
            int ret_code = uv_udp_send(req,
                    &_client,
                    &msg,
                    1,
                    (const sockaddr*) &_server_addr,
                    udp_client::SendCb);
            ASSERT(ret_code == 0);
            // ret_code = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
            // ASSERT(ret_code == 0);
            return 0;
        }

        int Close()
        {
			uv_close((uv_handle_t *)&_client, udp_client::CloseCb);
            return 0;
        }

    private:
        static void AllocCb(uv_handle_t* handle, 
            size_t suggested_size, 
            uv_buf_t* buf)
        {
            buf->base = new char[1000];
            buf->len = 1000;
        }

        static void RecvCb(uv_udp_t* handle,
            ssize_t nread,
            const uv_buf_t* rcvbuf,
            const sockaddr* addr,
            unsigned flags)
        {
            if (nread <= 0)
            {
                return;
            }
			if (handle == NULL)
			{
				return;
			}
			udp_client_t_with_handle* uwh = static_cast<udp_client_t_with_handle *>(handle); 
            uwh->_handle->OnRecv(uwh->_client, rcvbuf->base, nread);
            delete rcvbuf->base;
        }

        static void SendCb(uv_udp_send_t* req, int status)
        {
            LOG_DEBUG("send success");
            // udp_client_send_t_with_handle* uswh = static_cast<udp_client_send_t_with_handle*>(req);
            // uswh->_handle->OnSent(uswh->_client, uswh->bufs[0].base, uswh->bufs[0].len);
            delete req;
        }

		static void CloseCb(uv_handle_t* handle) 
		{
			uv_is_closing(handle);
		}

		static void Run(uv_work_t * req)
		{
			ASSERT(uv_run(uv_default_loop(), UV_RUN_DEFAULT) == 0);
		}

		static void AfterRun(uv_work_t * req, int status)
		{
            delete req;
		}

    private:
        sockaddr_in _server_addr;
        sockaddr_in _client_addr;
        udp_client_t_with_handle _client;
    };
}
