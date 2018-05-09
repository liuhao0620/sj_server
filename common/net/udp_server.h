#pragma once
#include <map>
#include <uv.h>
#include "net_base.h"
#include "common_def.h"
#include "lock.h"
#include "unique_id.h"

namespace sj
{
	class udp_server;
	class udp_server_handle
	{
	public:
		virtual void OnRecv(udp_server * server, unid_t sid, char * buf, size_t len) = 0;
		virtual void OnSent(udp_server * server, unid_t sid, char * buf, size_t len) = 0;		
	};

	struct udp_server_config
	{
		int _port;				//本机发送和接受的端口
		int _thread_num = 4;	//服务器线程数，默认为4
		std::string _name;		//服务器名称
	};

#define CHECK_ERR_CODE \
    if (err_code != 0) \
    { \
        _err_info = GetErrorInfo(err_code); \
        return err_code; \
    }    

	class udp_server
	{
	private:
		struct udp_session
		{
			unid_t _sid;
			sockaddr _addr;
		};

        struct uv_udp_t_with_server : public uv_udp_t
        {
            udp_server * _server;
        };

		struct recv_buf
		{
            char _buf[UDP_BUF_MAX_SIZE];
		};

        struct send_buf
        {
            udp_server * _server;
			unid_t _sid;
            size_t _len;
            char _buf[UDP_BUF_MAX_SIZE];
        };

        struct send_param : public uv_udp_send_t
        {
            send_buf * _send_buf;
        };

		typedef std::map<unid_t, udp_session *> map_sid_session_t;
		typedef std::map<unid_t, udp_session *> map_addr_session_t;
	public:
		udp_server()
		{
			_server._server = this;
			_handle = NULL;
			uv_loop_init(&_loop);
		}

		~udp_server()
		{
			uv_loop_close(&_loop);
			rw_lock_wguard l(_session_map_lock);
			for (map_sid_session_t::iterator iter = _session_map_by_sid.begin();
				iter != _session_map_by_sid.end(); ++ iter)
			{
				_session_stack.PutData(iter->second);
			}
			_session_map_by_sid.clear();
			_session_map_by_addr.clear();
		}

		bool Init(udp_server_config& cfg)
		{
			_config._port = cfg._port;
			_config._thread_num = cfg._thread_num;
			_config._name = cfg._name;
			return 0;
		}

		void SetHandle(udp_server_handle* ush)
		{
			_handle = ush;
		}

		int StartUp()
		{
            if (_handle == NULL)
            {
                _err_info = "no hanle";
                return -1;
            }
			_thread = new uv_thread_t[_config._thread_num];
			if (_thread == NULL)
			{
				_err_info = "malloc uv_thread_t failed";
				return -2;
			}
			int err_code = uv_async_init(&_loop, &_async_send, udp_server::AsyncSend);
			CHECK_ERR_CODE
			err_code = uv_async_init(&_loop, &_async_close, udp_server::AsyncClose);
			CHECK_ERR_CODE
			err_code = uv_ip4_addr("0.0.0.0", _config._port, &_self_addr);
			CHECK_ERR_CODE
			err_code = uv_udp_init(&_loop, &_server);
			CHECK_ERR_CODE
			err_code = uv_udp_bind(&_server, (const sockaddr *)&_self_addr, 0);
			CHECK_ERR_CODE
			err_code = uv_udp_recv_start(&_server, udp_server::AllocCb, udp_server::RecvCb);
			CHECK_ERR_CODE
			for (int i = 0; i < _config._thread_num; ++ i)
			{
				err_code = uv_thread_create(&(_thread[i]), udp_server::Run, (void *)this);
				CHECK_ERR_CODE
			}
			return 0;
		}

		int Send(unid_t sid, const char * buf, size_t len)
		{
            if (len > UDP_BUF_MAX_SIZE)
            {
                _err_info = "send buf is too long";
                return -1;
            }
			if (sid == 0)
			{
				_err_info = "send error : invalid sid";
				return -2;
			}
			send_buf * data = _send_buf_stack.GetData();
			data->_sid = sid;
            data->_server = this;
            data->_len = len;
			memcpy((void *)data->_buf, (const void *)buf, len);
			_async_send.data = (void *)data;
            int err_code = uv_async_send(&_async_send);
            CHECK_ERR_CODE
			return 0;
		}

		int Close()
		{
            _async_close.data = (void *)this;
            int err_code = uv_async_send(&_async_close);
            CHECK_ERR_CODE
			return 0;
		}

		bool DelSession(unid_t sid)
		{
			rw_lock_wguard l(_session_map_lock);
			map_sid_session_t::iterator iter = _session_map_by_sid.find(sid);
			if (iter == _session_map_by_sid.end())
			{
				return false;
			}
			_session_map_by_addr.erase(Sockaddr2Unid(&(iter->second->_addr)));
			_session_stack.PutData(iter->second);
			_session_map_by_sid.erase(sid);
			return true;
		}
	private:
		bool FindSockaddr(const unid_t sid, sockaddr & addr)
		{
			rw_lock_rguard l(_session_map_lock);
			map_sid_session_t::iterator iter = _session_map_by_sid.find(sid);
			if (iter == _session_map_by_sid.end())
			{
				return false;
			}
			memcpy((void *)&addr, (const void *)&(iter->second->_addr), sizeof(sockaddr));
			return true;
		}

		bool FindSid(const sockaddr * addr, unid_t & sid)
		{
			if (addr == NULL)
			{
				return false;
			}
			rw_lock_rguard l(_session_map_lock);
			map_addr_session_t::iterator iter = _session_map_by_addr.find(Sockaddr2Unid(addr));
			if (iter == _session_map_by_addr.end())
			{
				return false;
			}
			sid = iter->second->_sid;
			return true;
		}

		bool AddSession(const sockaddr * addr, unid_t & sid)
		{
			rw_lock_wguard l(_session_map_lock);
			map_addr_session_t::iterator iter = _session_map_by_addr.find(Sockaddr2Unid(addr));
			if (iter != _session_map_by_addr.end())
			{
				sid = iter->second->_sid;
				return false;
			}

			udp_session * session = _session_stack.GetData();
			session->_sid = GetUniqueID(UIT_UDP_SID);
			memcpy((void *)&(session->_addr), (const void *)addr, sizeof(sockaddr));
			_session_map_by_sid.insert(std::make_pair(session->_sid, session));
			_session_map_by_addr.insert(std::make_pair(Sockaddr2Unid(addr), session));
			sid = session->_sid;
			return true;
		}

		void SendInl(send_buf * data)
		{
			sockaddr addr;
			if (!FindSockaddr(data->_sid, addr))
			{
				_send_buf_stack.PutData(data);
				return;
			}
            send_param * req = _send_param_stack.GetData();
            req->_send_buf = data;
            uv_buf_t msg = uv_buf_init((char*)data->_buf, data->_len);
            int err_code = uv_udp_send(req,
                &_server,
                &msg,
                1,
                (const sockaddr*) &addr,
                udp_server::SendCb);
            if (err_code != 0)
            {
                _err_info = GetErrorInfo(err_code);
            }
		}

	private:
		static void AsyncSend(uv_async_t * handle)
        {
            send_buf * data = (send_buf *)handle->data;
            data->_server->SendInl(data);
        }

        static void AsyncClose(uv_async_t * handle)
        {
            udp_server * server = (udp_server *)handle->data;
            int err_code = uv_udp_recv_stop(&(server->_server));
            if (err_code != 0)
            {
                server->_err_info = GetErrorInfo(err_code);
            }
            uv_close((uv_handle_t *)(&(server->_server)), udp_server::CloseCb);
        }
        
        static void AllocCb(uv_handle_t * handle, 
            size_t suggested_size, 
            uv_buf_t * buf)
        {
            uv_udp_t_with_server * uws = (uv_udp_t_with_server *)handle;
            recv_buf * data = uws->_server->_recv_buf_stack.GetData();
			memset((void *)data->_buf, 0, UDP_BUF_MAX_SIZE);
            buf->base = data->_buf;
            buf->len = UDP_BUF_MAX_SIZE;
		}

        static void RecvCb(uv_udp_t * handle,
            ssize_t nread,
            const uv_buf_t * rcvbuf,
            const sockaddr * addr,
            unsigned flags)
        {
			uv_udp_t_with_server * uws = (uv_udp_t_with_server *)handle;
			do
			{
				if (nread <= 0)
				{
					break;
				}
				unid_t sid = 0;
				if (!uws->_server->FindSid(addr, sid))
				{
					uws->_server->AddSession(addr, sid);
				}
				uws->_server->_handle->OnRecv(uws->_server, sid, rcvbuf->base, nread);
			} while (false);
			uws->_server->_recv_buf_stack.PutData((recv_buf *)rcvbuf->base);
		}

        static void SendCb(uv_udp_send_t* req, int status)
        {
            send_param * param = (send_param *)req;
            param->_send_buf->_server->_handle->OnSent(param->_send_buf->_server,
                param->_send_buf->_sid, param->_send_buf->_buf, param->_send_buf->_len);
            param->_send_buf->_server->_send_buf_stack.PutData(param->_send_buf);
            param->_send_buf->_server->_send_param_stack.PutData(param);
        }

		static void CloseCb(uv_handle_t * handle) 
		{
			uv_is_closing(handle);
		}

		static void Run(void * data)
		{
            udp_server * server = (udp_server *)data;
            int err_code = uv_run(&(server->_loop), UV_RUN_DEFAULT);
            if (err_code != 0)
            {
                server->_err_info = GetErrorInfo(err_code);
            }
		}
	private:
		uv_loop_t _loop;
        uv_async_t _async_send;
        uv_async_t _async_close;
		uv_thread_t * _thread;
		sockaddr_in _self_addr;
		
		map_sid_session_t _session_map_by_sid;
		map_addr_session_t _session_map_by_addr;
		rw_lock _session_map_lock;

		uv_udp_t_with_server _server;
		udp_server_config _config;
		udp_server_handle * _handle;
		std::string _err_info;

		data_stack<udp_session, 1000> _session_stack;
		data_stack<send_buf, 4> _send_buf_stack;
		data_stack<send_param, 4> _send_param_stack;
		data_stack<recv_buf, 4> _recv_buf_stack;
	};
#undef CHECK_ERR_CODE
}
