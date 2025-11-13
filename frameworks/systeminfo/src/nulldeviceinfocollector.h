/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NULLDEVICEINFOCOLLECTOR_H
#define NULLDEVICEINFOCOLLECTOR_H

#include "deviceinfocollector.h"

namespace sysinf
{



class NullDeviceInfoCollector: public DeviceInfoCollector
{
public:
    DeviceInfo collectDeviceInfo() override {
        return DeviceInfo();
    }
    OsInfo collectOsInfo() override {
        return OsInfo();
    }
    std::vector<DisplayInfo> collectDisplayInfo() override {
        return std::vector<DisplayInfo>();
    }
    std::vector<CpuInfo> collectCpuInfo() override {
        return std::vector<CpuInfo>();
    }
    std::vector<GpuInfo> collectGpuInfo() override {
        return std::vector<GpuInfo>();
    }
    MultiGpuInfo collectMultiGpuInfo() override {
        return MultiGpuInfo();
    }
    MemoryInfo collectMemoryInfo() override {
        return MemoryInfo();
    }
    std::vector<StorageInfo> collectStorageInfo() override {
        return std::vector<StorageInfo>();
    }
    std::vector<BatteryInfo> collectBatteryInfo() override {
        return std::vector<BatteryInfo>();
    }
    std::vector<CameraInfo> collectCameraInfo() override {
        return std::vector<CameraInfo>();
    }
    FeatureInfo collectFeatureInfo() override  {
        return FeatureInfo();
    }
    SensorInfo collectSensorInfo() override {
        return SensorInfo();
    }
};



}

#endif // NULLDEVICEINFOCOLLECTOR_H

