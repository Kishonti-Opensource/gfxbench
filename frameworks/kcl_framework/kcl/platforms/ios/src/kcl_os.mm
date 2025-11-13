/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#include "kcl_io.h"
#include "kcl_os.h"

#import <mach/mach.h>
#import <mach/mach_host.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#import <UIKit/UIKit.h>


using namespace std;
KCL::OS *KCL::g_os;

namespace KCL
{
    class OSIphone : public OS
    {
        public:
        OSIphone()
        {
            struct timeval tv;
            gettimeofday(&tv, 0);
            m_beginTime = tv.tv_sec*1000+tv.tv_usec;

            //R path
            NSString *sandbox = [[NSBundle mainBundle] bundlePath];
            NSString *datarpath = [NSString stringWithFormat: @"%@/data/", sandbox];


            //RW path
            NSArray *documentPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
            NSString *documentsDir = [[documentPaths objectAtIndex:0] stringByAppendingString:@"/"];

            //UIDeviceHardware *uidevhw = [[[UIDeviceHardware alloc] init] autorelease];

            File::SetDataPath([datarpath UTF8String]);
            File::SetRWDataPath([documentsDir UTF8String]);

            //strcpy( m_device_name, [[uidevhw platformString] UTF8String]);
        }

        void ResetInactivity()
		{
		}

        double GetTimeMilliSec()
		{
			struct timeval nowtime;
			double s;
			double u;

			gettimeofday( &nowtime, NULL );

			s = nowtime.tv_sec;
			u = nowtime.tv_usec;

			return ( s*1000 + u/1000 );
		}


        //void PreOpen(char *fn2, const char *fn);

        void Sleep (uint32 millisec)
		{
			usleep(millisec*1000);
		}

        const char * GetDeviceName()
		{
			return m_device_name;
		}

		void Log( const char *fmt, ...)
		{
			int ret;
			va_list args;

			va_start (args, fmt);
			int len = vsnprintf( 0, 0, fmt, args);
			va_end (args);

			char *buffer = new char[len + 1];

			va_start( args, fmt );
			vsprintf (buffer, fmt, args);
			ret = fprintf (stderr, "%s\n", buffer);

			va_end (args);
			delete [] buffer;
		}

		double GetBatteryLevel ()
		{
			return [[UIDevice currentDevice] batteryLevel];
		}

        double GetDisplayBrightness ()
		{
			return [[UIScreen mainScreen] brightness];
		}

        bool IsBatteryCharging()
		{
			bool b = [[UIDevice currentDevice] batteryState] != UIDeviceBatteryStateUnplugged;
			return b;
		}

		void SetDisplayBrightness(double newvalue)
		{
			[[UIScreen mainScreen] setBrightness:newvalue];
		}

        uint32 GetFreeMemoryMBytes()
		{
			mach_port_t           host_port = mach_host_self();
			mach_msg_type_number_t   host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
			vm_size_t               pagesize;
			vm_statistics_data_t     vm_stat;

			host_page_size(host_port, &pagesize);

			if (host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) != KERN_SUCCESS) NSLog(@"Failed to fetch vm statistics");

			//natural_t   mem_used = (vm_stat.active_count + vm_stat.inactive_count + vm_stat.wire_count) * pagesize;
			natural_t   mem_free = vm_stat.free_count * pagesize;
			//natural_t   mem_total = mem_used + mem_free;


			//printf(" free mem: %d\n", mem_free / 1024 / 1024);
			return mem_free / 1024 / 1024;

		}

        KCL_Status LoadingCallback( float p)
		{
			uint32 mem = GetFreeMemoryMBytes();
			if( mem < 8)
			{
				//return KCL_TESTERROR_OUT_OF_MEMORY;
			}
			if (cancelled) return KCL_TESTERROR_CANCELLED;
			return KCL_TESTERROR_NOERROR;
		}


        void* LoadModuleLibrary(const char* libraryName)
		{
			assert(0);
			return 0;
		}

        void ReleaseLibrary(void* module)
		{
		}

        uint32 m_beginTime;
        char m_datarpath[1024];
        char m_datarwpath[1024];
        char m_shaderpath[1024];
        char m_device_name[512];
    };

}


KCL::OS* KCL::OS::CreateI( const char *device_name)
{
    OSIphone* self = new OSIphone;

	return self;
}

void KCL::OS::DestroyI( OS** os)
{
    delete *os;
    *os = 0;
}


