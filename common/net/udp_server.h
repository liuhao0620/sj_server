#pragma once
#include <map>
#include "uv.h"
#include "common_def.h"
#include "lock.h"
#include "unique_id.h"

namespace sj
{
    struct udp_session
    {
        unid_t sid;
        const sockaddr* addr;
    };

	class udp_server;
    class udp_server_handle
    {
    public:
        virtual void OnRecv(udp_server* server, unid_t sid, char* buf, size_t len) = 0;
    };

    struct udp_server_t_with_handle : public uv_udp_t
    {
        udp_server_handle* _handle;
		udp_server* _server;
    };

    class udp_server
    {
    public:
        int Init(udp_server_handle* ush)
        {
            _server._handle = ush;
			_server._server = this;
            return 0;
        }
        
		int StartUp(const char* ip, int port)
        {
            sockaddr_in addr;
            int ret_code = uv_ip4_addr(ip, port, &addr);
            ASSERT(ret_code == 0);
			if (ret_code != 0) { return ret_code; }
            ret_code = uv_udp_init(uv_default_loop(), &_server);
            ASSERT(ret_code == 0);
			if (ret_code != 0) { return ret_code; }
            ret_code = uv_udp_bind(&_server, (const sockaddr *)&addr, 0);
            ASSERT(ret_code == 0);
			if (ret_code != 0) { return ret_code; }
            ret_code = uv_udp_recv_start(&_server, 
                udp_server::AllocCb,
                udp_server::RecvCb);
            ASSERT(ret_code == 0);
			if (ret_code != 0) { return ret_code; }
            ret_code = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
            ASSERT(ret_code == 0);
			if (ret_code != 0) { return ret_code; }
            return 0;
        }
        
		int Stop()
        {
            int ret_code = uv_udp_recv_stop(&_server);
            ASSERT(ret_code == 0);
			if (ret_code != 0) { return ret_code; }
			uv_close((uv_handle_t *)&_server, udp_server::CloseCb);
            rw_lock_wguard _l(_session_map_lock);
			for (map_sid_session_t::iterator iter = _sid_session_map.begin();
				iter != _sid_session_map.end();
				++ iter)
			{
				delete iter->second;
			}
			_sid_session_map.clear();
			_addr_session_map.clear();
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
			udp_server_t_with_handle* uwh = static_cast<udp_server_t_with_handle *>(handle); 
            udp_session* session = NULL;
            if (!uwh->_server->FindSession(addr, session))
			{
				uwh->_server->AddSession(addr, session);
			}
			ASSERT(session != NULL);
            uwh->_handle->OnRecv(uwh->_server, session->sid, rcvbuf->base, nread);
            delete rcvbuf->base;
        }

		static void CloseCb(uv_handle_t* handle) 
		{
			uv_is_closing(handle);
		}

		bool FindSession(const unid_t sid, udp_session*& session)
		{
			rw_lock_rguard _l(_session_map_lock);
			map_sid_session_t::iterator iter = _sid_session_map.find(sid);
			if (iter == _sid_session_map.end())
			{
				return false;
			}
			session = iter->second;
			return true;
		}

		bool FindSession(const sockaddr* addr, udp_session*& session)
		{
			rw_lock_rguard _l(_session_map_lock);
			map_addr_session_t::iterator iter = _addr_session_map.find(addr);
			if (iter == _addr_session_map.end())
			{
				return false;
			}
			session = iter->second;
			return true;
		}

		//如果已经存在,则返回false,不存在则增加, 返回true
		//无论存在与否,session都会返回该地址对应的udp_session
		bool AddSession(const sockaddr* addr, udp_session*& session)
		{
			rw_lock_wguard _l(_session_map_lock);
			map_addr_session_t::iterator iter = _addr_session_map.find(addr);
			if (iter != _addr_session_map.end())
			{
				session = iter->second;
				return false;
			}
			session = new udp_session;
			session->sid = GetUniqueID(UIT_UDP_SID);
			session->addr = addr;
			_sid_session_map.insert(std::make_pair(session->sid, session));
			_addr_session_map.insert(std::make_pair(session->addr, session));
			return true;
		}

	public:
		bool DelSession(unid_t sid)
		{
			rw_lock_wguard _l(_session_map_lock);
			map_sid_session_t::iterator iter = _sid_session_map.find(sid);
			if (iter == _sid_session_map.end())
			{
				return false;
			}
			_addr_session_map.erase(iter->second->addr);
			delete iter->second;
			_sid_session_map.erase(sid);
			return true;
		}

    private:
        typedef std::map<unid_t, udp_session*> map_sid_session_t;
        typedef std::map<const sockaddr*, udp_session*> map_addr_session_t;
        map_sid_session_t _sid_session_map;
        map_addr_session_t _addr_session_map;
        rw_lock _session_map_lock;

        udp_server_t_with_handle _server;
    };
}
