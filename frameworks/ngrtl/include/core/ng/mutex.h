/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_MUTEX_INCLUDED
#define NG_MUTEX_INCLUDED

#include "ng/ngrtl_core_export.h"
#include "ng/macros.h"

#if _MSC_VER >= 1700
#define NG_MUTEX_USE_STD
#endif

#if !defined(NG_MUTEX_USE_STD) && !defined(NG_MUTEX_USE_PTHREADS) && !defined(NG_MUTEX_USE_WINAPI)
	#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_MINGW)
		#define NG_MUTEX_USE_WINAPI
	#else
		#define NG_MUTEX_USE_PTHREADS
	#endif
#endif

#if defined(NG_MUTEX_USE_STD)
	#include <mutex>
	namespace ng
	{
		using std::mutex;
		using std::unique_lock;
	}
#else
	#include "ng/noncopyable.h"
	#include "ng/require.h"

	namespace ng
	{
		template<typename Mutex>
		class NGRTL_EXPORT unique_lock
			: private noncopyable
		{
		private:
			Mutex* m;
			bool is_locked;
		public:
			unique_lock()
				: m(0),is_locked(false)
			{}
			explicit unique_lock(Mutex& m_)
				: m(&m_),is_locked(false)
			{
				lock();
			}
			void swap(unique_lock& other)
			{
				std::swap(m,other.m);
				std::swap(is_locked,other.is_locked);
			}
			~unique_lock()
			{
				if(owns_lock())
				{
					m->unlock();
				}
			}
			void lock()
			{
				require(!owns_lock(), "unique_lock already owns lock");
				m->lock();
				is_locked=true;
			}
			bool try_lock()
			{
				require(!owns_lock(), "unique_lock already owns lock");
				is_locked=m->try_lock();
				return is_locked;
			}
			void unlock()
			{
				require(owns_lock(), "unique_lock doesn't own lock");
				m->unlock();
				is_locked=false;
			}
			bool operator!() const
			{
				return !owns_lock();
			}
			bool owns_lock() const
			{
				return is_locked;
			}
			Mutex* mutex() const
			{
				return m;
			}
			Mutex* release()
			{
				Mutex* const res=m;
				m=0;
				is_locked=false;
				return res;
			}
		};
	}
	#if defined(NG_MUTEX_USE_PTHREADS)
		#include <pthread.h>

		namespace ng
		{
			class NGRTL_EXPORT mutex
				: private ng::noncopyable
			{
			public:
				mutex();
				~mutex();

				void lock();
				bool try_lock();
				void unlock();

				typedef pthread_mutex_t* native_handle_type;
				native_handle_type native_handle();

				//typedef detail::try_lock_wrapper<mutex> scoped_try_lock;

			private:
				pthread_mutex_t m;
			};
		}
	#elif defined(NG_MUTEX_USE_WINAPI)
		#include <windows.h>
		namespace ng
		{
			class NGRTL_EXPORT mutex
				: private ng::noncopyable
			{
			public:
				mutex();
				~mutex();

				void lock();
				bool try_lock();
				void unlock();

				typedef HANDLE* native_handle_type;
				native_handle_type native_handle();

				//typedef detail::try_lock_wrapper<mutex> scoped_try_lock;

			private:
				HANDLE m;
			};
		}
	#endif
#endif

#endif
