/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "osxdeviceinfocollector.h"

#include "Poco/Process.h"
#include "Poco/PipeStream.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <IOKit/ps/IOPowerSources.h>
#import <IOKit/ps/IOPSKeys.h>

#include <sys/types.h>
#include <sys/sysctl.h>
#include <cmath>
#include <set>
#include <sstream>
#include <fstream>
#include <iostream>



using namespace sysinf;

NSArray *systemProfile = nil;

struct MemInfo
{
    std::string size;
    std::string type;
    std::string speed;
    
    bool isNull() { return size.empty() && type.empty() && speed.empty(); }
    bool equalsTo(const MemInfo &memInfo)
    {
        return size == memInfo.size &&
                type == memInfo.type &&
                speed == memInfo.speed;
    }
};

inline std::string fromSysCtl(const char* key)
{
    size_t size;
    sysctlbyname(key, NULL, &size, NULL, 0);
    std::string str(' ', size);
    sysctlbyname(key, &str[0], &size, NULL, 0);
    return str;
}

template <class Type>
inline Type fromSysCtl(const char* key)
{
    Type value;
    size_t size;
    size = sizeof(value);
    sysctlbyname(key, &value, &size, NULL, 0);
    return value;
}

inline std::string fromPlist(const std::string &plist, const std::string &key)
{
    std::string keyEntry = "<key>" + key + "</key>";
    int keyPos = plist.find(keyEntry);
    int valueFirst = plist.find("<string>", keyPos)  + std::string("<string>").length();
    int valueLast = plist.find("</string>", valueFirst);
    std::string value = plist.substr(valueFirst, valueLast - valueFirst);
    return value;
}

std::string codeName(const std::string &versionStr)
{
    int firstDot = versionStr.find('.');
    int secondDot = versionStr.find(firstDot, '.');
    int minor = atoi(versionStr.substr(firstDot + 1, secondDot - firstDot).c_str());
    int maior = atoi(versionStr.substr(0, firstDot).c_str());
    switch(maior)
    {
        case 11:
        {
          return "Big Sur";
        }
        case 12:
        {
            return "Monterey";
        }
        default:
            return "Unknown";
    }
    switch(minor)
    {
    case 1:
        return "Puma";
    case 2:
        return "Jaguar";
    case 3:
        return "Panther";
    case 4:
        return "Tiger";
    case 5:
        return "Leopard";
    case 6:
        return "Snow Leopard";
    case 7:
        return "Lion";
    case 8:
        return "Mountain Lion";
    case 9:
        return "Mavericks";
    case 10:
        return "Yosemite";
    case 11:
        return "El Capitan";
    case 12:
        return "Sierra Fuji"; // with this and below versions are macOS-s
    case 13:
        return "High Sierra";
    case 14:
        return "Mojave";
    case 15:
        return "Catalina";
    default:
        return "Unknown";
    }
}

void initSystemProfile()
{
    Poco::Pipe outPipe;
    Poco::Process::Args args;
    args.push_back("SPDisplaysDataType");
    args.push_back("SPStorageDataType");
    args.push_back("SPBluetoothDataType");
    args.push_back("SPCameraDataType");
    args.push_back("SPAirPortDataType");
    args.push_back("SPMemoryDataType");
    args.push_back("-xml");
    Poco::ProcessHandle ph(Poco::Process::launch("system_profiler", args, 0, &outPipe, 0));
    Poco::PipeInputStream istream(outPipe);
    std::stringstream sstream;
    char line[1024];
    while(!istream.eof())
    {
        istream.getline(line, 1024);
        sstream << line << "\n";
    }
    NSString *nsstr = [NSString stringWithCString:sstream.str().c_str() encoding:NSASCIIStringEncoding];
    NSData *plistData = [nsstr dataUsingEncoding:NSUTF8StringEncoding];
    NSError *error = nil;
    NSPropertyListFormat format;
    systemProfile = [NSPropertyListSerialization propertyListWithData:plistData options:NSPropertyListImmutable format:&format error:&error];
    if(error != nil)
    {
        NSLog(@"SystemProfile.plist to NSArray error: '%@'", error);
        return;
    }
}



DeviceInfo OsxDeviceInfoCollector::collectDeviceInfo()
{
    initSystemProfile();
    DeviceInfo deviceInfo;
    deviceInfo.manufacturer = "Apple";
    deviceInfo.name = fromSysCtl("hw.model");
    return deviceInfo;
}



OsInfo OsxDeviceInfoCollector::collectOsInfo()
{
    OsInfo osInfo;
    osInfo.shortName = "macosx";
    osInfo.arch = fromSysCtl("hw.machine");
    
    std::ifstream ifs("/System/Library/CoreServices/SystemVersion.plist");
    if(ifs.is_open()) {
        std::ostringstream plist;
        plist << ifs.rdbuf();
        std::string version = fromPlist(plist.str(), "ProductVersion");
        osInfo.build = fromPlist(plist.str(), "ProductBuildVersion");
        osInfo.name = fromPlist(plist.str(), "ProductName");
        osInfo.longName = osInfo.name + " " + version + " " + codeName(version);
    } else {
        NSString* osVersionNSString = [[NSProcessInfo processInfo] operatingSystemVersionString];
        std::string osVersion([osVersionNSString UTF8String]);
        osVersion.erase(osVersion.find("Version "), std::string("Version ").length());
        int buildStart = osVersion.find("(Build ") + std::string("(Build ").length();
        std::string version = osVersion.substr(0, osVersion.find(" "));
        osInfo.build = osVersion.substr(buildStart, osVersion.find(")")-buildStart);
        osInfo.name = "Mac OS X ";
        osInfo.longName = osInfo.name + " " + version + " " + codeName(version);
    }
    return osInfo;
}



std::vector<DisplayInfo> OsxDeviceInfoCollector::collectDisplayInfo()
{
    std::vector<DisplayInfo> displayInfos;
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
    return displayInfos;
}



std::vector<CpuInfo> OsxDeviceInfoCollector::collectCpuInfo()
{
    CpuInfo cpuInfo;
    cpuInfo.name = fromSysCtl("machdep.cpu.brand_string");
    cpuInfo.frequencyMHz = fromSysCtl<uint64_t>("hw.cpufrequency") / 1.0E6;
    cpuInfo.cores = fromSysCtl<int>("machdep.cpu.core_count");
    cpuInfo.threads = fromSysCtl<int>("machdep.cpu.thread_count");
    return std::vector<CpuInfo>(1, cpuInfo);
}



std::vector<GpuInfo> OsxDeviceInfoCollector::collectGpuInfo()
{
    std::vector<GpuInfo> gpuInfos;
    for(NSDictionary *dict in systemProfile) {
        NSString *value = [dict objectForKey:@"_dataType"];
        if (![value isEqualToString:@"SPDisplaysDataType"]) {
            continue;
        }
        NSArray *items = [dict objectForKey:@"_items"];
        for(NSDictionary *gpuData in items) {
            GpuInfo gpuInfo;

            NSString *model = [gpuData objectForKey:@"sppci_model"];
            if(model) {
                gpuInfo.name = [model UTF8String];
            }
            std::stringstream ss;

            NSString *vram = [gpuData objectForKey:@"spdisplays_vram"];
            if(vram) {
                gpuInfo.memory = atoll([vram UTF8String]);
                ss << [vram UTF8String] << " ";
            }

            NSString *vendor = [gpuData objectForKey:@"spdisplays_vendor"];
            if(vendor) {
                std::string vendorStd = [vendor UTF8String];
                int start = vendorStd.find("(") + 1;
                int length = vendorStd.find(")", start) - start;
                if (length > 0) {
                    vendorStd = vendorStd.substr(start, length);
                }
                gpuInfo.vendor = vendorStd;
            }

            NSString *deviceId = [gpuData objectForKey:@"spdisplays_device-id"];
            if(deviceId) {
                gpuInfo.ids = [deviceId UTF8String];
            }
            
            NSString *pcie = [gpuData objectForKey:@"spdisplays_pcie_width"];
            if(pcie) {
                gpuInfo.pcie = [pcie UTF8String];
                ss << "PCIe " << [pcie UTF8String];
            }

            gpuInfos.push_back(gpuInfo);
        }
        break;
    }
    return gpuInfos;
}



MultiGpuInfo OsxDeviceInfoCollector::collectMultiGpuInfo()
{
    return MultiGpuInfo();
}



MemoryInfo OsxDeviceInfoCollector::collectMemoryInfo()
{
    MemoryInfo memoryInfo;
    memoryInfo.sizeBytes = fromSysCtl<uint64_t>("hw.memsize");

    for (NSDictionary *dict in systemProfile) {
        NSString *value = [dict objectForKey:@"_dataType"];
        if(![value isEqualToString:@"SPMemoryDataType"]) {
            continue;
        }
        NSArray *items = [dict objectForKey:@"_items"];
        if ([items count] > 0)
        {
            NSDictionary *memDict = [items objectAtIndex:0];
            NSArray *memItems = [memDict objectForKey:@"_items"];
            std::vector<MemInfo> memInfos;
            for(NSDictionary *memItem in memItems)
            {
                MemInfo mi;
                mi.size = [[memItem objectForKey:@"dimm_size"] UTF8String];
                mi.type = [[memItem objectForKey:@"dimm_type"] UTF8String];
                mi.speed = [[memItem objectForKey:@"dimm_speed"] UTF8String];
                memInfos.push_back(mi);
            }
            bool identic = true;
            for(size_t i = 1; i < memInfos.size() && 2 <= memInfos.size(); i++) {
                if(!memInfos[i-1].equalsTo(memInfos[i])) {
                    identic = false;
                }
		
            }
	    if (identic)
	    {
		    identic = (memInfos.size() > 0);
	    }
	    std::stringstream ss;
            if(identic)
            {
                ss << memInfos.size() << " x " << memInfos[0].size << " " << memInfos[0].type << " " << memInfos[0].speed;
            }
            else
            {
                for(size_t i(0); i < memInfos.size(); i++)
                {
                    ss << memInfos[i].size << " " << memInfos[i].type << " " << memInfos[i].speed;
                    if(i != memInfos.size() - 1)
                        ss << " / ";
                }
            }
            memoryInfo.details = ss.str();
        }
        break;
    }
    return memoryInfo;
}



std::vector<StorageInfo> OsxDeviceInfoCollector::collectStorageInfo()
{
    std::vector<StorageInfo> storageInfos;
    std::set<std::string> disks;
    for(NSDictionary *dict in systemProfile)
    {
        NSString *value = [dict objectForKey:@"_dataType"];
        if(![value isEqualToString:@"SPStorageDataType"])
        {
            continue;
        }
        NSArray *items = [dict objectForKey:@"_items"];
        for(NSDictionary *storageData in items)
        {
            StorageInfo storageInfo;

            NSString *bsd_name = [storageData objectForKey:@"bsd_name"];
            if (bsd_name)
            {
                std::string name = [bsd_name UTF8String];
                std::string diskname = name.substr(0, 5);
                if(disks.find(diskname) != disks.end())
                {
                    // disk is added, skip it
                    // e.g.: below are the same disk
                    // disk3s1
                    // disk3s1s1
                    // disk3s2
                    continue;
                }
                disks.insert(diskname);
            }
            
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
    return storageInfos;
}



std::vector<BatteryInfo> OsxDeviceInfoCollector::collectBatteryInfo()
{
    std::vector<BatteryInfo> batteryInfos;
    
    CFTypeRef info = IOPSCopyPowerSourcesInfo();
    if (info == nullptr) {
        return batteryInfos;
    }
    CFArrayRef list = IOPSCopyPowerSourcesList(info);
    if (list == nullptr) {
    CFRelease(info);
        return batteryInfos;
    }

    for (int i = 0; i < CFArrayGetCount(list); ++i) {
        const void* item = CFArrayGetValueAtIndex(list, i);
        NSDictionary* battery = (NSDictionary*)IOPSGetPowerSourceDescription(info, item);
        if (battery == nullptr) {
            continue;
        }
        BatteryInfo batteryInfo;
        batteryInfo.isConnected = [(NSString*)[battery objectForKey:@kIOPSPowerSourceStateKey] isEqualToString:@kIOPSACPowerValue];
        batteryInfo.isCharging = [[battery objectForKey:@kIOPSIsChargingKey] boolValue];
        double capacity = [[battery objectForKey:@kIOPSCurrentCapacityKey] doubleValue];
        double maxCapacity = [[battery objectForKey:@kIOPSMaxCapacityKey] doubleValue];
        batteryInfo.levelRatio = capacity/maxCapacity;
        batteryInfos.push_back(batteryInfo);
    }
    
    CFRelease(list);
    CFRelease(info);
    return batteryInfos;
}



std::vector<CameraInfo> OsxDeviceInfoCollector::collectCameraInfo()
{
    std::vector<CameraInfo> cameraInfos;
    for (NSDictionary *dict in systemProfile) {
        NSString *value = [dict objectForKey:@"_dataType"];
        if (![value isEqualToString:@"SPCameraDataType"]) {
            continue;
        }
        NSArray *items = [dict objectForKey:@"_items"];
        for (NSDictionary *camItem in items) {
            CameraInfo cameraInfo;
            cameraInfo.name = [[camItem objectForKey:@"_name"] UTF8String];
            cameraInfo.type = "CAMERA_TYPE_FRONT";
            /*
            cameraInfo.pictureWidthPixels = device.ImageResX;
            cameraInfo.pictureHeightPixels = device.ImageResY;
            cameraInfo.pictureResolutionMP = device.ImageResX * device.ImageResY * 0.000001;
            cameraInfo.videoWidthPixels = device.VideoResX;
            cameraInfo.videoHeightPixels = device.VideoResY;
            cameraInfo.videoResolutionMP = device.VideoResX * device.VideoResY * 0.000001;
            */
            cameraInfos.push_back(cameraInfo);
        }
        break;
    }
    return cameraInfos;
}



FeatureInfo OsxDeviceInfoCollector::collectFeatureInfo()
{
    FeatureInfo featureInfo;

    for(NSDictionary *dict in systemProfile) {
        NSString *value = [dict objectForKey:@"_dataType"];
        if([value isEqualToString:@"SPBluetoothDataType"]) {
            NSArray *items = [dict objectForKey:@"_items"];
            featureInfo.features["bluetooth"] = ([items count] > 0);
        } else if([value isEqualToString:@"SPCameraDataType"]) {
            NSArray *items = [dict objectForKey:@"_items"];
            featureInfo.features["camera (face)"] = ([items count] > 0);
        } else if([value isEqualToString:@"SPAirPortDataType"]) {
            NSArray *items = [dict objectForKey:@"_items"];
            featureInfo.features["wifi"] = ([items count] > 0);
        }
    }

    return featureInfo;
}



SensorInfo OsxDeviceInfoCollector::collectSensorInfo()
{
    return SensorInfo();
}
