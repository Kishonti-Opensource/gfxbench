/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "windowsruntimeinfo.h"

#include <Windows.h>
#include <PowrProf.h>

#include <vector>



using namespace tfw;


/*
 * Note that this structure definition was accidentally omitted from WinNT.h. This error will be
 * corrected in the future. In the meantime, to compile your application, include the structure
 * definition contained in this topic in your source code.
 * https://msdn.microsoft.com/en-us/library/windows/desktop/aa373184.aspx
 */
typedef struct _PROCESSOR_POWER_INFORMATION {
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;



int WindowsRuntimeInfo::cpuCount()
{
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    return systemInfo.dwNumberOfProcessors;
}



double WindowsRuntimeInfo::minCpuFrequencyMHz(int)
{
    return 0.0;
}



double WindowsRuntimeInfo::maxCpuFrequencyMHz(int cpuIndex)
{
    std::vector<PROCESSOR_POWER_INFORMATION> processors(cpuCount());
    CallNtPowerInformation(ProcessorInformation, NULL, 0,
            &processors[0], static_cast<ULONG>(processors.size() * sizeof(PROCESSOR_POWER_INFORMATION)));
    return processors.at(cpuIndex).MaxMhz;
}



double WindowsRuntimeInfo::currentCpuFrequencyMHz(int cpuIndex)
{
    std::vector<PROCESSOR_POWER_INFORMATION> processors(cpuCount());
    CallNtPowerInformation(ProcessorInformation, NULL, 0,
            &processors[0], static_cast<ULONG>(processors.size() * sizeof(PROCESSOR_POWER_INFORMATION)));
    return processors.at(cpuIndex).CurrentMhz;
}



double WindowsRuntimeInfo::currentGpuFrequencyMHz()
{
    return 0.0;
}



RuntimeInfo::BatteryStatus WindowsRuntimeInfo::batteryStatus()
{
    SYSTEM_BATTERY_STATE batteryState = {};
    CallNtPowerInformation(SystemBatteryState, NULL, 0, &batteryState, sizeof(batteryState));
    return batteryState.Discharging ? BATTERY_DISCHARGING : BATTERY_CHARGING;
}



double WindowsRuntimeInfo::batteryLevelPercent()
{
    SYSTEM_BATTERY_STATE batteryState = {};
    CallNtPowerInformation(SystemBatteryState, NULL, 0, &batteryState, sizeof(batteryState));
    if (batteryState.MaxCapacity <= 0) {
        return 0.0;
    }
    return 100.0 * batteryState.RemainingCapacity / batteryState.MaxCapacity;
}



double WindowsRuntimeInfo::batteryTemperatureCelsius()
{
    return 0.0;
}
