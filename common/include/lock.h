#pragma once
#include "uv.h"

namespace sj
{
    class rw_lock
    {
    public:
        rw_lock() { uv_rwlock_init(&_lock); }
        ~rw_lock() { uv_rwlock_destroy(&_lock); }

        void RLock() { uv_rwlock_rdlock(&_lock); }
        void RUnlock() { uv_rwlock_rdunlock(&_lock); }
        void WLock() { uv_rwlock_wrlock(&_lock); }
        void WUnlock() { uv_rwlock_wrunlock(&_lock); }

    private:
        uv_rwlock_t _lock;
    };

	class mutex_lock
	{
	public:
		mutex_lock() { uv_mutex_init(&_lock); }
		~mutex_lock() { uv_mutex_destroy(&_lock); }

		void Lock() { uv_mutex_lock(&_lock); }
		void Unlock() { uv_mutex_unlock(&_lock); }

	private:
		uv_mutex_t _lock;
	};

    class rw_lock_rguard
    {
    public:
        rw_lock_rguard(rw_lock& l) : _lock(l) { _lock.RLock(); }
        ~rw_lock_rguard() { _lock.RUnlock(); }
    private:
        rw_lock& _lock;
    };

    class rw_lock_wguard
    {
    public:
        rw_lock_wguard(rw_lock& l) : _lock(l) { _lock.WLock(); }
        ~rw_lock_wguard() { _lock.WUnlock(); }
    private:
        rw_lock& _lock;
    };

	class mutex_lock_guard
	{
	public:
		mutex_lock_guard(mutex_lock& l) : _lock(l) { _lock.Lock(); }
		~mutex_lock_guard() { _lock.Unlock(); }
	private:
		mutex_lock& _lock;
	};

}
