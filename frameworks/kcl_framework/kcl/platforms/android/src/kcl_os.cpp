/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_os.h"
#include "kcl_io.h"
#include "zlib.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <unistd.h>
#include <dirent.h>

#include "kcl_osimpl_android.h"
#include <android/asset_manager.h>

#include "platform/platform_android.h"

using namespace KCL;

#define SLEEP_MICROTIMER
//#define NV_TIMER


KCL::OS *KCL::g_os = 0;

namespace {

	long GetFreeMemory()
	{
		static const char* const sums[] = { "MemFree:", "Cached:", NULL };
		static const size_t sumsLen[] = { strlen("MemFree:"), strlen("Cached:"), 0 };
		FILE *fd = fopen("/proc/meminfo", "r");

		if (fd == 0)
		{
			INFO("Unable to open /proc/meminfo");
			return 0;
		}

		char buffer[256] = {0};
		const int len = fread(buffer, sizeof(buffer)-1, 1, fd);
		fclose(fd);

		if (len < 0) {
			INFO("Unable to read /proc/meminfo");
			return 0;
		}
		buffer[len] = 0;
		int numFound = 0;
		long mem = 0;

		char* p = buffer;
		while (*p && numFound < 2) {
			int i = 0;
			while (sums[i]) {
				if (strncmp(p, sums[i], sumsLen[i]) == 0) {
					p += sumsLen[i];
					while (*p == ' ') p++;
					char* num = p;
					while (*p >= '0' && *p <= '9') p++;
					if (*p != 0) {
						*p = 0;
						p++;
						if (*p == 0) p--;
					}
					mem += atoll(num) * 1024;
					numFound++;
					break;
				}
				i++;
			}
			p++;
		}
		INFO("free mem %u MB", mem / (1024 * 1024));
		return numFound > 0 ? mem / (1024 * 1024) : 1;
	}

}

namespace KCL
{

	void* OSImpl::LoadModuleLibrary(const char* libraryName)
	{
		assert(0);
		return 0;
	}


	void OSImpl::ReleaseLibrary(void* module)
	{
		assert(0);
	}


	const char* OSImpl::GetSoftwareVersionString()
	{
		return m_software_version;
	}


	void OSImpl::SetSoftwareVersionString(const char* newvalue)
	{
		strncpy(m_software_version, newvalue, sizeof(m_software_version)-1);
		m_software_version[sizeof(m_software_version)-1] = 0;
	}


	KCL_Status OSImpl::LoadingCallback( float p)
	{
		if (cancelled) return KCL_TESTERROR_CANCELLED;
		if (m_low_memory_flag) return KCL_TESTERROR_OUT_OF_MEMORY;
		return KCL_TESTERROR_NOERROR;
	}


	uint32 OSImpl::GetFreeMemoryMBytes()
	{
		return GetFreeMemory();
	}


	bool android_battery_charging;
	double android_battery_level;

	OSImpl::OSImpl(const char *device_name)
	{
		memset(m_device_name, 0, 2048);
		memset(m_software_version, 0, sizeof(m_software_version));

		strcpy( m_device_name, device_name);
		m_low_memory_flag = 0;
		m_battery_charging = false;
		m_battery_level = 1;
		m_gputype=GetGPUType();
	}

	const char* OSImpl::GetDeviceName()
	{
		return m_device_name;
	}


	void OSImpl::ResetInactivity()
	{
	}


	void OSImpl::Sleep (uint32 millisec)
	{
#ifdef SLEEP_MICROTIMER
		usleep(millisec * 1000);
#else
		struct timespec timeLeft;
		timeLeft.tv_sec = 0;
		timeLeft.tv_nsec = millisec * 1000000;
		do
		{

		} while ( nanosleep(&timeLeft, &timeLeft) == -1);
#endif
	}


	double OSImpl::GetTimeMilliSec()
	{
#ifdef NV_TIMER
		if(eglGetSystemTimeNV)
		{
			khronos_uint64_t egltime;
			khronos_uint64_t egltimequot;
			khronos_uint64_t egltimerem;

			egltime = eglGetSystemTimeNV();

			egltimequot = egltime / eglGetSystemTimeFrequencyNV();
			egltimerem = egltime - (eglGetSystemTimeFrequencyNV() * egltimequot);
			egltimequot *= 1000;
			egltimerem *= 1000;
			egltimerem /= eglGetSystemTimeFrequencyNV();
			egltimequot += egltimerem;
			return (long) egltimequot;
		}
#else
		struct timespec time_now;
		clock_gettime(CLOCK_MONOTONIC, &time_now);
		return (time_now.tv_sec * 1000LL) + time_now.tv_nsec / 1000000LL;
#endif
	}

	void OSImpl::Log( const char *fmt, ...)
	{
#ifndef DISTRIBUTION
		va_list args;

		va_start (args, fmt);
		int len = vsnprintf( 0, 0, fmt, args);
		va_end (args);

		char *buffer = new char[len + 1];
		va_start (args, fmt);
		vsprintf (buffer, fmt, args);
		const int SIZE = 4000;

		char temp[SIZE+1];
		memset(temp, 0, SIZE+1);
		int m = len / SIZE;
		for(int i = 0; i < m;i++)
		{
			memcpy(temp, &buffer[i*SIZE], SIZE);
			__android_log_print(ANDROID_LOG_INFO, "GLBNativeLog", "%s",  temp);
		}

		int m2 = len - m * SIZE;
		memset(temp, 0, SIZE+1);
		memcpy(temp, &buffer[m*SIZE], m2);
		__android_log_print(ANDROID_LOG_INFO, "GLBNativeLog", "%s",  temp);

		va_end (args);
		delete [] buffer;
#endif
	}


	OS* OS::CreateI (const char *device_name)
	{
		OSImpl *osi = new OSImpl( device_name);

		return osi;
	}

	void OS::DestroyI( OS** os)
	{
		delete *os;
		*os = 0;
	}

	double OSImpl::GetDisplayBrightness()
	{
		return m_display_brightness;
	}


	void OSImpl::SetDisplayBrightness(double newvalue)
	{
		m_display_brightness = newvalue;
	}

	float OSImpl::GetCoreTemperature()
	{
		return kishonti::android::Platform::batteryTemperature();
	}

	double OSImpl::GetBatteryLevel()
	{
		return kishonti::android::Platform::batteryLevel();
	}


	bool OSImpl::IsBatteryCharging()
	{
		return kishonti::android::Platform::batteryCharging();
	}


	int OSImpl::GetNumOfCPUCores() const
	{
		int maxIndex = readInt("/sys/devices/system/cpu/kernel_max");
		return (maxIndex >= 0) ? (maxIndex + 1) : -1;
	}

	int OSImpl::GetCurrentCPUFrequency(int index) const
	{
		char name[64];
		sprintf(name, "/sys/devices/system/cpu/cpu%i/cpufreq/scaling_cur_freq", index);
		return readInt(name);
	}

	char OSImpl::GetCurrentCPULoad(int index) const
	{
		char name[64];
		sprintf(name, "/sys/devices/system/cpu/cpu%i/cpufreq/cpu_load", index);
		return (char)readInt(name);
	}

	int OSImpl::GetMinCPUFrequency(int index) const
	{
		char name[64];
		sprintf(name, "/sys/devices/system/cpu/cpu%i/cpufreq/cpuinfo_min_freq", index);
		return readInt(name);
	}

	int OSImpl::GetMaxCPUFrequency(int index) const
	{
		char name[64];
		sprintf(name, "/sys/devices/system/cpu/cpu%i/cpufreq/cpuinfo_max_freq", index);
		return readInt(name);
	}

	const char* const GPUModules[] =
	{
		"/sys/module/mali/parameters/mali_gpu_clk",
		"/sys/module/pvrsrvkm/parameters/sgx_gpu_clk",
		"/sys/class/kgsl/kgsl-3d0/gpuclk",
		"/sys/module/galcore/parameters/gpuClock",
		"/sys/kernel/tegra_gpu/gpu_rate",
		"/sys/class/misc/mali0/device/clock",
	};

	GPUType OSImpl::GetGPUType() const
	{
		for (int i = 0; i< GPUType_MAX; ++i)
		{
			FILE* f = fopen(GPUModules[i], "r");
			if (f)
			{
				fclose(f);
				return (GPUType)i;
			}
		}
		return GPUType_Unknown;
	}

	int OSImpl::GetCurrentGPUFrequency() const
	{
		switch (m_gputype)
		{
			case Mali400:
			case PVR:
				return readInt(GPUModules[m_gputype]);
			case Adreno:
			case Vivante:
			case Tegra:
				return readInt(GPUModules[m_gputype])/1000000;
			case MaliT6xx:
				return parseMali();
			default:
				return -1;
		}
	}

	int OSImpl::parseMali()
	{
		FILE* f = fopen(GPUModules[MaliT6xx], "r");
		if (!f)
		{
			return -1;
		}
		char chars[255];
		fgets(chars,255,f);
		int mhz = -1;

		sscanf(chars,"Current sclk_g3d[G3D_BLK] = %dMhz",&mhz);

		if (mhz>=0)
		{
			fclose(f);
			return mhz;
		}

		sscanf(chars,"%d",&mhz);

		if (mhz>=0)
		{
			fclose(f);
			return mhz;
		}
		fclose(f);
		return -1;
	}

	int OSImpl::readInt(const char* fileName)
	{
		FILE* f = fopen(fileName, "r");
		if (!f)
		{
			return -1;
		}

		int val;
		int result = fscanf(f, "%d", &val);
		if (result != 1)
		{
			val = -1;
		}

		fclose(f);
		return val;
	}

}
