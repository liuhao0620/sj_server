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

    LOG_LEVEL& GetLogLevel()
    {
        static LOG_LEVEL _ll = LL_DEBUG;
        return _ll;
    }

    void SetLogLevel(LOG_LEVEL ll)
    {
        GetLogLevel() = ll;
    }

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

#define LOG_DEBUG sj::__LogDebug
#define LOG_TRACE sj::__LogTrace
#define LOG_INFO sj::__LogInfo
#define LOG_ERROR sj::__LogError
#define LOG_FATAL sj::__LogFatal

}
