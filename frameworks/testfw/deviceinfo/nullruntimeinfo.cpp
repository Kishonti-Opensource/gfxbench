/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "nullruntimeinfo.h"



using namespace tfw;



int NullRuntimeInfo::cpuCount()
{
    return 0;
}



double NullRuntimeInfo::minCpuFrequencyMHz(int)
{
    return 0.0;
}



double NullRuntimeInfo::maxCpuFrequencyMHz(int)
{
    return 0.0;
}



double NullRuntimeInfo::currentCpuFrequencyMHz(int)
{
    return 0.0;
}



double NullRuntimeInfo::currentGpuFrequencyMHz()
{
    return 0.0;
}



RuntimeInfo::BatteryStatus NullRuntimeInfo::batteryStatus()
{
    return BATTERY_DISCHARGING;
}



double NullRuntimeInfo::batteryLevelPercent()
{
    return 0.0;
}



double NullRuntimeInfo::batteryTemperatureCelsius()
{
    return 0.0;
}
