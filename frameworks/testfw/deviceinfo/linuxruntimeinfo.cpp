/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "linuxruntimeinfo.h"

#include <cctype>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <sstream>



using namespace tfw;



enum GpuType {
    GpuTypeUnknown = -1,
    Mali400 = 0,
    PVR,
    Adreno,
    Vivante,
    Tegra,
    MaliT6xx,
    GpuTypeMax
};

const char* const GPU_MODULES[] = {
    "/sys/module/mali/parameters/mali_gpu_clk",
    "/sys/module/pvrsrvkm/parameters/sgx_gpu_clk",
    "/sys/class/kgsl/kgsl-3d0/gpuclk",
    "/sys/module/galcore/parameters/gpuClock",
    "/sys/kernel/tegra_gpu/gpu_rate",
    "/sys/class/misc/mali0/device/clock",
};

GpuType getGpuType()
{
    for (int i = 0; i< GpuTypeMax; ++i){
        FILE* f = fopen(GPU_MODULES[i], "r");
        if (f) {
            fclose(f);
            return (GpuType)i;
        }
    }
    return GpuTypeUnknown;
}

int parseMali()
{
    int mhz = -1;    
    FILE* f = fopen(GPU_MODULES[MaliT6xx], "r");
    if (f) {
        char chars[255];
        char* res = fgets(chars, 255, f);
        if (res != 0) {
            fclose(f);
            return mhz;
        }
        sscanf(chars,"Current sclk_g3d[G3D_BLK] = %dMhz", &mhz);
        if (mhz < 0){
            sscanf(chars, "%d", &mhz);
        }
        fclose(f);
    }
    return mhz;
}



int readInt(const char *filename)
{
    int result = -1;
    FILE* f = fopen(filename, "r");
    if (f) {
        int res = fscanf(f, "%d", &result);
        if (res != EOF) {
            fclose(f);
            return result;
        }
        fclose(f);
    }
    return result;
}



class LinuxRuntimeInfo::Private
{
public:
    int gpuType;
};



LinuxRuntimeInfo::LinuxRuntimeInfo():
    d(new Private)
{
    d->gpuType = getGpuType();
}



LinuxRuntimeInfo::~LinuxRuntimeInfo()
{}



int LinuxRuntimeInfo::cpuCount()
{
    std::string presentString;
    std::ifstream file("/sys/devices/system/cpu/present");
    std::getline(file, presentString);
    for (size_t i = 0; i < presentString.size(); ++i) {
        if (!std::isdigit(presentString[i])) {
            presentString[i] = ' ';
        }
    }
    std::istringstream iss(presentString);
    int maximum = -1;
    while(iss.good()) {
        int value;
        iss >> value;
        maximum = std::max(maximum, value);
    }
    return maximum + 1;
}



double LinuxRuntimeInfo::minCpuFrequencyMHz(int cpuIndex)
{
    char name[64];
    sprintf(name, "/sys/devices/system/cpu/cpu%i/cpufreq/cpuinfo_min_freq", cpuIndex);
    return readInt(name);
}



double LinuxRuntimeInfo::maxCpuFrequencyMHz(int cpuIndex)
{
    char name[64];
    sprintf(name, "/sys/devices/system/cpu/cpu%i/cpufreq/cpuinfo_max_freq", cpuIndex);
    return readInt(name);
}



double LinuxRuntimeInfo::currentCpuFrequencyMHz(int cpuIndex)
{
    char name[64];
    sprintf(name, "/sys/devices/system/cpu/cpu%i/cpufreq/scaling_cur_freq", cpuIndex);
    return readInt(name) / 1000.0;
}



double LinuxRuntimeInfo::currentGpuFrequencyMHz()
{
    switch (d->gpuType) {
    case Mali400:
    case PVR:
        return readInt(GPU_MODULES[d->gpuType]);
    case Adreno:
    case Vivante:
    case Tegra:
        return readInt(GPU_MODULES[d->gpuType]) / 1000000.0;
    case MaliT6xx:
        return parseMali();
    default:
        return -1.0;
    }
}



RuntimeInfo::BatteryStatus LinuxRuntimeInfo::batteryStatus()
{
    std::string statusString;
    std::ifstream file("/sys/class/power_supply/battery/status");
    std::getline(file, statusString);
    
    switch (statusString[0]) {
    case 'C':
        return BATTERY_CHARGING;
    case 'D':
        return BATTERY_DISCHARGING;
    case 'N':
        return BATTERY_NOT_CHARGING;
    case 'F': // Full
        return BATTERY_FULL;
    default:
        return BATTERY_UNKNOWN;
    }
}



double LinuxRuntimeInfo::batteryLevelPercent()
{
    return readInt("/sys/class/power_supply/battery/capacity");
}



double LinuxRuntimeInfo::batteryTemperatureCelsius()
{
    return readInt("/sys/class/power_supply/battery/temp") / 10.0;
}

