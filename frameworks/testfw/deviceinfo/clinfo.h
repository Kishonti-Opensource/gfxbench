/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CLINFO_H_
#define CLINFO_H_

#include <cstdint>
#include <string>
#include <vector>

namespace tfw
{

class CLImageFormat
{
public:
    std::string mem_flags;
    std::string mem_object_type;
    std::string image_channel_order;
    std::string image_channel_data_type;
};


template<class F> void applyVisitor(const CLImageFormat& fmt, F visitor) {
    visitor("mem_flags", fmt.mem_flags);
    visitor("mem_object_type", fmt.mem_object_type);
    visitor("channel_order", fmt.image_channel_order);
    visitor("channel_data_type", fmt.image_channel_data_type);
}


class CLDeviceInfo
{
public:
    CLDeviceInfo();
    std::string vendor;
    std::string name;
    std::string version;
    std::string openclCVersion;
    std::string driverVersion;
    std::vector<std::string> extensions;
    bool compilerAvailable;
    uint32_t deviceAddressBits;
    std::string deviceType;
    uint32_t maxComputeUnits;
    uint32_t maxClockFrequency;
    uint64_t deviceGlobalMemSize;
    uint64_t maxMemAllocSize;
    std::string globalMemCacheType;
    uint64_t globalMemCacheSize;
    uint64_t globalMemCachelineSize;
    std::string localMemType;
    uint64_t localMemSize;
    uint64_t maxConstantBufferSize;
    uint32_t maxWorkItemDimensions;
    size_t maxWorkItemSizes[3];
    size_t maxWorkGorupSize;
    bool hasImageSupport;
    size_t image2DMaxSize[2];
    size_t image3DMaxSize[3];
    uint32_t maxReadImageArgs;
    uint32_t maxWriteImageArgs;
    uint32_t maxSamplers;
    std::vector<CLImageFormat> imageFormats;
    std::string error;
};

template<class F> void applyVisitor(const CLDeviceInfo& info, F visitor) {
    visitor("CL_DEVICE_VENDOR", info.vendor);
    visitor("CL_DEVICE_NAME", info.name);
    visitor("CL_DEVICE_VERSION", info.version);
    visitor("CL_DEVICE_OPENCL_C_VERSION", info.openclCVersion);
    visitor("CL_DEVICE_DRIVER_VERSION", info.driverVersion);
    visitor("CL_DEVICE_EXTENSIONS", info.extensions);
    visitor("CL_DEVICE_COMPILER_AVAILABLE", info.compilerAvailable);
    visitor("CL_DEVICE_ADDRESS_BITS", info.deviceAddressBits);
    visitor("CL_DEVICE_TYPE", info.deviceType);
    visitor("CL_DEVICE_MAX_COMPUTE_UNITS", info.maxComputeUnits);
    visitor("CL_DEVICE_MAX_CLOCK_FREQUENCY", info.maxClockFrequency);
    visitor("CL_DEVICE_GLOBAL_MEM_SIZE", info.globalMemCacheSize);
    visitor("CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE", info.globalMemCachelineSize);
    visitor("CL_DEVICE_MAX_MEM_ALLOC_SIZE", info.maxMemAllocSize);
    visitor("CL_DEVICE_GLOBAL_MEM_CACHE_TYPE", info.globalMemCacheType);
    visitor("CL_DEVICE_LOCAL_MEM_TYPE", info.localMemType);
    visitor("CL_DEVICE_LOCAL_MEM_SIZE", info.localMemSize);
    visitor("CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE", info.maxConstantBufferSize);
    visitor("CL_DEVICE_MAX_WORK_ITEM_SIZES",
            std::vector<size_t>(info.maxWorkItemSizes, info.maxWorkItemSizes + 3));
    visitor("CL_DEVICE_MAX_WORK_GROUP_SIZE", info.maxWorkGorupSize);

    visitor("CL_DEVICE_IMAGE_SUPPORT", info.hasImageSupport);
    visitor("CL_DEVICE_IMAGE2D_MAX_WIDTH", info.image2DMaxSize[0]);
    visitor("CL_DEVICE_IMAGE2D_MAX_HEIGHT", info.image2DMaxSize[1]);
    visitor("CL_DEVICE_IMAGE3D_MAX_WIDTH", info.image3DMaxSize[0]);
    visitor("CL_DEVICE_IMAGE3D_MAX_HEIGHT", info.image3DMaxSize[1]);
    visitor("CL_DEVICE_IMAGE3D_MAX_DEPTH", info.image3DMaxSize[2]);
    visitor("CL_DEVICE_MAX_READ_IMAGE_ARGS", info.maxReadImageArgs);
    visitor("CL_DEVICE_MAX_WRITE_IMAGE_ARGS", info.maxWriteImageArgs);
    visitor("CL_DEVICE_MAX_SAMPLERS", info.maxSamplers);
    visitor("image_formats", info.imageFormats);
    if (!info.error.empty())
    {
        visitor("error", info.error);
    }
}



class CLPlatformInfo
{
public:
    CLPlatformInfo();
    std::string vendor;
    std::string name;
    std::string version;
    std::string profile;
    std::vector<std::string> extensions;
    std::vector<CLDeviceInfo> devices;
};

template<class F> void applyVisitor(const CLPlatformInfo& info, F visitor) {
    visitor("CL_PLATFORM_VENDOR", info.vendor);
    visitor("CL_PLATFORM_NAME", info.name);
    visitor("CL_PLATFORM_VERSION", info.version);
    visitor("CL_PLATFORM_PROFILE", info.profile);
    visitor("CL_PLATFORM_EXTENSIONS", info.extensions);
    visitor("devices", info.devices);
}



class CLInfoCollector
{
public:
    CLInfoCollector();
    void collect();

    bool isOpenCLAvailable() const;
    size_t platformCount() const;
    const CLPlatformInfo &platform(int i) const;
    std::string serialize() const;
    std::string driverPath() const;

private:
    std::string driverPath_;
    std::vector<CLPlatformInfo> platforms_;
};

}

#endif  // CLINFO_H_
