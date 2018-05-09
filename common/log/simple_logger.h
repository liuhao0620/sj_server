#pragma once
#include <iostream>

namespace sj
{
    enum LOG_LEVEL
    {
        LL_DEBUG,
        LL_TRACE,
        LL_INFO,
        LL_ERROR,
        LL_FATAL,
    };

    class logger
    {
    public:
        static logger & GetInstance()
        {
            static logger _instance;
            return _instance;
        };

        LOG_LEVEL& GetLogLevel()
        {
            return _log_level;
        }

        void SetLogLevel(LOG_LEVEL ll)
        {
            _log_level = ll;
        } 

        template < class... ARGS >
        void __LogDebug(ARGS... args)
        {
            __Log(LL_DEBUG, std::cout, args...);
        }

        template < class... ARGS >
        void __LogTrace(ARGS... args)
        {
            __Log(LL_TRACE, std::cout, args...);
        }

        template < class... ARGS >
        void __LogInfo(ARGS... args)
        {
            __Log(LL_INFO, std::cout, args...);
        }

        template < class... ARGS >
        void __LogError(ARGS... args)
        {
            __Log(LL_ERROR, std::cout, args...);
        }

        template < class... ARGS >
        void __LogFatal(ARGS... args)
        {
            __Log(LL_FATAL, std::cout, args...);
        }

    private:
        template < class OS , class ARG >
        void __LogImp(OS& os, ARG arg)
        {
            os << arg << '\n';
        }

        template < class OS, class ARG, class... ARGS>
        void __LogImp(OS& os, ARG arg, ARGS... args)
        {
            os << arg;
            __LogImp(os, args...);
        }

        template < class OS, class... ARGS>
        void __Log(LOG_LEVEL ll, OS& os, ARGS... args)
        {
            if (ll < GetLogLevel())
            {
                return;
            }
            __LogImp(os, args...);
        }

    private:
        logger() : _log_level(LL_DEBUG) {}
        logger(const logger &);
        logger & operator= (const logger &);
    private:
        LOG_LEVEL _log_level;
    };

#define LOG_DEBUG sj::logger::GetInstance().__LogDebug
#define LOG_TRACE sj::logger::GetInstance().__LogTrace
#define LOG_INFO sj::logger::GetInstance().__LogInfo
#define LOG_ERROR sj::logger::GetInstance().__LogError
#define LOG_FATAL sj::logger::GetInstance().__LogFatal

}
