/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CPUQUERY_H
#define CPUQUERY_H

#ifdef WIN32
#include <winapifamily.h>
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include <pdh.h>
#pragma comment(lib, "Pdh.lib")
#endif
#endif//!WIN32
#include <stdio.h>

#define NUM_OF_CPU_QUERIES 32

class CPUQuery
{
public:
	CPUQuery();
	~CPUQuery();

	/*!Query cpu usage between 0..100%
	   index 0 == total cpu usage
	   index 1..8 per cpu usage
	   index 9 process cpu usage
	*/
	double getCPUCurrentValue(int counter_id);

	/*!return per cpu usage
	*/
	double operator[](int index)
	{
		return getCPUCurrentValue(1 + index);
	}
	void Dump();

private:
#ifdef WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	PDH_HQUERY cpuQuery[NUM_OF_CPU_QUERIES];
	PDH_HCOUNTER cpuResults[NUM_OF_CPU_QUERIES];
#endif
#elif defined (__linux__)
	int num_of_cpu_core;
	double a[4][NUM_OF_CPU_QUERIES];
	double b[4][NUM_OF_CPU_QUERIES];
	double m_avg;
	double loadavg;
	double cpuResults[NUM_OF_CPU_QUERIES];
	FILE *fp;
	/*
	/proc/stat
	1    2    3      4    5
	user nice system idle iowait
	
	                             user_delta(t)  +  nice_delta(t)  +  system_delta (t)
Percent CPU Usage = ---------------------------------------------------------------------- X 100
                       user_delta(t) + nice_delta(t) + system_delta (t) + idle_delta(t)
	 */
#endif//!WIN32

};


#endif
