/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef FREQUENCY_COUNTER_H_
#define FREQUENCY_COUNTER_H_

#include <stdint.h>
#include "ng/timer.h"


namespace ng
{

class FrequencyCounter
{
public:
	FrequencyCounter();
	void setTimeInterval(double seconds);
	bool tick();
	double frequency() const { return frequency_; }
	
private:
	int32_t count_;
	double frequency_;
	double timeInterval_;
	cpu_timer timer_;
};
	
}

#endif  // FREQUENCY_COUNTER_H_