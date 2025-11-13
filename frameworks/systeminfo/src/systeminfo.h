/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include "deviceinfo.h"
#ifndef DISABLE_COLLECTORS
#include "glinfo.h"

#ifndef __APPLE__
#include "clinfo.h"
#include "cudainfo.h"
#endif

#include "metalinfo.h"
#ifdef WIN32
#include "directxinfo.h"
#endif
#include "vulkaninfo.h"
#endif

namespace sysinf
{

class SystemInfo
{
public:
	SystemInfo()
	{
		hasMultiGpuInfo = 0;
		hasFeatureInfo = 0;
		hasSensorInfo = 0;
		hasGL = 0;
		hasEGL = 0;
		hasGLES = 0;
		hasCl = 0;
		hasCuda = 0;
		hasMetal = 0;
		hasDirectx = 0;
		hasDirectx12 = 0;
		hasVulkan = 0;
	}

    DeviceInfo deviceInfo;
    OsInfo osInfo;
    std::vector<DisplayInfo> displayInfo;
    std::vector<CpuInfo> cpuInfo;
    std::vector<GpuInfo> gpuInfo;
    bool hasMultiGpuInfo;
    MultiGpuInfo multiGpuInfo;
    MemoryInfo memoryInfo;
    std::vector<StorageInfo> storageInfo;
    std::vector<BatteryInfo> batteryInfo;
    std::vector<CameraInfo> cameraInfo;
    bool hasFeatureInfo;
    FeatureInfo featureInfo;
    bool hasSensorInfo;
    SensorInfo sensorInfo;

#ifndef DISABLE_COLLECTORS
    GLESInfo glInfo;
    EGLInfo eglInfo;
    GLESInfo glesInfo;
#ifndef __APPLE__
    ClInfo clInfo;
    CudaInfo cudaInfo;
#endif
    MetalInfo metalInfo;
#ifdef WIN32
	DirectxInfo directxInfo;
	Directx12Info directx12Info;
#endif
	VulkanInfo vulkanInfo;
#endif

    bool hasGL;
    bool hasEGL;
    bool hasGLES;
    bool hasCl;
    bool hasCuda;
    bool hasMetal;
	bool hasDirectx;
	bool hasDirectx12;
	bool hasVulkan;

    template<class F> void applyVisitor(F visitor) const
    {
        visitor("device", deviceInfo);
        visitor("os", osInfo);
        visitor("display", displayInfo);
        visitor("cpu", cpuInfo);
        visitor("gpu", gpuInfo);
        if (hasMultiGpuInfo) {
            visitor("multiGpu", multiGpuInfo);
        }
        visitor("memory", memoryInfo);
        visitor("storage", storageInfo);
        visitor("battery", batteryInfo);
        visitor("camera", cameraInfo);

#ifndef DISABLE_COLLECTORS
#ifndef __APPLE__
        if (hasCl) {
            visitor("cl", clInfo);
        }

        if (hasCuda) {
            visitor("cuda", cudaInfo);
        }
#endif // __APPLE__
        if (hasEGL) {
            visitor("egl", eglInfo);
        }

        if (hasGLES) {
            visitor("gles", glesInfo);
        }

        if (hasGL) {
            visitor("gl", glInfo);
        }

		if (hasVulkan) {
			visitor("vulkan", vulkanInfo);
		}
#ifdef WIN32
		if (hasDirectx) {
			visitor("directx", directxInfo);
		}
		if (hasDirectx12) {
			visitor("directx12", directx12Info);
		}
#endif
        if (hasMetal) {
            visitor("metal", metalInfo);
        }
#endif
        if (hasFeatureInfo) {
            visitor("features", featureInfo);
        }

        if (hasSensorInfo) {
            visitor("sensor", sensorInfo);
        }
    }

};


} //namespace sysinf

#endif // SYSTEMINFO_H
