/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DEVICEINFOCOLLECTOR_H
#define DEVICEINFOCOLLECTOR_H

#include <vector>
#include "systeminfo.h"

namespace sysinf
{

class DeviceInfoCollector
{
public:
    virtual ~DeviceInfoCollector() {}

    void collectAll(SystemInfo& systemInfo) {
        systemInfo.deviceInfo = collectDeviceInfo();
        systemInfo.osInfo = collectOsInfo();
        systemInfo.displayInfo = collectDisplayInfo();
        systemInfo.cpuInfo = collectCpuInfo();
        systemInfo.gpuInfo = collectGpuInfo();
        systemInfo.multiGpuInfo = collectMultiGpuInfo();
        systemInfo.memoryInfo = collectMemoryInfo();
        systemInfo.storageInfo = collectStorageInfo();
        systemInfo.batteryInfo = collectBatteryInfo();
        systemInfo.cameraInfo = collectCameraInfo();
        systemInfo.featureInfo = collectFeatureInfo();
        systemInfo.sensorInfo = collectSensorInfo();
        
        systemInfo.hasMultiGpuInfo = systemInfo.multiGpuInfo.isEnabled;
        systemInfo.hasFeatureInfo = systemInfo.featureInfo.features.size() > 0;
        systemInfo.hasSensorInfo = systemInfo.sensorInfo.sensors.size() > 0;
    }

    virtual DeviceInfo collectDeviceInfo() = 0;
    virtual OsInfo collectOsInfo() = 0;
    virtual std::vector<DisplayInfo> collectDisplayInfo() = 0;
    virtual std::vector<CpuInfo> collectCpuInfo() = 0;
    virtual std::vector<GpuInfo> collectGpuInfo() = 0;
    virtual MultiGpuInfo collectMultiGpuInfo() = 0;
    virtual MemoryInfo collectMemoryInfo() = 0;
    virtual std::vector<StorageInfo> collectStorageInfo() = 0;
    virtual std::vector<BatteryInfo> collectBatteryInfo() = 0;
    virtual std::vector<CameraInfo> collectCameraInfo() = 0;
    virtual FeatureInfo collectFeatureInfo() = 0;
    virtual SensorInfo collectSensorInfo() = 0;
};



}

#endif // DEVICEINFOCOLLECTOR_H
