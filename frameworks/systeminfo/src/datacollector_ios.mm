/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "datacollector.h"

#include <sys/types.h>
#include <sys/sysctl.h>
#import <AVFoundation/AVFoundation.h>
#import <OpenGLES/EAGL.h>
#import <UIKit/UIKit.h>
#include <iostream>
#include <iomanip>
#include "utils.h"

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

#include <TargetConditionals.h>


#if !(TARGET_IPHONE_SIMULATOR) 
#include <Metal/Metal.h>

extern id <MTLDevice> MTLCreateSystemDefaultDevice(void) WEAK_IMPORT_ATTRIBUTE;
#endif

using namespace sysinf;

namespace detail {

    NSString *iosPlatformString(NSString *platform)
    {
        if ([platform isEqualToString:@"iPhone1,1"])    return @"iPhone 1G";
        if ([platform isEqualToString:@"iPhone1,2"])    return @"iPhone 3G";
        if ([platform isEqualToString:@"iPhone2,1"])    return @"iPhone 3GS";
        if ([platform isEqualToString:@"iPhone3,1"])    return @"iPhone 4";
        if ([platform isEqualToString:@"iPhone3,3"])    return @"Verizon iPhone 4";
        if ([platform isEqualToString:@"iPhone4,1"])    return @"iPhone 4S";
        if ([platform isEqualToString:@"iPhone5,1"])    return @"iPhone 5 (GSM)";
        if ([platform isEqualToString:@"iPhone5,2"])    return @"iPhone 5 (GSM+CDMA)";
        if ([platform isEqualToString:@"iPhone5,3"])    return @"iPhone 5c (GSM)";
        if ([platform isEqualToString:@"iPhone5,4"])    return @"iPhone 5c (GSM+CDMA)";
        if ([platform isEqualToString:@"iPhone6,1"])    return @"iPhone 5s (GSM)";
        if ([platform isEqualToString:@"iPhone6,2"])    return @"iPhone 5s (GSM+CDMA)";

        if ([platform isEqualToString:@"iPhone7,1"])    return @"iPhone 6 Plus";
        if ([platform isEqualToString:@"iPhone7,2"])    return @"iPhone 6";
        if ([platform isEqualToString:@"iPhone7,3"])    return @"iPhone 6 ??";
        if ([platform isEqualToString:@"iPhone8,1"])    return @"iPhone 6 ??";
        if ([platform isEqualToString:@"iPhone8,2"])    return @"iPhone 6 ??";
        if ([platform isEqualToString:@"iPhone8,3"])    return @"iPhone 6 ??";

        if ([platform isEqualToString:@"iPod1,1"])      return @"iPod Touch 1G";
        if ([platform isEqualToString:@"iPod2,1"])      return @"iPod Touch 2G";
        if ([platform isEqualToString:@"iPod3,1"])      return @"iPod Touch 3G";
        if ([platform isEqualToString:@"iPod4,1"])      return @"iPod Touch 4G";
        if ([platform isEqualToString:@"iPod5,1"])      return @"iPod Touch 5G";
        if ([platform isEqualToString:@"iPad1,1"])      return @"iPad";
        if ([platform isEqualToString:@"iPad2,1"])      return @"iPad 2 (WiFi)";
        if ([platform isEqualToString:@"iPad2,2"])      return @"iPad 2 (GSM)";
        if ([platform isEqualToString:@"iPad2,3"])      return @"iPad 2 (CDMA)";
        if ([platform isEqualToString:@"iPad2,4"])      return @"iPad 2 (WiFi)";
        if ([platform isEqualToString:@"iPad2,5"])      return @"iPad Mini (WiFi)";
        if ([platform isEqualToString:@"iPad2,6"])      return @"iPad Mini (GSM)";
        if ([platform isEqualToString:@"iPad2,7"])      return @"iPad Mini (GSM+CDMA)";
        if ([platform isEqualToString:@"iPad3,1"])      return @"iPad 3 (WiFi)";
        if ([platform isEqualToString:@"iPad3,2"])      return @"iPad 3 (GSM+CDMA)";
        if ([platform isEqualToString:@"iPad3,3"])      return @"iPad 3 (GSM)";
        if ([platform isEqualToString:@"iPad3,4"])      return @"iPad 4 (WiFi)";
        if ([platform isEqualToString:@"iPad3,5"])      return @"iPad 4 (GSM)";
        if ([platform isEqualToString:@"iPad3,6"])      return @"iPad 4 (GSM+CDMA)";
        if ([platform isEqualToString:@"iPad4,1"])      return @"iPad Air (WiFi)";
        if ([platform isEqualToString:@"iPad4,2"])      return @"iPad Air (GSM+CDMA)";
        if ([platform isEqualToString:@"iPad4,3"])      return @"iPad Air (GSM)";
        if ([platform isEqualToString:@"i386"])         return @"Simulator";
        if ([platform isEqualToString:@"x86_64"])       return @"Simulator";
        return platform;
    }

    int getPpi(NSString *platform)
    {
        if ([platform isEqualToString:@"iPhone1,1"])    return 163;
        if ([platform isEqualToString:@"iPhone1,2"])    return 163;
        if ([platform isEqualToString:@"iPhone2,1"])    return 163;
        if ([platform isEqualToString:@"iPhone3,1"])    return 163 * 2;
        if ([platform isEqualToString:@"iPhone3,3"])    return 163 * 2;
        if ([platform isEqualToString:@"iPhone4,1"])    return 163 * 2;
        if ([platform isEqualToString:@"iPhone5,1"])    return 163 * 2;
        if ([platform isEqualToString:@"iPhone5,2"])    return 163 * 2;
        if ([platform isEqualToString:@"iPhone5,3"])    return 163 * 2;
        if ([platform isEqualToString:@"iPhone5,4"])    return 163 * 2;
        if ([platform isEqualToString:@"iPhone6,1"])    return 163 * 2;
        if ([platform isEqualToString:@"iPhone6,2"])    return 163 * 2;
        
        if ([platform isEqualToString:@"iPhone7,1"])    return 401;
        if ([platform isEqualToString:@"iPhone7,2"])    return 163 * 2;
        if ([platform isEqualToString:@"iPhone7,3"])    return 401;
        if ([platform isEqualToString:@"iPhone8,1"])    return 401;
        if ([platform isEqualToString:@"iPhone8,2"])    return 401;
        if ([platform isEqualToString:@"iPhone8,3"])    return 401;
        
        if ([platform isEqualToString:@"iPod1,1"])      return 163;
        if ([platform isEqualToString:@"iPod2,1"])      return 163;
        if ([platform isEqualToString:@"iPod3,1"])      return 163;
        if ([platform isEqualToString:@"iPod4,1"])      return 163;
        if ([platform isEqualToString:@"iPod5,1"])      return 163;
        
        if ([platform isEqualToString:@"iPad1,1"])      return 132;
        if ([platform isEqualToString:@"iPad2,1"])      return 132;
        if ([platform isEqualToString:@"iPad2,2"])      return 132;
        if ([platform isEqualToString:@"iPad2,3"])      return 132;
        if ([platform isEqualToString:@"iPad2,4"])      return 132;
        if ([platform isEqualToString:@"iPad2,5"])      return 163;
        if ([platform isEqualToString:@"iPad2,6"])      return 163;
        if ([platform isEqualToString:@"iPad2,7"])      return 163;
        if ([platform isEqualToString:@"iPad3,1"])      return 132 * 2;
        if ([platform isEqualToString:@"iPad3,2"])      return 132 * 2;
        if ([platform isEqualToString:@"iPad3,3"])      return 132 * 2;
        if ([platform isEqualToString:@"iPad3,4"])      return 132 * 2;
        if ([platform isEqualToString:@"iPad3,5"])      return 132 * 2;
        if ([platform isEqualToString:@"iPad3,6"])      return 132 * 2;
        if ([platform isEqualToString:@"iPad4,1"])      return 132 * 2;
        if ([platform isEqualToString:@"iPad4,2"])      return 132 * 2;
        if ([platform isEqualToString:@"iPad4,3"])      return 132 * 2;
        
        if ([platform isEqualToString:@"i386"])         return 163;
        if ([platform isEqualToString:@"x86_64"])       return 163;
        return 163;
    }

    int64_t totalDiskSpace()
    {
        NSDictionary *fattributes = [[NSFileManager defaultManager] attributesOfFileSystemForPath:NSHomeDirectory() error:nil];
        return [[fattributes objectForKey:NSFileSystemSize] longLongValue];
    }


    DeviceInfo CollectDeviceInfo() {
        size_t size;
        sysctlbyname("hw.machine", NULL, &size, NULL, 0);
        char *machine = (char*)malloc(size);
        sysctlbyname("hw.machine", machine, &size, NULL, 0);
        NSString *platform = [NSString stringWithCString:machine encoding:NSUTF8StringEncoding];
        free(machine);
        
        m_properties->setString(DEVICE_NAME, [iosPlatformString(platform) UTF8String]);
        m_properties->setString("device/id", [platform UTF8String]);

        DeviceInfo deviceInfo;
        deviceInfo.name = [iosPlatformString(platform) UTF8String];
        deviceInfo.id = [platform UTF8String];
        deviceInfo.manufacturer = "Apple";
        return deviceInfo;
    }
}

void DataCollector::Collect()
{
//     //---------------- Corporate check ------------------
// #if IS_CORPORATE
//     m_properties->setBool(CORPORATE, true);
// #endif
    
//     //---------------- Battery ----------------
//     UIDeviceBatteryState bstate = [[UIDevice currentDevice] batteryState];
    
//     m_properties->setDouble(BATTERY_LEVEL, [[UIDevice currentDevice] batteryLevel]);
//     m_properties->setBool(BATTERY_IS_CHARGING, bstate == UIDeviceBatteryStateCharging);
//     m_properties->setBool(BATTERY_IS_CONNECTED, bstate == UIDeviceBatteryStateCharging || bstate == UIDeviceBatteryStateFull);
    
//     //---------------- Camera ----------------
//     NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
//     int i = 0;
//     for (AVCaptureDevice *device in devices)
//     {
//         m_properties->setString(CreateIndexedKey(CAMERA, CAMERA_TYPE, i),
//                                 (device.position == AVCaptureDevicePositionBack) ? "CAMERA_TYPE_FRONT" : "CAMERA_TYPE_BACK");
//         m_properties->setBool(CreateIndexedKey(CAMERA, CAMERA_HAS_FLASH, i),
//                               device.hasFlash);
        
//         m_properties->setBool(CreateIndexedKey(CAMERA, "has/torch", i), device.hasTorch);
//         m_properties->setBool(CreateIndexedKey(CAMERA, "is/auto_focus_range_restriction_supported", i), device.isAutoFocusRangeRestrictionSupported);
//         m_properties->setBool(CreateIndexedKey(CAMERA, "is/exposure_point_of_interest_supported", i), device.isExposurePointOfInterestSupported);
//         m_properties->setBool(CreateIndexedKey(CAMERA, "is/focus_point_of_interest_supported", i), device.isFocusPointOfInterestSupported);
//         m_properties->setBool(CreateIndexedKey(CAMERA, "is/low_light_boost_supported", i), device.isLowLightBoostSupported);
//         m_properties->setBool(CreateIndexedKey(CAMERA, "is/smooth_auto_focus_supported", i), device.isSmoothAutoFocusSupported);
//         m_properties->setString(CreateIndexedKey(CAMERA, "model_id", i), [device.modelID UTF8String]);
//         m_properties->setString(CreateIndexedKey(CAMERA, "unique_id", i), [device.uniqueID UTF8String]);
        
//         if((device.position == AVCaptureDevicePositionBack))
//             m_properties->setBool(FEATURES_BACK_CAMERA, true);
//         if(device.position == AVCaptureDevicePositionFront)
//             m_properties->setBool(FEATURES_FRONT_CAMERA, true);
        
//         i++;
//     }
//     m_properties->setInt(CAMERA_COUNT, i);
    
//     //---------------- Chipset ----------------
//     m_properties->setString(CHIPSET_NAME, "");
    
//     //---------------- CPU ----------------
//     m_properties->setInt(CPU_CORES, (int)[NSProcessInfo processInfo].processorCount);
    
//     // //---------------- GPU ----------------
//     // m_properties->setString(CreateIndexedKey(GPU, GPU_NAME, 0), [gl_renderer UTF8String]);
//     // m_properties->setInt(GPU_COUNT, 1);
    
//     //---------------- Device ----------------
//     // Gets a string with the device model
//     size_t size;
//     sysctlbyname("hw.machine", NULL, &size, NULL, 0);
//     char *machine = (char*)malloc(size);
//     sysctlbyname("hw.machine", machine, &size, NULL, 0);
//     NSString *platform = [NSString stringWithCString:machine encoding:NSUTF8StringEncoding];
//     free(machine);
    
//     m_properties->setString(DEVICE_NAME, [iosPlatformString(platform) UTF8String]);
//     m_properties->setString("device/id", [platform UTF8String]);
    
//     //---------------- Display ----------------
//     float scaleFactor = 1.0f;
//     if ([[UIScreen mainScreen] respondsToSelector:@selector(nativeScale)])
//     {
//         scaleFactor = [[UIScreen mainScreen] nativeScale];
//     }
//     else if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)])
//     {
//         scaleFactor = [[UIScreen mainScreen] scale];
//     }
    
//     int x_res = [UIScreen mainScreen].applicationFrame.size.width * scaleFactor;
//     int y_res = [UIScreen mainScreen].applicationFrame.size.height * scaleFactor;
    
//     int ppi = getPpi(platform);
//     m_properties->setInt(DISPLAY_DPI_X, ppi);
//     m_properties->setInt(DISPLAY_DPI_Y, ppi);
    
//     int x = x_res > y_res ? x_res : y_res;
//     int y = x_res > y_res ? y_res : x_res;
    
//     m_properties->setInt(DISPLAY_RES_X, x);
//     m_properties->setInt(DISPLAY_RES_Y, y);
//     m_properties->setDouble("display/scale_factor", scaleFactor);
    
//     float inch = powf( powf(((float)x_res/(float)ppi), 2.0f) + powf(((float)y_res/(float)ppi), 2.0f), 0.5f);
    
//     std::stringstream ss;
//     ss.str("");
//     ss << x << " x " << y;
//     ss <<  ", " << std::setprecision(2) << inch << "\"";
    
//     m_properties->setString(DISPLAY_MAJOR, ss.str());
    
    
//     //---------------- Features ----------------
    
//     //---------------- Memory ----------------
//     m_properties->setLong(MEMORY_SIZE, [NSProcessInfo processInfo].physicalMemory);
    
//     //---------------- OS ----------------
//     m_properties->setString(OS_NAME, [[[UIDevice currentDevice] systemName] UTF8String]);
//     m_properties->setString(OS_BUILD, [[[UIDevice currentDevice] systemVersion] UTF8String]);
    
    
//     //---------------- Storage ----------------
//     m_properties->setLong(CreateIndexedKey(STORAGE, STORAGE_SIZE, 0), totalDiskSpace());
//     m_properties->setBool(CreateIndexedKey(STORAGE, STORAGE_ISREMOVABLE, 0), false);
//     m_properties->setInt(STORAGE_COUNT, 1);
    
//     //---------------- Appinfo ----------------
//     m_properties->setString(APPINFO_BENCHMARK_ID, [[[NSBundle mainBundle] objectForInfoDictionaryKey:(NSString*) @"BUIProdctId"] UTF8String]);
//     m_properties->setString(APPINFO_INSTALLERNAME, "appstore");
//     m_properties->setString(APPINFO_STORENAME,     "appstore");
//     m_properties->setString(APPINFO_LOCALE, [((NSString *)[NSLocale preferredLanguages].firstObject) UTF8String]);
//     m_properties->setString(APPINFO_PACKAGE_NAME, [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleIdentifier"] UTF8String]);
//     m_properties->setString(APPINFO_PLATFORM, "apple");
//     m_properties->setString(APPINFO_VERSION, [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"BUIVersion"] UTF8String]);
    
    
    
//     //TODO delete this
//     for (PropertyIter it = m_properties->iterator(); !it.done(); it.next())
//     {
//         std::cout << "\"" << it.name()
//         << "\": \"" << it.value().getString()
//         << "\"" << std::endl;
//     };
}
