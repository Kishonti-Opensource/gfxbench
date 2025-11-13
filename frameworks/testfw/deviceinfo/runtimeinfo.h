/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RUNTIMEINFO_H
#define RUNTIMEINFO_H



namespace tfw
{



class RuntimeInfo
{
public:
    enum BatteryStatus {
        BATTERY_CHARGING,
        BATTERY_DISCHARGING,
        BATTERY_NOT_CHARGING,
        BATTERY_FULL,
        BATTERY_UNKNOWN
    };
    
    virtual ~RuntimeInfo() {}
    
    virtual int cpuCount() = 0;
    virtual double minCpuFrequencyMHz(int cpuIndex) = 0;
    virtual double maxCpuFrequencyMHz(int cpuIndex) = 0;
    virtual double currentCpuFrequencyMHz(int cpuIndex) = 0;
    
    virtual double currentGpuFrequencyMHz() = 0;
    
    virtual BatteryStatus batteryStatus() = 0;
    virtual double batteryLevelPercent() = 0;
    virtual double batteryTemperatureCelsius() = 0;
};



}



#endif // RUNTIMEINFO_H
