/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/mutex.h"

#if defined(NG_MUTEX_USE_STD)
	//nothing to do
#elif defined(NG_MUTEX_USE_PTHREADS)
	#include "ng/require.h"
	#include "ng/format.h"
	#include <errno.h>
	namespace ng
	{
		mutex::
			mutex()
		{
			int const res = pthread_mutex_init(&m, NULL);
			require(!res, FORMATSTR("pthread_mutex_init returned %s", res));
		}

		mutex::
			~mutex()
		{
			int ret;
			do
			{
				ret = pthread_mutex_destroy(&m);
			} while (ret == EINTR);
		}

		bool mutex::
			try_lock()
		{
			int res;
			do
			{
				res = pthread_mutex_trylock(&m);
			} while (res == EINTR);
			require(!res || (res==EBUSY), FORMATSTR("pthread_mutex_trylock returned %s", res));

			return !res;
		}
		void mutex::
            lock()
		{
			int res;
			do
			{
				res = pthread_mutex_lock(&m);
			} while (res == EINTR);
			require(!res, FORMATSTR("pthread_mutex_lock returned %s", res));
		}
		void mutex::
            unlock()
		{
			int ret;
			do
			{
				ret = pthread_mutex_unlock(&m);
			} while (ret == EINTR);
			assert(!ret);
		}
	}
#elif defined(NG_MUTEX_USE_WINAPI)
	#include <windows.h>
	#include "ng/require.h"
	#include "ng/format.h"
	namespace ng
	{
		mutex::
			mutex()
		{
			m = CreateMutex(NULL, FALSE, NULL);
			require(m != NULL, FORMATSTR("CreateMutex failed, error code: %s", GetLastError()));
		}

		mutex::
			~mutex()
		{
			CloseHandle(m);
		}

		bool waitForMutex(HANDLE m, DWORD time)
		{
			switch(WaitForSingleObject(m, time))
			{
			case WAIT_ABANDONED: //owner thread terminated without release
				require(false, FORMATSTR("Mutex owner thread terminated without releasing it."));
				return true;
			case WAIT_OBJECT_0: //ownership granted
				return true;
			case WAIT_TIMEOUT: //timeout expired
				return false;
			case WAIT_FAILED: //function failed
				if (time == INFINITE)
					require(false, FORMATSTR("Mutex locking error: %s", GetLastError()));
				return false;
			default:
				return false; //Without default path MSVC would give C4715 error.
			}
		}

		bool mutex::
			try_lock()
		{
			return waitForMutex(m, 0);
		}

		void mutex::
            lock()
		{
			waitForMutex(m, INFINITE);
		}

		void mutex::
            unlock()
		{
			BOOL ret = ReleaseMutex(m);
			assert(ret);
		}
	}
#endif
