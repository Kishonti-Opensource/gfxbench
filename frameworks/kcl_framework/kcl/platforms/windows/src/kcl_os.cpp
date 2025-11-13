/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_os.h"
#include "kcl_io.h"

#define NOMINMAX
#define NOCOMM

#include <windows.h>
#include <map>
#include <stdio.h>
#include <direct.h>
#include <string>
#include <cassert>

#include "winapifamilynull.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#include <shlwapi.h>
#include <psapi.h>
#include <BatClass.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>

#pragma comment (lib, "setupapi.lib")

#else

#include <chrono>
#include <thread>
#include <mmsystem.h>

#endif


using namespace KCL;

KCL::OS *KCL::g_os = NULL;


struct OSImpl : public OS
{
	int GetNumOfCPUCores() const;

	OSImpl( const char *device_name);
	void Sleep (KCL::uint32 millisec);
	double GetTimeMilliSec();

	const char* GetDataDirectory();
	const char* GetDataRWDirectory();
	const char* GetShaderDirectory();
	const char* GetDeviceName();
	void ResetInactivity();
	void Log( const char *a, ...);
	KCL::uint32 GetFreeMemoryMBytes();

	void* LoadModuleLibrary(const char* libraryName);
	void ReleaseLibrary(void* module);

    bool IsBatteryCharging();

	double GetDisplayBrightness()
	{
		return 1.0;
	}

	double GetBatteryLevel();

	void SetBatteryCharging(bool isCharging)
	{
		m_battery_charging = isCharging;
	}

	void SetBatteryLevel(double level)
	{
		m_battery_level = level;
	}

	std::map<std::string, HMODULE> m_loaded_libraries;

	LARGE_INTEGER m_freq;
	double m_freq_inv;
	char m_device_name[2048];
	bool m_battery_charging;
	double m_battery_level;
};



struct BatteryState
{
    double capacity;
    double maxCapacity;
    bool isInstalled;
    bool isConnected;
    bool isCharging;

    BatteryState() : capacity(1.0), maxCapacity(-1.0),
                    isInstalled(false), isConnected(false), isCharging(false) {};
};



static BatteryState GetBatteryState()
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#define GBS_HASBATTERY 0x1
#define GBS_ONBATTERY  0x2
	// Returned value includes GBS_HASBATTERY if the system has a
	// non-UPS battery, and GBS_ONBATTERY if the system is running on
	// a battery.
	//
	// dwResult & GBS_ONBATTERY means we have not yet found AC power.
	// dwResult & GBS_HASBATTERY means we have found a non-UPS battery.

	DWORD dwResult = GBS_ONBATTERY;
	DWORD capacity = -1;
	DWORD fullcapacity = -1;

	// IOCTL_BATTERY_QUERY_INFORMATION,
	// enumerate the batteries and ask each one for information.

	HDEVINFO hdev =	SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY, 0, 	0, 	DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (INVALID_HANDLE_VALUE != hdev)
	{
		// Limit search to 100 batteries max
		for (int idev = 0; idev < 100; idev++)
		{
			SP_DEVICE_INTERFACE_DATA did = {0};
			did.cbSize = sizeof(did);

			if (SetupDiEnumDeviceInterfaces(hdev,	0,	&GUID_DEVCLASS_BATTERY,	idev,	&did))
			{
				DWORD cbRequired = 0;

				SetupDiGetDeviceInterfaceDetail(hdev,&did,	0,	0,	&cbRequired,0);
				if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
				{
					PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd =
						(PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR,
						cbRequired);
					if (pdidd)
					{
						pdidd->cbSize = sizeof(*pdidd);
						if (SetupDiGetDeviceInterfaceDetail(hdev,
							&did,
							pdidd,
							cbRequired,
							&cbRequired,
							0))
						{
							// Enumerated a battery.  Ask it for information.
							HANDLE hBattery =
								CreateFile(pdidd->DevicePath,
								GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
							if (INVALID_HANDLE_VALUE != hBattery)
							{
								// Ask the battery for its tag.
								BATTERY_QUERY_INFORMATION bqi = {0};

								DWORD dwWait = 0;
								DWORD dwOut;

								if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_TAG,&dwWait,sizeof(dwWait),	&bqi.BatteryTag, sizeof(bqi.BatteryTag), &dwOut, NULL) && bqi.BatteryTag)
								{
									// With the tag, you can query the battery info.
									BATTERY_INFORMATION bi = {0};
									bqi.InformationLevel = BatteryInformation;

									if (DeviceIoControl(hBattery,
										IOCTL_BATTERY_QUERY_INFORMATION,
										&bqi,
										sizeof(bqi),
										&bi,
										sizeof(bi),
										&dwOut,
										NULL))
									{
										fullcapacity = bi.FullChargedCapacity;
										// Only non-UPS system batteries count
										if (bi.Capabilities & BATTERY_SYSTEM_BATTERY)
										{
											if (!(bi.Capabilities & BATTERY_IS_SHORT_TERM))
											{
												dwResult |= GBS_HASBATTERY;
											}

											// Query the battery status.
											BATTERY_WAIT_STATUS bws = {0};
											bws.BatteryTag = bqi.BatteryTag;

											BATTERY_STATUS bs;
											if (DeviceIoControl(hBattery,
												IOCTL_BATTERY_QUERY_STATUS,
												&bws,
												sizeof(bws),
												&bs,
												sizeof(bs),
												&dwOut,
												NULL))
											{
												if (bs.PowerState & BATTERY_POWER_ON_LINE)
												{
													dwResult &= ~GBS_ONBATTERY;
												}
											}
											capacity = bs.Capacity;
										}
									}
								}
								CloseHandle(hBattery);
							}
						}
						LocalFree(pdidd);
					}
				}
			}
			else  if (ERROR_NO_MORE_ITEMS == GetLastError())
			{
				break;  // Enumeration failed - perhaps we're out of items
			}
		}
		SetupDiDestroyDeviceInfoList(hdev);
	}

	//  Final cleanup:  If we didn't find a battery, then presume that we
	//  are on AC power.

	if (!(dwResult & GBS_HASBATTERY))
		dwResult &= ~GBS_ONBATTERY;

    BatteryState result;
    result.capacity = capacity;
    result.maxCapacity = fullcapacity;
    result.isCharging = !(dwResult & GBS_ONBATTERY);
    result.isConnected = false;
    result.isInstalled = false;
	return result;

#else
    return BatteryState();
#endif
}



bool OSImpl::IsBatteryCharging()
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    BatteryState batteryState = GetBatteryState();
    return batteryState.isCharging;
#else
	return m_battery_charging;
#endif
}



double OSImpl::GetBatteryLevel()
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    BatteryState batteryState = GetBatteryState();
    return batteryState.capacity / batteryState.maxCapacity;
#else
	return m_battery_level;
#endif
}


void* OSImpl::LoadModuleLibrary(const char* libraryName)
{
	/*
	m_loaded_libraries[libraryName] = LoadLibrary(libraryName);
	if(m_loaded_libraries[libraryName])
	{
		Module* (*module)(void) =  (Module*(*)(void)) GetProcAddress(m_loaded_libraries[libraryName], "createModule");
		return module();
	}
	*/
	return 0;
}


void OSImpl::ReleaseLibrary(void* module)
{
	/*
	std::map<std::string, HMODULE>::iterator it;

	for( it = m_loaded_libraries.begin(); it!=m_loaded_libraries.end();)
	{
	if(it->second)
	{
	void (*deleteModule)(Module*) =  (void(*)(Module*)) GetProcAddress(it->second, "deleteModule");
	deleteModule( (Module*)module);
	}

	FreeLibrary(it->second);
	m_loaded_libraries.erase(it);
	break;
	}
	*/
}


KCL::uint32 OSImpl::GetFreeMemoryMBytes()
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	/*
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	INFO("Used memory %d", pmc.WorkingSetSize);
	*/
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	long freeMemory = memInfo.ullAvailPhys / (1024 * 1024);
	INFO("Total free memory in MB: %d", freeMemory);
	return freeMemory;
#else
	return 0;
#endif
}


OSImpl::OSImpl(const char *device_name) : m_battery_charging(false), m_battery_level(0)
{
	QueryPerformanceFrequency( &m_freq);
	m_freq_inv = 1.0 /(double)m_freq.QuadPart;

	strcpy( m_device_name, device_name);
}

void OSImpl::ResetInactivity()
{
}


struct timespec
{
	time_t tv_sec;
	long int tv_nsec;
};


void nanosleep (const struct timespec *requested_delay)
{
	if (requested_delay->tv_sec > 0)
		/* At least one second. Millisecond resolution is sufficient. */
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		Sleep(requested_delay->tv_sec * 1000 + requested_delay->tv_nsec / 1000000);
#else
		std::this_thread::sleep_for(std::chrono::milliseconds(requested_delay->tv_sec * 1000 + requested_delay->tv_nsec / 1000000));
#endif
	else
	{
		/* Use Sleep for the largest part, and busy-loop for the rest. */
		static double frequency;
		if (frequency == 0)
		{
			LARGE_INTEGER freq;
			if (!QueryPerformanceFrequency (&freq))
			{
				/* Cannot use QueryPerformanceCounter. */
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
				Sleep(requested_delay->tv_nsec / 1000000);
#else
				std::this_thread::sleep_for(std::chrono::milliseconds(requested_delay->tv_nsec / 1000000));
#endif
				return;
			}
			frequency = (double) freq.QuadPart / 1000000000.0;
		}
		long long expected_counter_difference = requested_delay->tv_nsec * frequency;
		int sleep_part = (int) requested_delay->tv_nsec / 1000000 - 10;
		LARGE_INTEGER before;
		QueryPerformanceCounter (&before);
		long long expected_counter = before.QuadPart + expected_counter_difference;
		if (sleep_part > 0)
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
			Sleep(sleep_part);
#else
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_part));
#endif
		for (;;)
		{
			LARGE_INTEGER after;
			QueryPerformanceCounter (&after);
			if (after.QuadPart >= expected_counter)
				break;
		}
	}
}


void OSImpl::Sleep (uint32 millisec)
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	::Sleep(millisec);
#else
	std::this_thread::sleep_for(std::chrono::milliseconds(millisec));
#endif
	/*
	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = millisec * 1000000;
	nanosleep (&t);
	*/
}


double OSImpl::GetTimeMilliSec()
{
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);

	return ((double)time.QuadPart * m_freq_inv) * 1000.0;
}


const char* OSImpl::GetDataDirectory()
{
	return KCL::File::GetDataPath().c_str();
}


const char* OSImpl::GetDataRWDirectory()
{
	return KCL::File::GetDataRWPath().c_str();
}


const char* OSImpl::GetDeviceName()
{
	return m_device_name;
}


void OSImpl::Log( const char *fmt, ...)
{
#ifndef OPT_COMMUNITY_BUILD
	va_list args;

	va_start (args, fmt);
	int len = vsnprintf( 0, 0, fmt, args);
	va_end (args);

	char *buffer = new char[len + 1];
	va_start (args, fmt);
	vsprintf (buffer, fmt, args);
	std::string msg = buffer;
	msg = "Log: " + msg + "\n";
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	fprintf (stderr, "%s", msg.c_str());
#endif
	OutputDebugStringA(msg.c_str());

	va_end (args);
	delete [] buffer;
#endif
}


int OSImpl::GetNumOfCPUCores() const
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	return sysinfo.dwNumberOfProcessors;
#else
	return 0;
#endif
}


#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#pragma comment(lib, "Winmm.lib")
#endif

OS* OS::CreateI(const char *device_name )
{
	OSImpl *osi = new OSImpl( device_name);
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		timeBeginPeriod(1);
#endif
	return osi;
}

void OS::DestroyI( OS** os)
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	timeEndPeriod(1);
#endif
	delete *os;
	*os = 0;
}
