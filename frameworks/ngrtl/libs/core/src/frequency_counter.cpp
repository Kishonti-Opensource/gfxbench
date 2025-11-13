/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/frequency_counter.h"
#include "ng/require.h"


namespace ng
{

	FrequencyCounter::FrequencyCounter()
		: count_(0)
		, frequency_(0)
		, timeInterval_(1.0)
	{
	}
	
	bool FrequencyCounter::tick()
	{
		++count_;
		double dt = timer_.elapsed().wall;
		if (dt > timeInterval_)
		{
			timer_ = cpu_timer();
			frequency_ = count_ / dt;
			count_ = 0;
			return true;
		}
		return false;
	}
	
	void FrequencyCounter::setTimeInterval(double seconds)
	{
		require(seconds > 0);
		timeInterval_ = seconds;
	}
}
