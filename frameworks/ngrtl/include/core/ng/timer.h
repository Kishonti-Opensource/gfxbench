/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_TIMER_INCLUDED
#define NG_TIMER_INCLUDED

#include "ng/ngrtl_core_export.h"

namespace ng
{
	struct NGRTL_EXPORT cpu_times
	{
		double wall, user, system;

		cpu_times()
			: wall(0)
			, user(0)
			, system(0)
		{}
		void clear()
		{
			wall = user = system = 0;
		}

		cpu_times& operator+=(const cpu_times& y)
		{
			wall += y.wall;
			user += y.user;
			system += y.system;
			return *this;
		}
	};



	//Returns wall, user, system times. The source of these clocks are platform-dependent.
	//If system/user time is not supported on your platform, then returns user = wall, system = 0
	class NGRTL_EXPORT cpu_timer
	{
	public:
		cpu_timer(bool bStart = true)
			: m_is_stopped(true)
		{
			if ( bStart )
				start();
		}
		bool is_stopped() const { return m_is_stopped; }
		cpu_times elapsed() const;  // does not stop()
		void start();
		void stop();
		void resume(); 
		void set_elapsed(const cpu_times& t);
	private:
		cpu_times m_times; //stores elapsed time if stopped, stores running-since time if not stopped
		bool m_is_stopped;
	};
}

#endif
