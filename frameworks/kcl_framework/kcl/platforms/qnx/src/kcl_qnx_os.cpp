/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//#include "kcl_os.h"
//#include "misc2.h"
//#include "zlib.h"
//#include "fbo.h"

#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <kcl_os.h>
#include <kcl_io.h>
KCL::OS *KCL::g_os = NULL;

namespace KCL
{
	struct OSImpl : public OS
	{
		OSImpl( const char *device_name);
		void Sleep (uint32 millisec);
		double GetTimeMilliSec();

		const char* GetDataDirectory();
		const char* GetDataRWDirectory();
		const char* GetDeviceName();

		void ResetInactivity();
		void Log( const char *a, ...);

		double GetBatteryLevel()
		{
			return -1.0;
		}
		int GetNumOfCPUCores() const;
		bool IsBatteryCharging()
		{
			return false;
		}
		char m_device_name[512];
	};

	int OSImpl::GetNumOfCPUCores() const
	{
	  return 1;//TODO
	}

	OSImpl::OSImpl( const char *device_name)
	{
		strcpy( m_device_name, device_name);
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
		usleep(millisec * 1000);
	}

	double OSImpl::GetTimeMilliSec()
	{
		struct timeval nowtime;
		double s;
		double u;

		gettimeofday( &nowtime, NULL );

		s = nowtime.tv_sec;
		u = nowtime.tv_usec;

		return ( s*1000 + u/1000 );
	}

	const char* OSImpl::GetDataDirectory()
	{
		return KCL::AssetFile::GetDataPath().c_str();
	}

	const char* OSImpl::GetDataRWDirectory()
	{
		return KCL::AssetFile::GetDataRWPath().c_str();
	}

	void OSImpl::Log( const char *fmt, ...)
	{
		va_list args;

		va_start (args, fmt);
		int len = vsnprintf( 0, 0, fmt, args);
		va_end (args);

		char *buffer = new char[len + 1];
		va_start (args, fmt);
		vsprintf (buffer, fmt, args);
		std::string msg = buffer;
		msg = "[INFO ]: " + msg + "\n";
		fprintf (stderr, "%s", msg.c_str());
		va_end (args);
		delete [] buffer;
	}

	OS* OS::CreateI( const char *device_name)
	{
		OSImpl *osi = new OSImpl( device_name);

		return osi;
	}

	void OS::DestroyI( OS** os)
	{
		delete *os;
		*os = 0;
	}
}

