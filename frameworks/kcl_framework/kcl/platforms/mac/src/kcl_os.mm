/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#import <Foundation/Foundation.h>
#import <IOKit/ps/IOPowerSources.h>
#import <IOKit/ps/IOPSKeys.h>

#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "zlib.h"

#include <kcl_os.h>
#include <kcl_io.h>

#include <iostream>

KCL::OS *KCL::g_os = NULL;

namespace KCL
{
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
    
    static BatteryState GetBatteryState();
    
	struct OSImpl : public OS
	{
		OSImpl( const char *device_name);
		void Sleep (uint32 millisec);
		double GetTimeMilliSec();
        
		const char* GetDataDirectory();
		const char* GetDataRWDirectory();
		const char* GetDeviceName();
        
        double GetBatteryLevel();
        bool IsBatteryCharging();
		
        void ResetInactivity();
		void Log( const char *a, ...);

		char m_device_name[512];
	};

	
	OSImpl::OSImpl( const char *device_name)
	{
		strcpy( m_device_name, device_name);
		
		NSString *sandbox = [[NSBundle mainBundle] bundlePath];
		NSString *datarpath = [NSString stringWithFormat: @"%@/Contents/Resources/data/", sandbox];
		
		
		//RW path
		NSArray *documentPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString *documentsDir = [[documentPaths objectAtIndex:0] stringByAppendingString:@"/"];
		
		KCL::File::SetDataPath([datarpath UTF8String]);
		KCL::File::SetRWDataPath([documentsDir UTF8String]);
		
	
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
        printf("Log: ");
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
            printf("%s",  temp);
        }
        
        int m2 = len - m * SIZE;
        memset(temp, 0, SIZE+1);
        memcpy(temp, &buffer[m*SIZE], m2);
        printf("%s",  temp);
        
        va_end (args);
        delete [] buffer;
        printf("\n");
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
    
    double OSImpl::GetBatteryLevel()
    {
        BatteryState state = GetBatteryState();
        return state.capacity/state.maxCapacity;
    }
    
    bool OSImpl::IsBatteryCharging()
    {
        BatteryState state = GetBatteryState();
        return state.isCharging || state.isConnected;
    }
    
    static BatteryState GetBatteryState()
    {
       
        CFTypeRef info;
        CFArrayRef list;
        CFDictionaryRef battery;
        BatteryState state;
        
        info = IOPSCopyPowerSourcesInfo();
        if(info == NULL)
            return state;
        list = IOPSCopyPowerSourcesList(info);
        if(list == NULL) {
            CFRelease(info);
            return state;
        }
        
        
        if(CFArrayGetCount(list) && (battery = IOPSGetPowerSourceDescription(info, CFArrayGetValueAtIndex(list, 0))))
        {
            state.isInstalled = [[(NSDictionary*)battery objectForKey:@kIOPSIsPresentKey] boolValue];
            state.isConnected = [(NSString*)[(NSDictionary*)battery objectForKey:@kIOPSPowerSourceStateKey] isEqualToString:@kIOPSACPowerValue];
            state.isCharging = [[(NSDictionary*)battery objectForKey:@kIOPSIsChargingKey] boolValue];
            state.capacity = [[(NSDictionary*)battery objectForKey:@kIOPSCurrentCapacityKey] doubleValue];
            state.maxCapacity = [[(NSDictionary*)battery objectForKey:@kIOPSMaxCapacityKey] doubleValue];
        }

        CFRelease(list);
        CFRelease(info);
        
        return state;
    }
}
