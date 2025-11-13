/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef IOSDEVICEINFOCOLLECTOR_H
#define IOSDEVICEINFOCOLLECTOR_H

#include "deviceinfocollector.h"

namespace sysinf
{



class IosDeviceInfoCollector: public DeviceInfoCollector
{
public:
    DeviceInfo collectDeviceInfo() override;
    OsInfo collectOsInfo() override;
    std::vector<DisplayInfo> collectDisplayInfo() override;
    std::vector<CpuInfo> collectCpuInfo() override;
    std::vector<GpuInfo> collectGpuInfo() override;
    MultiGpuInfo collectMultiGpuInfo() override;
    MemoryInfo collectMemoryInfo() override;
    std::vector<StorageInfo> collectStorageInfo() override;
    std::vector<BatteryInfo> collectBatteryInfo() override;
    std::vector<CameraInfo> collectCameraInfo() override;
    FeatureInfo collectFeatureInfo() override;
    SensorInfo collectSensorInfo() override;
};



}

#endif // IOSDEVICEINFOCOLLECTOR_H
