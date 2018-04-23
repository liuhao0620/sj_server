#pragma once
#include "common_def.h"
#include "lock.h"

namespace sj
{
	enum UNIQUE_ID_TYPE 
	{
		UIT_UDP_SID = 0,
		UIT_TCP_SID = 1,
		UIT_COUNT,
		UIT_MAX_COUNT = 0xFF,
	};

	unid_t GetUniqueID(UNIQUE_ID_TYPE uit)
	{
		static struct
		{
			unid_t _index = 0;
			mutex_lock _lock;
		}idxs[UIT_COUNT];
		mutex_lock_guard _l(idxs[uit]._lock);
		idxs[uit]._index ++;
		return (idxs[uit]._index << 8) | (uit & UIT_MAX_COUNT);
	}
}

