/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/timer.h"

#include <utility>

#include "ng/log.h"
#include "ng/macros.h"
#include "ng/require.h"
#include <algorithm>

#if defined _WIN32
#  if _MSC_VER >= 1700
#    define USE_STD_CHRONO
#  else
#    if !defined UNDER_CE
#      define USE_WINDOWS_API
#    endif
#    include <Windows.h>
#  endif
#elif __linux__
#  include <time.h>
#elif defined NACL
#  include <sys/time.h>
#elif __APPLE__
#  include <mach/mach.h>
#  include <mach/mach_time.h>
#else
#  include <sys/times.h>
#endif

#ifdef USE_STD_CHRONO
#  include <chrono>
#endif

namespace ng
{
	namespace
	{
		static const double c_nanosecToSec = 1e-9;
		void get_cpu_times(OUT cpu_times& ct)
		{
#ifdef USE_STD_CHRONO
			std::chrono::duration<double> tick = std::chrono::high_resolution_clock::now().time_since_epoch();
			ct.wall = tick.count();
			ct.user = ct.wall;
			ct.system = 0.0;
#elif defined USE_WINDOWS_API
			static bool is_clocks_per_sec_set(false);
			static double clocks_per_sec;
			if ( !is_clocks_per_sec_set )
			{
				LARGE_INTEGER freq;
				is_clocks_per_sec_set = QueryPerformanceFrequency(&freq) != 0;
				clocks_per_sec = is_clocks_per_sec_set ? (double)freq.QuadPart : 0.0;
			}
			LARGE_INTEGER pc;
			if ( QueryPerformanceCounter(&pc) )
			{
				ct.wall = (double)pc.QuadPart / clocks_per_sec;
			}
#if defined(_WINRT_DLL)
			// TODO: There is no equivalent API call to GetProcessTimes() on Windows Runtime. This implementation is the same as the __linux__ version.
			ct.user = ct.wall;
			ct.system = 0.0; // TODO: fix me
#else
			FILETIME kernel, user, d1, d2;
			if ( GetProcessTimes(GetCurrentProcess(), &d1, &d2, &kernel, &user) )
			{
				ULARGE_INTEGER kernel64, user64;
				kernel64.HighPart = kernel.dwHighDateTime;
				kernel64.LowPart = kernel.dwLowDateTime;
				user64.HighPart = user.dwHighDateTime;
				user64.LowPart = user.dwLowDateTime;
				const double c_filetime_clocks_per_sec = 1.0/100e-9; //100 ns
				ct.system = (double)kernel64.QuadPart / c_filetime_clocks_per_sec;
				ct.user = (double)user64.QuadPart / c_filetime_clocks_per_sec;
			}
#endif

#elif defined NACL
			ct.user = ct.wall = clock()/(double)(CLOCKS_PER_SEC);
			ct.system = 0;
#elif __linux__ || __QNX__
			// http://linux.die.net/man/3/clock_gettime
			struct timespec times;
			int ret = clock_gettime(CLOCK_MONOTONIC, &times);
			require(ret == 0);
			ct.user = ct.wall = times.tv_sec + c_nanosecToSec * times.tv_nsec;
			ct.system = 0; // TODO: fix me
#elif __APPLE__
			// https://developer.apple.com/library/mac/#qa/qa1398/_index.html
			uint64_t tick = mach_absolute_time();
			static mach_timebase_info_data_t sTimebaseInfo = {0};
			if ( sTimebaseInfo.denom == 0 ) {
				(void) mach_timebase_info(&sTimebaseInfo);
			}
			uint64_t timeNano = tick * sTimebaseInfo.numer / sTimebaseInfo.denom;
			ct.wall = ct.user = timeNano * c_nanosecToSec;
#else
			// http://linux.die.net/man/2/times
#if !defined UNDER_CE && !defined EMSCRIPTEN
			struct tms tms;
			static clock_t clkTck = 0;
			if (0 == clkTck) {
				clkTck = ::sysconf(_SC_CLK_TCK);
			}
			ct.wall = (double)::times(&tms) / (double)clkTck;
			ct.user = tms.tms_utime / (double) clkTck;
			ct.system = tms.tms_stime / (double) clkTck;
#else
			throw std::runtime_error(FORMATCSTR("get_cpu_times not implemented yet %s:%s", __FILE__, __LINE__));
#endif
#endif
		}

		void operator-=(INOUT cpu_times& x, const cpu_times& y)
		{
			x.system -= y.system;
			x.user -= y.user;
			x.wall -= y.wall;
		}

		void swap(cpu_times& x, cpu_times& y)
		{
			std::swap(x.system, y.system);
			std::swap(x.user, y.user);
			std::swap(x.wall, y.wall);
		}
	}

	void cpu_timer::
		start()
	{
		m_is_stopped = false;
		get_cpu_times(OUT m_times);
	}

	void cpu_timer::
		stop()
	{
		if ( !m_is_stopped )
		{
			cpu_times t;
			get_cpu_times(OUT t);
			m_is_stopped = true;
			t -= m_times;
			swap(t, m_times);
		}
	}

	void cpu_timer::
		resume()
	{
		if ( m_is_stopped )
		{
			cpu_times t;
			get_cpu_times(OUT t);
			t -= m_times;
			swap(t, m_times);
			m_is_stopped = false;
		}
	}

	void cpu_timer::
		set_elapsed(const cpu_times& t)
	{
		m_is_stopped = true;
		m_times = t;
	}

	cpu_times cpu_timer::
		elapsed() const
	{
		if ( m_is_stopped )
			return m_times;
		else
		{
			cpu_times t;
			get_cpu_times(OUT t);
			t -= m_times;
			return t;
		}
	}
}
