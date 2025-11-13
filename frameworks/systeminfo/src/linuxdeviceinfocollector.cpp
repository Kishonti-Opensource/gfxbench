/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "linuxdeviceinfocollector.h"

#include "Poco/Process.h"
#include "Poco/PipeStream.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <cmath>
#include <sstream>
#include <string.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <unistd.h>

using namespace sysinf;

bool checkFile(const char *filename)
{
	struct stat buff;
	return stat(filename, &buff) == 0;
}

/*
return it was success 
*/
bool getUname(const char* file, char *result, const int size = 256)
{

	FILE *f = popen(file, "r");
	if(f)
	{
		if(fgets(result, size, f))
			;
		pclose(f);
		return true;
	}
	return false;
}


void getStr(const char* file, char *result, const int size = 256)
{
	if(result)
	{
		sprintf(result, " ");
		if(file)
		{
			FILE *f = fopen(file, "r");
			if(f)
			{
				if(fgets(result, size, f))
					;
				fclose(f);
			}
		}
	}
}


DeviceInfo LinuxDeviceInfoCollector::collectDeviceInfo()
{
	DeviceInfo deviceInfo;

	char name[1024];
	char sys_vendor[256];
	char chipset[256];

	getStr("/sys/devices/virtual/dmi/id/sys_vendor", sys_vendor);
	getStr("/sys/devices/virtual/dmi/id/product_name", chipset);

	/*	
		check the characters in the chipset and sys_vendor because on some device
		probably two byte length
	*/
	bool valid = false;
	for( char s = 'a'; s != 'z';++s)
	{
		if(sys_vendor[0] == tolower(s))
		{
			valid = true;
			break;
		}
	}

	if (strstr(sys_vendor, "System manufacturer") != nullptr || valid == false)
	{
		getStr("/sys/devices/virtual/dmi/id/board_vendor", sys_vendor);
	}

	valid = false;
	for( char s = 'a'; s != 'z';++s)
	{
		if(chipset[0] == tolower(s))
		{
			valid = true;
			break;
		}
	}

	if (strstr(chipset, "System Product Name") != nullptr || valid == false)
	{
		getStr("/sys/devices/virtual/dmi/id/board_name", chipset);
	}

	sprintf(name, "%s %s", sys_vendor, chipset);

	deviceInfo.name = name;
	deviceInfo.manufacturer = sys_vendor;
	deviceInfo.chipset = chipset;
	return deviceInfo;
}


OsInfo LinuxDeviceInfoCollector::collectOsInfo()
{
	OsInfo osInfo;

	char name[256];
	char longname[256];
	char arch[256];
	char build[256];
	
	getUname("/bin/uname -sr", name);//linux generic...
	osInfo.name = name;

	getUname("/bin/uname -m", arch);//x86_64
	osInfo.arch = arch;//x86_64

	osInfo.shortName = "linux";//linux

	if(checkFile("/etc/os-release"))
	{
		getUname("cat /etc/os-release |grep -oP '\\bNAME=\"*\\K\\w+'", longname);
		osInfo.longName = longname;//LSB_NAME

		getUname("cat /etc/os-release |grep -oP '\\bVERSION=\"*\\K([^\"]*)'", build);
		osInfo.build = build;//LSB_VERSION
	}

	return osInfo;
}


std::vector<DisplayInfo> LinuxDeviceInfoCollector::collectDisplayInfo()
{
    std::vector<DisplayInfo> displayInfos;
/*
    for(NSDictionary *dict in systemProfile) {
        NSString *value = [dict objectForKey:@"_dataType"];
        if (![value isEqualToString:@"SPDisplaysDataType"]) {
            continue;
        }
        NSArray *items = [dict objectForKey:@"_items"];
        for(NSDictionary *gpuData in items) {
            NSArray *displays = [gpuData objectForKey:@"spdisplays_ndrvs"];
            if (displays == nullptr) {
                continue;
            }
            for(NSDictionary *dsp in displays) {
                DisplayInfo displayInfo;
                std::stringstream ss;

                NSString *builtIn = [dsp objectForKey:@"spdisplays_builtin"];
                if (builtIn && [builtIn isEqualToString:@"spdisplays_yes"])
                    ss << "Built-in ";
                
                NSString *type = [dsp objectForKey:@"spdisplays_display_type"];
                if (type && [type isEqualToString:@"spdisplays_retinaLCD"])
                    ss << "Retina ";
                
                NSString *displayName = [dsp objectForKey:@"_name"];
                if(displayName) {
                    std::string dname([displayName UTF8String]);
                    if(dname == "Color LCD") {
                        dname = "LCD";
                    }
                    ss << dname;
                }
                
                displayInfo.name = ss.str();
                
                NSString *displayRes = [dsp objectForKey:@"_spdisplays_pixels"];
                if(displayRes) {
                    std::string res([displayRes UTF8String]);
                    std::string separator(" x ");
                    int pos = res.find(separator);
                    displayInfo.widthPixels = atoi(res.substr(0, pos).c_str());
                    displayInfo.heightPixels = atoi(res.substr(pos+separator.length()).c_str());
                }
                displayInfos.push_back(displayInfo);
            }
        }
        break;
    }
*/
    return displayInfos;
}



//TODO
// get all physical cpu and handle the hyperthread/core_num
//
std::vector<CpuInfo> LinuxDeviceInfoCollector::collectCpuInfo()
{
/*	
On multi-core CPUs, /proc/cpuinfo contains the two fields "siblings" and "cpu cores" whereas the following calculation is applied:[4]

"siblings" = (HT per CPU package) * (# of cores per CPU package)
"cpu cores" = (# of cores per CPU package)

A CPU package means physical CPU which can have multiple cores (single core for one, dual core for two, quad core for four). This allows to better distinguish between hyper-threading and dual-core, i.e. the number of hyper-threads per CPU package can be calculated by siblings / CPU cores. If both values for a CPU package are the same, then hyper-threading is not supported.[5] For instance, a CPU package with siblings=2 and "cpu cores"=2 is a dual-core CPU but does not support hyper-threading.
*/
	char cpu0freqStr[256];
	getStr("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", cpu0freqStr);
	double cpu0freq = atof(cpu0freqStr);


	char cpuModelName[256];
	char cpuModelNameFinal[256];
	getUname("cat /proc/cpuinfo |grep 'model name'", cpuModelName);

	char *p = &cpuModelName[0];
	p = strtok(cpuModelName, ":");
	if(p)
	{
		p = strtok(NULL, ":");
	}
	else
	{
		p = &cpuModelName[0];
	}

	sprintf(cpuModelNameFinal, "%s", p);
	FILE *f = fopen("/proc/cpuinfo", "r");
	int siblings_num = 0;
	int core_num = 0;

	if(f)
	{
		int size = 256;
		char str[256];
		char cpu_cores[256];
		char siblings[256];

		while(fgets(str, size, f))
		{
			if(strstr(str, "siblings"))
			{
				sprintf(siblings,"%s", str);
			}
			if(strstr(str, "cpu cores"))
			{
				sprintf(cpu_cores, "%s", str);
			}
		}
		fclose(f);

		p = &siblings[0];
		p = strtok(siblings, ":");
		if(p)
		{
			p = strtok(NULL, ":");
		}
		siblings_num = atoi(p);


		p = &cpu_cores[0];
		p = strtok(cpu_cores, ":");
		if(p)
		{
			p = strtok(NULL, ":");
		}
		core_num = atoi(p);
	}	
	CpuInfo cpuInfo;
	cpuInfo.name = cpuModelNameFinal;
	cpuInfo.frequencyMHz = cpu0freq / 1000;
	cpuInfo.cores = core_num;
	cpuInfo.threads= siblings_num;

	return std::vector<CpuInfo>(1, cpuInfo);
}


std::vector<GpuInfo> LinuxDeviceInfoCollector::collectGpuInfo()
{
    std::vector<GpuInfo> gpuInfos;

    const int size = 512;
    char result[size];
    const char *gpuIDS = "lspci -nn|grep VGA|grep -o -E '(\\[\\w+:\\w+\\])'";
    //const char *gpuNameCmd = "lspci -nn|grep VGA|grep -o -E ': .*\\[' | cut -c 3- | rev | cut -c 3- | rev";
    const char *gpuNameCmd = "lspci -nn|grep VGA|grep -o -P '(?<=: ).*(?= \\[)'";

    struct S
    {
        char result[size];
    };
    std::vector<S> gpu_names;
    FILE *f2 = popen(gpuNameCmd, "r");
    if(f2)
    {
        while(fgets(result, size, f2))
        {
            S p;
            strcpy(p.result, result);
            gpu_names.push_back(p);
        }
    }
    pclose(f2);
    
    FILE *f = popen(gpuIDS, "r");
	if(f)
	{
        int it = 0;
		while(fgets(result, size, f))
        {
            int vendor;
            int product;
            sscanf(result, "[%x:%x]", &vendor, &product);
            GpuInfo gpuInfo;

            std::stringstream vendor_stream;
            std::stringstream product_stream;
            vendor_stream << "0x" << std::setw(4) << std::setfill('0') << std::hex << vendor;
            product_stream << "0x" << std::setw(4) << std::setfill('0') << std::hex << product;
            gpuInfo.vendor = vendor_stream.str(); //vendor id
            gpuInfo.ids = product_stream.str(); //product id
    //      printf("============%s %s\n", vendor_stream.str().c_str(), product_stream.str().c_str());
            gpuInfo.name = gpu_names[it++].result;
            //gpuInfo.memory = atoll([vram UTF8String]);
            gpuInfos.push_back(gpuInfo);
        }
        pclose(f);
    }

    return gpuInfos;
}


MultiGpuInfo LinuxDeviceInfoCollector::collectMultiGpuInfo()
{
    return MultiGpuInfo();
}



MemoryInfo LinuxDeviceInfoCollector::collectMemoryInfo()
{
    MemoryInfo memoryInfo;
	std::ifstream procFile("/proc/meminfo");
	std::string freemem;
	procFile >> freemem >> freemem;
	const uint64_t KB = 1024;
    memoryInfo.sizeBytes = std::stoull(freemem) * KB;
//    memoryInfo.details = "";
    return memoryInfo;
}



std::vector<StorageInfo> LinuxDeviceInfoCollector::collectStorageInfo()
{
    std::vector<StorageInfo> storageInfos;
/*
    for(NSDictionary *dict in systemProfile) {
        NSString *value = [dict objectForKey:@"_dataType"];
        if(![value isEqualToString:@"SPStorageDataType"]) {
            continue;
        }
        NSArray *items = [dict objectForKey:@"_items"];
        for(NSDictionary *storageData in items) {
            StorageInfo storageInfo;

            NSDictionary *physicalDrive = [storageData objectForKey:@"physical_drive"];
            if (!physicalDrive) {
                physicalDrive = [[storageData objectForKey:@"com.apple.corestorage.pv"] firstObject];
            }

            NSString *mediaName = [physicalDrive objectForKey:@"media_name"];
            if (mediaName) {
                storageInfo.name = [mediaName UTF8String];
            }

            NSString *isInternal = [physicalDrive objectForKey:@"is_internal_disk"];
            if (isInternal) {
                storageInfo.isRemovable = ![isInternal isEqualToString:@"yes"];
            }

            NSNumber *capacity = [storageData objectForKey:@"size_in_bytes"];
            if (capacity != nullptr) {
                storageInfo.sizeBytes = [capacity longLongValue];
            }

            storageInfos.push_back(storageInfo);
        }
        break;
    }
*/
    return storageInfos;
}


std::vector<BatteryInfo> LinuxDeviceInfoCollector::collectBatteryInfo()
{
	std::vector<BatteryInfo> batteryInfos;
/*
	const int size = 256;
	char result[size];
	int capacity;
	std::vector<BatteryInfo> batteryInfos;
	BatteryInfo batteryInfo;

	if(checkFile("/sys/class/power_supply/BAT0/status"))
	{
		getStr("/sys/class/power_supply/BAT0/technology", result);
		batteryInfo.technology = result;
		getStr("/sys/class/power_supply/BAT0/status", result);
		batteryInfo.isConnected = strstr(result, "Discharging") == NULL;
		batteryInfo.isCharging = strstr(result, "Charging") != NULL;

		getStr("/sys/class/power_supply/BAT0/capacity", result);
		capacity = atoi(result);
		batteryInfo.levelRatio = capacity / 100.0;
		batteryInfos.push_back(batteryInfo);
	}
	else
	{
		if(checkFile("/sys/class/power_supply/BAT1/status"))
		{
			getStr("/sys/class/power_supply/BAT1/technology", result);
			batteryInfo.technology = result;
			getStr("/sys/class/power_supply/BAT1/status", result);
			batteryInfo.isConnected = strstr(result, "Discharging") == NULL;
			batteryInfo.isCharging = strstr(result, "Charging") != NULL;

			getStr("/sys/class/power_supply/BAT1/capacity", result);
			capacity = atoi(result);
			batteryInfo.levelRatio = capacity / 100.0;
			batteryInfos.push_back(batteryInfo);
		}
	}
*/
	return batteryInfos;
}


std::vector<CameraInfo> LinuxDeviceInfoCollector::collectCameraInfo()
{
    std::vector<CameraInfo> cameraInfos;
    return cameraInfos;
}



FeatureInfo LinuxDeviceInfoCollector::collectFeatureInfo()
{
    FeatureInfo featureInfo;
    return featureInfo;
}



SensorInfo LinuxDeviceInfoCollector::collectSensorInfo()
{
    return SensorInfo();
}
