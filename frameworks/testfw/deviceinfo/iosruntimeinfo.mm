/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "iosruntimeinfo.h"

#import <UIKit/UIKit.h>



using namespace tfw;



int IosRuntimeInfo::cpuCount()
{
    return 0;
}



double IosRuntimeInfo::minCpuFrequencyMHz(int)
{
    return 0.0;
}



double IosRuntimeInfo::maxCpuFrequencyMHz(int)
{
    return 0;
}



double IosRuntimeInfo::currentCpuFrequencyMHz(int)
{
    return 0;
}



double IosRuntimeInfo::currentGpuFrequencyMHz()
{
    return 0.0;
}



RuntimeInfo::BatteryStatus IosRuntimeInfo::batteryStatus()
{
    UIDeviceBatteryState state = [[UIDevice currentDevice] batteryState];
    switch (state) {
    case UIDeviceBatteryStateUnknown:
        return BATTERY_UNKNOWN;
    case UIDeviceBatteryStateUnplugged:
        return BATTERY_DISCHARGING;
    case UIDeviceBatteryStateCharging:
        return BATTERY_CHARGING;
    case UIDeviceBatteryStateFull:
        return BATTERY_FULL;
    default:
        return BATTERY_UNKNOWN;
    }
}



double IosRuntimeInfo::batteryLevelPercent()
{
    return 100.0 * [[UIDevice currentDevice] batteryLevel];
}



double IosRuntimeInfo::batteryTemperatureCelsius()
{
    return 0.0;
}
