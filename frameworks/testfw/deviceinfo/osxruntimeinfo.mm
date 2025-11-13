/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "osxruntimeinfo.h"

#import <Foundation/Foundation.h>
#import <IOKit/ps/IOPowerSources.h>
#import <IOKit/ps/IOPSKeys.h>



using namespace tfw;



int OsxRuntimeInfo::cpuCount()
{
    return 0;
}



double OsxRuntimeInfo::minCpuFrequencyMHz(int)
{
    return 0.0;
}



double OsxRuntimeInfo::maxCpuFrequencyMHz(int)
{
    return 0;
}



double OsxRuntimeInfo::currentCpuFrequencyMHz(int)
{
    return 0;
}



double OsxRuntimeInfo::currentGpuFrequencyMHz()
{
    return 0.0;
}



RuntimeInfo::BatteryStatus OsxRuntimeInfo::batteryStatus()
{
    CFTypeRef info;
    CFArrayRef list;
    CFDictionaryRef battery;
    
    info = IOPSCopyPowerSourcesInfo();
    if(info == NULL)
        return BATTERY_UNKNOWN;
    list = IOPSCopyPowerSourcesList(info);
    if(list == NULL) {
        CFRelease(info);
        return BATTERY_UNKNOWN;
    }
    
    bool isConnected = false;
    bool isCharging = false;
    if(CFArrayGetCount(list) && (battery = IOPSGetPowerSourceDescription(info, CFArrayGetValueAtIndex(list, 0))))
    {
        isConnected = [(NSString*)[(__bridge NSDictionary*)battery objectForKey:@kIOPSPowerSourceStateKey] isEqualToString:@kIOPSACPowerValue];
        isCharging = [[(__bridge NSDictionary*)battery objectForKey:@kIOPSIsChargingKey] boolValue];
    }
    
    CFRelease(list);
    CFRelease(info);
    
    if (isConnected) {
        if (isCharging) {
            return BATTERY_CHARGING;
        } else {
            return BATTERY_FULL;
        }
    } else {
        return BATTERY_DISCHARGING;
    }
}



double OsxRuntimeInfo::batteryLevelPercent()
{
    CFTypeRef info;
    CFArrayRef list;
    CFDictionaryRef battery;
    
    info = IOPSCopyPowerSourcesInfo();
    if(info == NULL)
        return 0.0;
    list = IOPSCopyPowerSourcesList(info);
    if(list == NULL) {
        CFRelease(info);
        return 0.0;
    }
    
    double capacity = 0.0;
    double maxCapacity = 1.0;
    if(CFArrayGetCount(list) && (battery = IOPSGetPowerSourceDescription(info, CFArrayGetValueAtIndex(list, 0))))
    {
        capacity = [[(__bridge NSDictionary*)battery objectForKey:@kIOPSCurrentCapacityKey] doubleValue];
        maxCapacity = [[(__bridge NSDictionary*)battery objectForKey:@kIOPSMaxCapacityKey] doubleValue];
    }
    
    CFRelease(list);
    CFRelease(info);
    
    return 100.0 * capacity / maxCapacity;
}



double OsxRuntimeInfo::batteryTemperatureCelsius()
{
    return 0.0;
}
