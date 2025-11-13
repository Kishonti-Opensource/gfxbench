/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/hr_clock.h"
#include "ng/safecast.h"

#if (defined __cplusplus && __cplusplus >= 201103) || _MSC_VER >= 1800
#  define USE_STD_CHRONO
#  include <chrono>
#elif defined WIN32 || defined UNDER_CE
#    define USE_WINDOWS_API
#    include <windows.h>
#elif defined __APPLE__
#  include <mach/mach.h>
#  include <mach/mach_time.h>
#  define USE_MACH
#elif defined __linux__
#  define USE_MONOTONIC_CLOCK
#  include <time.h>
#else
#  define USE_TIME
#  include <time.h>
#endif

namespace ng {


hr_time_point::
	hr_time_point()
	: t(0)
{
}

double hr_time_point::
	seconds_since_epoch() const
{
	return (double)t / hr_clock::clocks_per_sec();
}

hr_duration hr_time_point::
	operator-(const hr_time_point& y) const
{
	return hr_duration(t  - y.t);
}

hr_time_point hr_time_point::
	operator+(const hr_duration& y) const
{
	return hr_time_point(t + y.d);
}

hr_time_point hr_time_point::
	operator-(const hr_duration& y) const
{
	return hr_time_point(t  - y.d);
}

hr_time_point::
	hr_time_point(int64_t t)
	: t(t)
{}

hr_duration::
	hr_duration(int64_t d)
	: d(d)
{}

#define DEFINE_HR_REL(OP) \
	bool hr_time_point:: \
		operator OP (const hr_time_point& y) const \
	{ return t OP y.t; } \
	bool hr_duration:: \
		operator OP (const hr_duration& y) const \
	{ return d OP y.d; }

DEFINE_HR_REL(<)
DEFINE_HR_REL(<=)
DEFINE_HR_REL(>)
DEFINE_HR_REL(>=)
DEFINE_HR_REL(!=)
DEFINE_HR_REL(==)

#undef DEFINE_HR_REL

double hr_duration::
	seconds() const
{
	return d / hr_clock::clocks_per_sec();
}

hr_time_point hr_clock::
	now()
{
#ifdef USE_STD_CHRONO
	return hr_time_point(SAFE_CAST<int64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
#elif defined USE_WINDOWS_API
	LARGE_INTEGER pc;
	if(QueryPerformanceCounter(&pc))
		return hr_time_point(pc.QuadPart);
	else
		return hr_time_point();
#elif defined USE_TIME
	return hr_time_point(SAFE_CAST<int64_t>(time(NULL)));
#elif defined USE_MONOTONIC_CLOCK
	struct timespec times;
	int ret = clock_gettime(CLOCK_MONOTONIC, &times);
	if(ret != 0)
		return hr_time_point();
	const int64_t _1e9 = 1000000000LL;
	return hr_time_point(times.tv_sec * _1e9 + times.tv_nsec);
#elif defined USE_MACH
	return hr_time_point(mach_absolute_time());
#else
#error
#endif
}

double hr_clock::
	clocks_per_sec()
{
	if(s_clocks_per_sec == 0)
	{
#ifdef USE_STD_CHRONO
	auto min_duration = std::chrono::high_resolution_clock::duration::min();
	s_clocks_per_sec = 1.0 / std::chrono::duration<double>(min_duration).count();
#elif defined USE_WINDOWS_API
	LARGE_INTEGER freq;
	if(QueryPerformanceFrequency(&freq) != 0)
		s_clocks_per_sec = (double)freq.QuadPart;
#elif defined USE_TIME
	s_clocks_per_sec = 1;
#elif defined USE_MONOTONIC_CLOCK
	s_clocks_per_sec = 1000000000LL;
#elif defined USE_MACH
	mach_timebase_info_data_t tid;
	mach_timebase_info(&tid);
	s_clocks_per_sec = 1e9*(double)tid.denom/(double)tid.numer;
#else
#error
#endif
	}
	return s_clocks_per_sec;
}


double hr_clock::
	s_clocks_per_sec(0);

}