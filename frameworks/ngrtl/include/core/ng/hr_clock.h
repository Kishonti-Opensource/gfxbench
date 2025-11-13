/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INCLUDE_GUARD_hr_clock_1403618930
#define INCLUDE_GUARD_hr_clock_1403618930

#include <stdint.h>

namespace ng {

class hr_duration;
class hr_clock;

class hr_time_point
{
public:
	hr_time_point(); //initializes to zero
	double seconds_since_epoch() const;
	hr_duration operator-(const hr_time_point& y) const;
	hr_time_point operator+(const hr_duration& y) const;
	hr_time_point operator-(const hr_duration& y) const;
	bool operator<(const hr_time_point& y) const;
	bool operator<=(const hr_time_point& y) const;
	bool operator>(const hr_time_point& y) const;
	bool operator>=(const hr_time_point& y) const;
	bool operator==(const hr_time_point& y) const;
	bool operator!=(const hr_time_point& y) const;
private:
	friend class hr_duration;
	friend class hr_clock;

	int64_t t; //actual type is impl-dependent

	hr_time_point(int64_t t);
};

class hr_duration
{
public:
	hr_duration(); //initializes to zero
	double seconds() const;
	hr_duration operator+(const hr_duration& y) const;
	hr_duration operator-(const hr_duration& y) const;
	hr_time_point operator+(const hr_time_point& y) const;
	hr_duration operator-() const;
	bool operator<(const hr_duration& y) const;
	bool operator<=(const hr_duration& y) const;
	bool operator>(const hr_duration& y) const;
	bool operator>=(const hr_duration& y) const;
	bool operator==(const hr_duration& y) const;
	bool operator!=(const hr_duration& y) const;
private:
	friend class hr_time_point;
	int64_t d; //actual type is impl-dependent

	hr_duration(int64_t d);
};

class hr_clock
{
public:
	static hr_time_point now(); //return the tick period, in seconds
	static double clocks_per_sec();
private:
	static double s_clocks_per_sec;
};

}

#endif

