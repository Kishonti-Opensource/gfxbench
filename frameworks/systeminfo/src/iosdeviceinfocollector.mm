/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "iosdeviceinfocollector.h"

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
        // Apple TV
        if ([platform isEqualToString:@"AppleTV2,1"])      return @"Apple TV 2G";
        if ([platform isEqualToString:@"AppleTV3,1"])      return @"Apple TV 3G";
        if ([platform isEqualToString:@"AppleTV3,2"])      return @"Apple TV 3G";
        if ([platform isEqualToString:@"AppleTV5,3"])      return @"Apple TV 4G";

        // Apple Watch
        if ([platform isEqualToString:@"Watch1,1"])      return @"Apple Watch";
        if ([platform isEqualToString:@"Watch1,2"])      return @"Apple Watch";

        if ([platform isEqualToString:@"iPhone1,1"])    return @"iPhone 2G";
        if ([platform isEqualToString:@"iPhone1,2"])    return @"iPhone 3G";
        if ([platform isEqualToString:@"iPhone2,1"])    return @"iPhone 3GS";
        if ([platform isEqualToString:@"iPhone3,1"])    return @"iPhone 4";
        if ([platform isEqualToString:@"iPhone3,2"])    return @"iPhone 4";
        if ([platform isEqualToString:@"iPhone3,3"])    return @"iPhone 4";
        if ([platform isEqualToString:@"iPhone4,1"])    return @"iPhone 4S";
        if ([platform isEqualToString:@"iPhone5,1"])    return @"iPhone 5";
        if ([platform isEqualToString:@"iPhone5,2"])    return @"iPhone 5 (GSM+CDMA)";
        if ([platform isEqualToString:@"iPhone5,3"])    return @"iPhone 5c (GSM)";
        if ([platform isEqualToString:@"iPhone5,4"])    return @"iPhone 5c (GSM+CDMA)";
        if ([platform isEqualToString:@"iPhone6,1"])    return @"iPhone 5s (GSM)";
        if ([platform isEqualToString:@"iPhone6,2"])    return @"iPhone 5s (GSM+CDMA)";
        if ([platform isEqualToString:@"iPhone7,1"])    return @"iPhone 6 Plus";
        if ([platform isEqualToString:@"iPhone7,2"])    return @"iPhone 6";
        if ([platform isEqualToString:@"iPhone8,1"])    return @"iPhone 6s";
        if ([platform isEqualToString:@"iPhone8,2"])    return @"iPhone 6s Plus";

        if ([platform isEqualToString:@"iPod1,1"])      return @"iPod Touch (1 Gen)";
        if ([platform isEqualToString:@"iPod2,1"])      return @"iPod Touch (2 Gen)";
        if ([platform isEqualToString:@"iPod3,1"])      return @"iPod Touch (3 Gen)";
        if ([platform isEqualToString:@"iPod4,1"])      return @"iPod Touch (4 Gen)";
        if ([platform isEqualToString:@"iPod5,1"])      return @"iPod Touch (5 Gen)";

        if ([platform isEqualToString:@"iPad1,1"])      return @"iPad";
        if ([platform isEqualToString:@"iPad1,2"])      return @"iPad 3G";
        if ([platform isEqualToString:@"iPad2,1"])      return @"iPad 2 (WiFi)";
        if ([platform isEqualToString:@"iPad2,2"])      return @"iPad 2";
        if ([platform isEqualToString:@"iPad2,3"])      return @"iPad 2 (CDMA)";
        if ([platform isEqualToString:@"iPad2,4"])      return @"iPad 2";
        if ([platform isEqualToString:@"iPad2,5"])      return @"iPad Mini (WiFi)";
        if ([platform isEqualToString:@"iPad2,6"])      return @"iPad Mini";
        if ([platform isEqualToString:@"iPad2,7"])      return @"iPad Mini (GSM+CDMA)";
        if ([platform isEqualToString:@"iPad3,1"])      return @"iPad 3 (WiFi)";
        if ([platform isEqualToString:@"iPad3,2"])      return @"iPad 3 (GSM+CDMA)";
        if ([platform isEqualToString:@"iPad3,3"])      return @"iPad 3";
        if ([platform isEqualToString:@"iPad3,4"])      return @"iPad 4 (WiFi)";
        if ([platform isEqualToString:@"iPad3,5"])      return @"iPad 4";
        if ([platform isEqualToString:@"iPad3,6"])      return @"iPad 4 (GSM+CDMA)";
        if ([platform isEqualToString:@"iPad4,1"])      return @"iPad Air (WiFi)";
        if ([platform isEqualToString:@"iPad4,2"])      return @"iPad Air (Cellular)";
        if ([platform isEqualToString:@"iPad4,4"])      return @"iPad Mini 2 (WiFi)";
        if ([platform isEqualToString:@"iPad4,5"])      return @"iPad Mini 2 (Cellular)";
        if ([platform isEqualToString:@"iPad4,6"])      return @"iPad Mini 2";
        if ([platform isEqualToString:@"iPad4,7"])      return @"iPad Mini 3";
        if ([platform isEqualToString:@"iPad4,8"])      return @"iPad Mini 3";
        if ([platform isEqualToString:@"iPad4,9"])      return @"iPad Mini 3";
        if ([platform isEqualToString:@"iPad5,1"])      return @"iPad Mini 4 (WiFi)";
        if ([platform isEqualToString:@"iPad5,2"])      return @"iPad Mini 4 (LTE)";
        if ([platform isEqualToString:@"iPad5,3"])      return @"iPad Air 2 (WiFi)";
        if ([platform isEqualToString:@"iPad5,4"])      return @"iPad Air 2 (Cellular)";
        if ([platform isEqualToString:@"iPad6,7"])      return @"iPad Pro (WiFi)";
        if ([platform isEqualToString:@"iPad6,8"])      return @"iPad Pro (Cellular)";

        if ([platform isEqualToString:@"AppleTV2,1"])   return @"Apple TV 2G";
        if ([platform isEqualToString:@"AppleTV3,1"])   return @"Apple TV 3";
        if ([platform isEqualToString:@"AppleTV3,2"])   return @"Apple TV 3 (2013)";

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
        if ([platform isEqualToString:@"iPhone8,1"])    return 163 * 2;
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
        if ([platform isEqualToString:@"iPad4,4"])      return 326;
        if ([platform isEqualToString:@"iPad4,5"])      return 326;
        if ([platform isEqualToString:@"iPad4,6"])      return 326;
        if ([platform isEqualToString:@"iPad4,7"])      return 326;
        if ([platform isEqualToString:@"iPad4,8"])      return 326;
        if ([platform isEqualToString:@"iPad4,9"])      return 326;
        if ([platform isEqualToString:@"iPad5,1"])      return 326;
        if ([platform isEqualToString:@"iPad5,2"])      return 326; 
        if ([platform isEqualToString:@"iPad5,3"])      return 264;
        if ([platform isEqualToString:@"iPad5,4"])      return 264;
        if ([platform isEqualToString:@"iPad6,7"])      return 264;
        if ([platform isEqualToString:@"iPad6,8"])      return 264;
        
        if ([platform isEqualToString:@"i386"])         return 163;
        if ([platform isEqualToString:@"x86_64"])       return 163;


        return 163;
    }

    int64_t totalDiskSpace()
    {
        NSDictionary *fattributes = [[NSFileManager defaultManager] attributesOfFileSystemForPath:NSHomeDirectory() error:nil];
        return [[fattributes objectForKey:NSFileSystemSize] longLongValue];
    }
}


DeviceInfo IosDeviceInfoCollector::collectDeviceInfo() {
    size_t size;
    sysctlbyname("hw.machine", NULL, &size, NULL, 0);
    char *machine = (char*)malloc(size);
    sysctlbyname("hw.machine", machine, &size, NULL, 0);
    NSString *platform = [NSString stringWithCString:machine encoding:NSUTF8StringEncoding];
    free(machine);

    DeviceInfo deviceInfo;
    deviceInfo.name = [detail::iosPlatformString(platform) UTF8String];
    deviceInfo.attributes.insert(std::pair<std::string,std::string>("id",[platform UTF8String]));
    deviceInfo.manufacturer = "Apple";
    return deviceInfo;
}


OsInfo IosDeviceInfoCollector::collectOsInfo() {
    OsInfo osInfo;   

    osInfo.name = [[[UIDevice currentDevice] systemName] UTF8String];
    osInfo.longName = [[[UIDevice currentDevice] systemName] UTF8String];
    osInfo.shortName = [[[UIDevice currentDevice] systemName] UTF8String];
    osInfo.build = [[[UIDevice currentDevice] systemVersion] UTF8String];

    return osInfo;
}


std::vector<DisplayInfo> IosDeviceInfoCollector::collectDisplayInfo() {
    DeviceInfo deviceInfo = collectDeviceInfo();
    NSNumber *scaleFactor = [NSNumber numberWithFloat:1.0f];
    scaleFactor = [NSNumber numberWithFloat:[[UIScreen mainScreen] nativeScale]];
    
    int x_res = [UIScreen mainScreen].fixedCoordinateSpace.bounds.size.width * [scaleFactor floatValue];
    int y_res = [UIScreen mainScreen].fixedCoordinateSpace.bounds.size.height * [scaleFactor floatValue];
    
    int ppi = detail::getPpi([NSString stringWithUTF8String:deviceInfo.attributes["id"].c_str()]);
    int x = x_res > y_res ? x_res : y_res;
    int y = x_res > y_res ? y_res : x_res;
    
    float inch = powf( powf(((float)x/(float)ppi), 2.0f) + powf(((float)y/(float)ppi), 2.0f), 0.5f);


    std::vector<DisplayInfo> displayInfoVector;
    DisplayInfo displayInfo;

    displayInfo.name = "Device Screen";
    displayInfo.diagonalInches = inch;
    displayInfo.widthPixels = x;
    displayInfo.heightPixels = y;
    displayInfo.xDpi = ppi;
    displayInfo.yDpi = ppi;
    displayInfo.attributes.insert(std::pair<std::string,std::string>("scale_factor", [[scaleFactor stringValue] UTF8String]));

    displayInfoVector.push_back(displayInfo);

    return displayInfoVector;
}


std::vector<CpuInfo> IosDeviceInfoCollector::collectCpuInfo() {
    std::vector<CpuInfo> cpuInfoVector;
    CpuInfo cpuInfo;

    cpuInfo.cores = (int)[NSProcessInfo processInfo].processorCount;

    cpuInfoVector.push_back(cpuInfo);
    return cpuInfoVector;
}


std::vector<GpuInfo> IosDeviceInfoCollector::collectGpuInfo() {
    std::vector<GpuInfo> gpuInfoVector;
    return gpuInfoVector;
}


MultiGpuInfo IosDeviceInfoCollector::collectMultiGpuInfo() {
    return MultiGpuInfo();
}


MemoryInfo IosDeviceInfoCollector::collectMemoryInfo() {
    MemoryInfo memoryInfo;
    memoryInfo.sizeBytes = [NSProcessInfo processInfo].physicalMemory;
    return memoryInfo;
}


std::vector<StorageInfo> IosDeviceInfoCollector::collectStorageInfo() {
    std::vector<StorageInfo> storageInfoVector;
    StorageInfo storageInfo;

    storageInfo.sizeBytes = detail::totalDiskSpace();
    storageInfo.isRemovable = false;

    storageInfoVector.push_back(storageInfo);
    return storageInfoVector;
}


std::vector<BatteryInfo> IosDeviceInfoCollector::collectBatteryInfo() {
    std::vector<BatteryInfo> batteryInfoVector;
    BatteryInfo batteryInfo;

    UIDeviceBatteryState bstate = [[UIDevice currentDevice] batteryState];
    batteryInfo.levelRatio = [[UIDevice currentDevice] batteryLevel];
    batteryInfo.isCharging = bstate == UIDeviceBatteryStateCharging;
    batteryInfo.isConnected = bstate == UIDeviceBatteryStateCharging || bstate == UIDeviceBatteryStateFull;

    batteryInfoVector.push_back(batteryInfo);
    return batteryInfoVector;
}


std::vector<CameraInfo> IosDeviceInfoCollector::collectCameraInfo() {
    std::vector<CameraInfo> cameraInfoVector;

    NSArray *devices = [[NSArray alloc] init];// [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    int i = 0;
    for (AVCaptureDevice *device in devices)
    {
        CameraInfo cameraInfo;
        
        int32_t xMaxPixels = 0;
        int32_t yMaxPixels = 0;
        
        // Format
        int32_t format_count = 0;
        for (AVCaptureDeviceFormat *format in device.formats) {
            NSString *formatName = [NSString stringWithFormat:@"format n%i", format_count];
            cameraInfo.attributes.insert(std::pair<std::string,std::string>([formatName UTF8String], [[format description] UTF8String]));
//            NSLog(@"%@ - format: %@", device.localizedName, format);
            
            long currentMaxDim = xMaxPixels * yMaxPixels;
            long newMaxDim = format.highResolutionStillImageDimensions.width * format.highResolutionStillImageDimensions.height;
            
            if(currentMaxDim < newMaxDim) {
                xMaxPixels = format.highResolutionStillImageDimensions.width;
                yMaxPixels = format.highResolutionStillImageDimensions.height;
            }
            
            format_count++;
        }
        
        long squarePixels = xMaxPixels * yMaxPixels;
        double megapixels = ceil(squarePixels / 1024.0 / 1024.0 * 10.0) / 10.0;
        
        
        cameraInfo.name = [device.localizedName UTF8String];
        cameraInfo.type = (device.position == AVCaptureDevicePositionBack) ? "CAMERA_TYPE_FRONT" : "CAMERA_TYPE_BACK";
        cameraInfo.pictureWidthPixels = xMaxPixels;
        cameraInfo.pictureHeightPixels = yMaxPixels;
        cameraInfo.pictureResolutionMP = megapixels;
        cameraInfo.videoWidthPixels = xMaxPixels;
        cameraInfo.videoHeightPixels = yMaxPixels;
        cameraInfo.videoResolutionMP = megapixels;
        cameraInfo.hasAutofocus = [device isFocusModeSupported:AVCaptureFocusModeAutoFocus];
        cameraInfo.hasFlash = [device hasFlash];
        cameraInfo.hasHdr = [device automaticallyAdjustsVideoHDREnabled] || [device isVideoHDREnabled],
        cameraInfo.hasTouchFocus = [device isFocusPointOfInterestSupported];
        
        
        // Auto Focus
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("focus_auto", [device isFocusModeSupported:AVCaptureFocusModeAutoFocus] ? "1" : "0"));
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("focus_continous_auto", [device isFocusModeSupported:AVCaptureFocusModeContinuousAutoFocus] ? "1" : "0"));
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("focus_locked", [device isFocusModeSupported:AVCaptureFocusModeLocked] ? "1" : "0"));
        
        // Exposure
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("exposure_auto", [device isExposureModeSupported:AVCaptureExposureModeAutoExpose] ? "1" : "0"));
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("exposure_continous_auto", [device isExposureModeSupported:AVCaptureExposureModeContinuousAutoExposure] ? "1" : "0"));
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("exposure_custom", [device isExposureModeSupported:AVCaptureExposureModeCustom] ? "1" : "0"));
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("exposure_locked", [device isExposureModeSupported:AVCaptureExposureModeLocked] ? "1" : "0"));
        
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("exposure_point_of_interest", [device isExposurePointOfInterestSupported] ? "1" : "0"));
        
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("white_balance_auto", [device isWhiteBalanceModeSupported:AVCaptureWhiteBalanceModeAutoWhiteBalance] ? "1" : "0"));
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("white_balance_continous_auto", [device isWhiteBalanceModeSupported:AVCaptureWhiteBalanceModeContinuousAutoWhiteBalance] ? "1" : "0"));
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("white_balance_locked", [device isWhiteBalanceModeSupported:AVCaptureWhiteBalanceModeLocked] ? "1" : "0"));
        
        // Flash
        //cameraInfo.attributes.insert(std::pair<std::string,std::string>("flash_auto", [device isFlashModeSupported:AVCaptureFlashModeAuto] ? "1" : "0"));
        //cameraInfo.attributes.insert(std::pair<std::string,std::string>("flash_off", [device isFlashModeSupported:AVCaptureFlashModeOff] ? "1" : "0"));
        //cameraInfo.attributes.insert(std::pair<std::string,std::string>("flash_on", [device isFlashModeSupported:AVCaptureFlashModeOn] ? "1" : "0"));
        
        // Torch
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("torch", [device hasTorch] ? "1" : "0"));
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("torch_auto", [device isTorchModeSupported:AVCaptureTorchModeAuto] ? "1" : "0"));
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("torch_on", [device isTorchModeSupported:AVCaptureTorchModeOff] ? "1" : "0"));
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("torch_off", [device isTorchModeSupported:AVCaptureTorchModeOn] ? "1" : "0"));
        
        // LowLight Boost
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("low_light_boost", [device isLowLightBoostSupported] ? "1" : "0"));
        
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("modelID", [[device modelID] UTF8String]));
        cameraInfo.attributes.insert(std::pair<std::string,std::string>("uniqueID", [[device uniqueID] UTF8String]));

        cameraInfoVector.push_back(cameraInfo);
        
        i++;
    }
    return cameraInfoVector;
}


FeatureInfo IosDeviceInfoCollector::collectFeatureInfo() {
    return FeatureInfo();
}


SensorInfo IosDeviceInfoCollector::collectSensorInfo() {
    return SensorInfo();
}


