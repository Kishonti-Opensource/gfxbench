/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <kcl_os.h>
#include <kcl_io.h>

#include <sys/time.h>
#include <ppapi/utility/completion_callback_factory.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>

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
		void LogFunc( int32_t res, char* str);

		int GetNumOfCPUCores() const;

		char m_device_name[512];
		pp::CompletionCallbackFactory<OSImpl> cbFactory;
	};

	int OSImpl::GetNumOfCPUCores() const
	{
	  return 1;
	}
	
	OSImpl::OSImpl( const char *device_name)
	{
		strcpy( m_device_name, device_name);
		cbFactory.Initialize(this);
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
		timespec t;
		memset(&t,0,sizeof(timespec));
		t.tv_sec=millisec/1000;
		t.tv_nsec=millisec%1000*1000;
		nanosleep(&t,NULL);
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
		char* buffer=new char[8192];
		int ret;
		va_list args;
		va_start (args, fmt);
		char fmtAlt[8192];
		strcpy(fmtAlt,"LOG: ");
		strcat(fmtAlt,fmt);
		vsprintf (buffer, fmtAlt, args);
		pp::Module::Get()->core()->CallOnMainThread(0,cbFactory.NewCallback(&OSImpl::LogFunc, buffer),0);
		va_end (args);
	}

	void OSImpl::LogFunc(int32_t res, char* str)
	{
		pp::Module::InstanceMap instances = pp::Module::Get()->current_instances();
		pp::Module::InstanceMap::iterator iter = instances.begin();
		if (iter!=instances.end())
		{
			iter->second->LogToConsole(PP_LOGLEVEL_LOG,pp::Var(str));
		}
		delete[] str;
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