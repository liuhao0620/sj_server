#pragma once
#include <string>
#include <uv.h>
#include "lock.h"
#include "common_def.h"

#define UDP_BUF_MAX_SIZE (1000)

namespace sj
{
    unid_t Sockaddr2Unid(const sockaddr * addr)
    {
        return *(unid_t *)addr;
    }

    std::string GetErrorInfo(int err_code)
    {
        if (err_code == 0)
        {
            return "";
        }
        std::string err_info = std::to_string(err_code);
        err_info += " : ";
        const char * temp_chars = uv_err_name(err_code);
        if (temp_chars != NULL)
        {
            err_info = temp_chars;
            err_info += " : ";
        }
        else
        {
            err_info = "unknown system errcode : ";
        }
        temp_chars = uv_strerror(err_code);
        if (temp_chars)
        {
            err_info += temp_chars;
        }
        return std::move(err_info);
    }

    template < class DATATYPE, int MAXSIZE >
    class data_stack
    {
    public:
        data_stack()
        {
            _start_pointer = new DATATYPE[MAXSIZE];
            _cur_pos = MAXSIZE - 1;
            for (int i = 0; i < MAXSIZE; ++ i)
            {
                _dd[i] = _start_pointer + i;
            }
        }
        ~data_stack()
        {
            delete _start_pointer;
        }

        DATATYPE * GetData()
        {
            mutex_lock_guard l(_lock);
            if (_cur_pos == 0)
            {
                return new DATATYPE;
            }
            return _dd[_cur_pos --];
        }

        void PutData(DATATYPE * data)
        {
            if (data < _start_pointer || data - _start_pointer >= MAXSIZE)
            {
                delete data;
                return;
            }
            mutex_lock_guard l(_lock);
            _dd[++ _cur_pos] = data;
        }
    private:
        DATATYPE * _dd[MAXSIZE];
        int _cur_pos;
        DATATYPE * _start_pointer;
        mutex_lock _lock;
    };
}
