/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef OSXRUNTIMEINFO_H
#define OSXRUNTIMEINFO_H

#include "runtimeinfo.h"



namespace tfw
{



class OsxRuntimeInfo: public RuntimeInfo
{
public:
    virtual int cpuCount();
    virtual double minCpuFrequencyMHz(int cpuIndex);
    virtual double maxCpuFrequencyMHz(int cpuIndex);
    virtual double currentCpuFrequencyMHz(int cpuIndex);
    
    virtual double currentGpuFrequencyMHz();
    
    virtual BatteryStatus batteryStatus();
    virtual double batteryLevelPercent();
    virtual double batteryTemperatureCelsius();
};



}



#endif // OSXRUNTIMEINFO_H
