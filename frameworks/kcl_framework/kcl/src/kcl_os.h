/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_OS_H
#define KCL_OS_H

#include <kcl_base.h>

#define INFO(...) if(KCL::g_os) KCL::g_os->Log( __VA_ARGS__)

namespace KCL
{
	class OS
	{
	public:
		virtual int GetNumOfCPUCores() const { return 0; }

		virtual int GetCurrentCPUFrequency(int index) const	{ return 0; }
		virtual char GetCurrentCPULoad(int index) const { return 0; }
		virtual int GetMinCPUFrequency(int index) const	{ return 0; }
		virtual int GetMaxCPUFrequency(int index) const	{ return 0; }

		virtual int GetCurrentGPUFrequency() const { return -1; }

		virtual void Sleep (unsigned int millisec) = 0;
		virtual double GetTimeMilliSec() = 0;

		virtual const char* GetDeviceName() = 0;
		virtual void ResetInactivity() = 0;
		virtual void Log( const char *a, ...) = 0;

		static OS* CreateI( const char *device_name);
		static void DestroyI( OS** os);
		
		virtual void* LoadModuleLibrary(const char* libraryName){return 0;};
		virtual void ReleaseLibrary(void* module){};

		virtual float GetCoreTemperature() { return -1; }
		virtual double GetBatteryLevel () { return -1.0; }
		virtual double GetDisplayBrightness () { return -1.0; }
		
		virtual void SetDisplayBrightness(double newvalue)  {};
		virtual void SetBatteryLevel(double newvalue)  {};
		
		virtual void SetBatteryCharging(bool newvalue){};
		virtual bool IsBatteryCharging () { return true; }

		virtual const char* GetSoftwareVersionString() { return ""; }
		virtual void SetSoftwareVersionString(const char* newvalue){};
		
		virtual unsigned int GetFreeMemoryMBytes() 
		{ 
			return (unsigned int)~0;
		}

		void cancelTestLoading()
		{
			cancelled = true;
		}

		virtual KCL_Status LoadingCallback( float p)
		{
			if (cancelled) return KCL_TESTERROR_CANCELLED;
			return KCL_TESTERROR_NOERROR;
		}

	protected:
		OS()
		{
			cancelled=false;
		}

		virtual ~OS()
		{
		}

		bool cancelled;
	};

	extern OS *g_os;
}
#endif
