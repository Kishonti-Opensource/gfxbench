/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_os.h"

namespace KCL
{
	enum GPUType {
		GPUType_Unknown = -1,
		Mali400 = 0,
		PVR,
		Adreno,
		Vivante,
		Tegra,
		MaliT6xx,
		GPUType_MAX
	};

	struct OSImpl : public OS
	{
		virtual int GetNumOfCPUCores() const;

		virtual int GetCurrentCPUFrequency(int index) const;
		virtual char GetCurrentCPULoad(int index) const;
		virtual int GetMinCPUFrequency(int index) const;
		virtual int GetMaxCPUFrequency(int index) const;

		virtual int GetCurrentGPUFrequency() const;

		void* LoadModuleLibrary(const char* libraryName);
		void ReleaseLibrary(void* module);

		OSImpl(const char *device_name);
		void Sleep (uint32 millisec);
		double GetTimeMilliSec();

		const char* GetDeviceName();
		void ResetInactivity();
		void Log( const char *a, ...);

		const char* GetSoftwareVersionString();
		void SetSoftwareVersionString(const char* newvalue);

		float GetCoreTemperature();
		double GetBatteryLevel();

		double GetDisplayBrightness();
		void SetDisplayBrightness(double newvalue);

		bool IsBatteryCharging();

		uint32 GetFreeMemoryMBytes();
		KCL_Status LoadingCallback( float p);

		unsigned int m_globalFBO;
		char m_device_name[2048];

		char m_software_version[256];
		bool m_battery_charging;
		double m_display_brightness;
		double m_battery_level;
		int m_low_memory_flag;
		//		std::map<std::string, void*> m_loaded_libraries;

	private:
		static int readInt(const char* fileName);
		static int parseMali();
		GPUType GetGPUType() const;
		GPUType m_gputype;	

	};

}
