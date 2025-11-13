/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "CPUQuery.h"
#include <stdio.h>
#include "ng/log.h"

#if defined (__linux__)
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#endif

CPUQuery::CPUQuery()
{
#ifdef WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	HANDLE process = GetCurrentProcess();
	const int SELECTED_CPU = 1;
	DWORD_PTR processAffinityMask = 1 << SELECTED_CPU;

	BOOL success = SetProcessAffinityMask(process, processAffinityMask);
	printf("Selecting CPU %d (success=%s)\n", SELECTED_CPU, success ? "true" : "false");


	for (int i = 0; i < NUM_OF_CPU_QUERIES; i++)
	{
		PdhOpenQuery(NULL, NULL, &cpuQuery[i]);
	}
	PDH_STATUS ret;
	ret = PdhAddCounter(cpuQuery[0], "\\Processor(_Total)\\% Processor Time", NULL, &cpuResults[0]);
	PdhAddCounter(cpuQuery[1], "\\Processor(0)\\% Processor Time", NULL, &cpuResults[1]);
	PdhAddCounter(cpuQuery[2], "\\Processor(1)\\% Processor Time", NULL, &cpuResults[2]);
	PdhAddCounter(cpuQuery[3], "\\Processor(2)\\% Processor Time", NULL, &cpuResults[3]);
	PdhAddCounter(cpuQuery[4], "\\Processor(3)\\% Processor Time", NULL, &cpuResults[4]);
	PdhAddCounter(cpuQuery[5], "\\Processor(4)\\% Processor Time", NULL, &cpuResults[5]);
	PdhAddCounter(cpuQuery[6], "\\Processor(5)\\% Processor Time", NULL, &cpuResults[6]);
	PdhAddCounter(cpuQuery[7], "\\Processor(6)\\% Processor Time", NULL, &cpuResults[7]);
	PdhAddCounter(cpuQuery[8], "\\Processor(7)\\% Processor Time", NULL, &cpuResults[8]);

	PdhAddCounter(cpuQuery[9], "\\Process(scene_editor)\\% Processor Time", NULL, &cpuResults[9]);
	PdhAddCounter(cpuQuery[10], "\\Process(testfw_app)\\% Processor Time", NULL, &cpuResults[10]);

	for (int i = 0; i < NUM_OF_CPU_QUERIES; i++)
	{
		ret = PdhCollectQueryData(cpuQuery[i]);
		if (ret != ERROR_SUCCESS)
		{
			PdhCloseQuery(cpuQuery[i]);
			cpuQuery[i] = 0;
		}
	}
#endif
#elif defined(__linux__)
	m_avg = 0.0;
	num_of_cpu_core = 0;
	fp = fopen("/proc/stat","r");
	for(int i = 0; i < NUM_OF_CPU_QUERIES;i++)
	{
		char cpuname[512];
		int ret = fscanf(fp,"%s %lf %lf %lf %lf %*d %*d %*d %*d %*d %*d", cpuname, &a[0][i],&a[1][i],&a[2][i],&a[3][i]);
		if (ret < 1)
		{
			NGLOG_INFO("couldn't read cpu");
		}
		if( strstr(cpuname, "cpu") == nullptr )
		{
			break;
		}
		++num_of_cpu_core;
		//printf("%s %lf %lf %lf %lf\n",cpuname, a[0][i], a[1][i], a[2][i], a[3][i]);
	}
	fclose(fp);
#endif//!WIN32
}


double CPUQuery::getCPUCurrentValue(int counter_id)
{
#ifdef WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	PDH_FMT_COUNTERVALUE counterVal;
	if (cpuQuery[counter_id])
	{
		PdhCollectQueryData(cpuQuery[counter_id]);
		PdhGetFormattedCounterValue(cpuResults[counter_id], PDH_FMT_DOUBLE, NULL, &counterVal);
	}
	else
	{
		counterVal.doubleValue = -1.0;
	}
	return counterVal.doubleValue;
#else
	return -1.0;
#endif
#elif defined (__linux__)
	return cpuResults[counter_id];
#else
	return -1.0;
#endif//!WIN32
}


void CPUQuery::Dump()
{
#if defined (__linux__)
	int len;
	char buffer[8192];
	len = sprintf(buffer, "CPU Load (total,c1,c2,c3,etc): ");
	fp = fopen("/proc/stat","r");
	for(int i = 0; i < num_of_cpu_core; i++)
	{
		int ret = fscanf(fp,"%*s %lf %lf %lf %lf  %*d %*d %*d %*d %*d %*d",&b[0][i],&b[1][i],&b[2][i],&b[3][i]);
		if (ret < 1)
		{
			NGLOG_INFO("couldn't read cpu");
		}
		loadavg = (((b[0][i]+b[1][i]+b[2][i]) - (a[0][i]+a[1][i]+a[2][i])) / ((b[0][i]+b[1][i]+b[2][i]+b[3][i]) - (a[0][i]+a[1][i]+a[2][i]+a[3][i]))) * 100;
		for(int h = 0;h < 4;h++)
		{
			a[h][i] = b[h][i];
		}
	
		cpuResults[i] = floor(loadavg);
		if(i==0)//HACK process's cpu equal with total cpu usage
		{
			cpuResults[0xA] = cpuResults[i];
		}
		len += sprintf(buffer + len, "%2.0lf ", cpuResults[i]);
	}
	m_avg += cpuResults[0];
	m_avg /= 2;
	NGLOG_INFO("avg %s", m_avg);
	NGLOG_INFO("%s", buffer);
	fclose(fp);

    return ;
#else
	for (int i = 0; i < 8; i++)
	{
		printf("%0.0f ", (*this)[i]);
	}
	printf("\n\n");
#endif
}


CPUQuery::~CPUQuery()
{
#ifdef WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	for (int i = 0; i < NUM_OF_CPU_QUERIES; i++)
	{
		if (cpuQuery[i] != 0)
		{
			PdhCloseQuery(cpuQuery[i]);
			cpuQuery[i] = 0;
		}
	}
#endif
#endif//!WIN32
}
