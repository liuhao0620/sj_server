#pragma once
#include <deque>
#include <string>
#include <uv.h>
#include "common_def.h"
#include "lock.h"

namespace sj
{
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
    class data_deque
    {
    public:
        data_deque()
        {
            _start_pointer = new DATATYPE[MAXSIZE];
            for (int i = 0; i < MAXSIZE; ++ i)
            {
                _dd.push_back(_start_pointer + i);
            }
        }
        ~data_deque()
        {
            _dd.clear();
            delete _start_pointer;
        }

        DATATYPE * GetData()
        {
            mutex_lock_guard l(_lock);
            if (_dd.empty())
            {
                return new DATATYPE;
            }
            DATATYPE * temp = _dd.front();
            _dd.pop_front();
            return temp;
        }

        void PutData(DATATYPE * data)
        {
            if (data - _start_pointer >= MAXSIZE)
            {
                delete data;
            }
            mutex_lock_guard l(_lock);
            _dd.push_back(data);
        }
    private:
        std::deque<DATATYPE *> _dd;
        DATATYPE * _start_pointer;
        mutex_lock _lock;
    };
}
