#pragma once
#include <string.h>
#include "net_base.h"

namespace sj
{
    class udp_client;
    class udp_client_handle
    {
    public:
        virtual void OnRecv(udp_client * client, char * buf, size_t len) = 0;
        virtual void OnSent(udp_client * client, char * buf, size_t len) = 0;
    };

    struct udp_client_config
    {
        std::string _server_ip;     // 连接的服务器IP地址
        int _server_port;           // 连接的服务器端口
        int _port;                  // 本机发送和接收端口
        std::string _name;          // 客户端名称
    };

#define CHECK_ERR_CODE \
    if (err_code != 0) \
    { \
        _err_info = GetErrorInfo(err_code); \
        return err_code; \
    }    

    class udp_client
    {
    private:
        struct uv_udp_t_with_client : public uv_udp_t
        {
            udp_client * _client;
        };

        struct recv_buf
        {
            char _buf[UDP_BUF_MAX_SIZE];
        };

        struct send_buf
        {
            udp_client * _client;
            size_t _len;
            char _buf[UDP_BUF_MAX_SIZE];
        };

        struct send_param : public uv_udp_send_t
        {
            send_buf * _send_buf;
        };
    public:
        udp_client()
        {
            _client._client = this;
            _handle = NULL;
            uv_loop_init(&_loop);
        }

        ~udp_client()
        {
            uv_loop_close(&_loop);
        }

    public:
        bool Init(udp_client_config& cfg)
        {
            _config._server_ip = cfg._server_ip;
            _config._server_port = cfg._server_port;
            _config._port = cfg._port;
            _config._name = cfg._name;
            return true;
        }

        void SetHandle(udp_client_handle* uch)
        {
            _handle = uch;
        }

        int StartUp()
        {
            if (_handle == NULL)
            {
                _err_info = "no hanle";
                return -1;
            }
            int err_code = uv_async_init(&_loop, &_async_send, udp_client::AsyncSend);
            CHECK_ERR_CODE
            err_code = uv_async_init(&_loop, &_async_close, udp_client::AsyncClose);
            CHECK_ERR_CODE
            err_code = uv_ip4_addr(_config._server_ip.c_str(), _config._server_port, &_server_addr);
            CHECK_ERR_CODE
            err_code = uv_ip4_addr(_config._server_ip.c_str(), _config._port, &_self_addr);
            CHECK_ERR_CODE
            err_code = uv_udp_init(&_loop, &_client);
            CHECK_ERR_CODE
            err_code = uv_udp_bind(&_client, (const sockaddr *)&_self_addr, 0);
            CHECK_ERR_CODE
            err_code = uv_udp_recv_start(&_client, udp_client::AllocCb, udp_client::RecvCb);
            CHECK_ERR_CODE   
            err_code = uv_thread_create(&_thread, udp_client::Run, (void *)this);
            CHECK_ERR_CODE
            return 0;
        }

        int Send(const char * buf, size_t len)
        {
            if (len > UDP_BUF_MAX_SIZE)
            {
                _err_info = "send buf is too long";
                return -1;
            }
            send_buf * data = _send_buf_stack.GetData();
            data->_client = this;
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

    private:
        void SendInl(send_buf * data)
        {
            send_param * req = _send_param_stack.GetData();
            req->_send_buf = data;
            uv_buf_t msg = uv_buf_init((char*)data->_buf, data->_len);
            int err_code = uv_udp_send(req,
                &_client,
                &msg,
                1,
                (const sockaddr*) &_server_addr,
                udp_client::SendCb);
            if (err_code != 0)
            {
                _err_info = GetErrorInfo(err_code);
            }
        }

    private:
        static void AsyncSend(uv_async_t * handle)
        {
            send_buf * data = (send_buf *)handle->data;
            data->_client->SendInl(data);
        }

        static void AsyncClose(uv_async_t * handle)
        {
            udp_client * client = (udp_client *)handle->data;
            int err_code = uv_udp_recv_stop(&(client->_client));
            if (err_code != 0)
            {
                client->_err_info = GetErrorInfo(err_code);
            }
            uv_close((uv_handle_t *)(&(client->_client)), udp_client::CloseCb);
        }
        
        static void AllocCb(uv_handle_t * handle, 
            size_t suggested_size, 
            uv_buf_t * buf)
        {
            uv_udp_t_with_client * uwc = (uv_udp_t_with_client *)handle;
            recv_buf * data = uwc->_client->_recv_buf_stack.GetData();
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
            uv_udp_t_with_client * uwc = (uv_udp_t_with_client *)handle;
            do
            {
                if (nread <= 0)
                {
                    break;
                }
                uwc->_client->_handle->OnRecv(uwc->_client, rcvbuf->base, nread);
            } while(false);
            uwc->_client->_recv_buf_stack.PutData((recv_buf *)rcvbuf->base);
        }

        static void SendCb(uv_udp_send_t* req, int status)
        {
            send_param * param = (send_param *)req;
            param->_send_buf->_client->_handle->OnSent(param->_send_buf->_client,
                param->_send_buf->_buf, param->_send_buf->_len);
            param->_send_buf->_client->_send_buf_stack.PutData(param->_send_buf);
            param->_send_buf->_client->_send_param_stack.PutData(param);
        }

		static void CloseCb(uv_handle_t * handle) 
		{
			uv_is_closing(handle);
		}

		static void Run(void * data)
		{
            udp_client * client = (udp_client *)data;
            int err_code = uv_run(&(client->_loop), UV_RUN_DEFAULT);
            if (err_code != 0)
            {
                client->_err_info = GetErrorInfo(err_code);
            }
		}

    private:
        uv_loop_t _loop;
        uv_async_t _async_send;
        uv_async_t _async_close;
        uv_thread_t _thread;
        sockaddr_in _server_addr;
        sockaddr_in _self_addr;

        uv_udp_t_with_client _client;
        udp_client_config _config;
        udp_client_handle * _handle; 
        std::string _err_info;
        
        data_stack<send_buf, 4> _send_buf_stack;
        data_stack<send_param, 4> _send_param_stack;
        data_stack<recv_buf, 4> _recv_buf_stack;			
    };
#undef CHECK_ERR_CODE
}
