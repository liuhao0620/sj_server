#pragma once
#include <map>
#include <assert.h>
#include <functional>
#include "uv.h"
#include "common_def.h"
#include "lock.h"

namespace sj
{
    struct udp_session
    {
        unid_t sid;
        sockaddr* addr;
    };

    class udp_server_handle
    {
    public:
        virtual void OnRecv(unid_t sid, char* buf, size_t len) = 0;
    };

    struct udp_t_with_handle : public uv_udp_t
    {
        udp_server_handle* _handle;
    };

    class udp_server
    {
    public:
        int Init(udp_server_handle* ush)
        {
            _server._handle = ush;
            return 0;
        }
        int StartUp(const char* ip, int port)
        {
            sockaddr_in addr;
            uv_loop_t *_loop = uv_default_loop();
            int ret_code = uv_ip4_addr(ip, port, &addr);
            assert(ret_code != 0);
            ret_code = uv_udp_init(_loop, &_server);
            assert(ret_code != 0);
            ret_code = uv_udp_bind(&_server, (const sockaddr *)&addr, 0);
            assert(ret_code != 0);
            ret_code = uv_udp_recv_start(&_server, 
                udp_server::AllocCb,
                udp_server::RecvCb);
            assert(ret_code != 0);

            ret_code = uv_run(_loop, UV_RUN_DEFAULT);
            assert(ret_code != 0);
            return 0;
        }
        int Stop()
        {
            int ret_code = uv_udp_recv_stop(&_server);
            assert(ret_code != 0);
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
            const struct sockaddr* addr,
            unsigned flags)
        {
            if (nread <= 0)
            {
                return;
            }
            unid_t sid = 0;
            udp_t_with_handle* uwh = static_cast<udp_t_with_handle *>(handle);
            
            uwh->_handle->OnRecv(sid, rcvbuf->base, nread);
            delete rcvbuf->base;
        }

    private:
        typedef std::map<unid_t, udp_session*> map_sid_session_t;
        typedef std::map<udp_session*, unid_t> map_session_sid_t;
        map_sid_session_t _sid_session_map;
        map_session_sid_t _session_sid_map;
        rw_lock _ss_lock;

        udp_t_with_handle _server;
    };
}