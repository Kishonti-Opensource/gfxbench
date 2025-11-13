/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CLINFO_H
#define CLINFO_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace sysinf
{



class CLImageFormat
{
public:
    std::string mem_flags;
    std::string mem_object_type;
    std::string image_channel_order;
    std::string image_channel_data_type;
    
    template<class F> void applyVisitor(F visitor) const {
        visitor("mem_flags", mem_flags);
        visitor("mem_object_type", mem_object_type);
        visitor("channel_order", image_channel_order);
        visitor("channel_data_type", image_channel_data_type);
    }
};



class CLDeviceInfo
{
public:
    CLDeviceInfo() {
        for (int i = 0; i < 3; ++i) {
            maxWorkItemSizes[i] = 0;
        }
    }
    std::string name;
    std::string vendor;
    std::string type;
    std::string driverVersion;
    std::string profile;
    std::string version;
    std::string openClCVersion;
    std::string error;
    size_t maxWorkItemSizes[3];
    std::vector<CLImageFormat> imageFormats;
    std::map<std::string, std::string> stringAttributes;
    std::map<std::string, std::vector<std::string> > arrayAttributes;
    std::map<std::string, uint64_t> integerAttributes;
    
    template<class F> void applyVisitor(F visitor) const {
        visitor("major", name);
        visitor("minor", version);

        visitor("CL_DEVICE_NAME", name);
        visitor("CL_DEVICE_VENDOR", vendor);
        visitor("CL_DEVICE_TYPE", type);
        visitor("CL_DRIVER_VERSION", driverVersion);
        visitor("CL_DEVICE_PROFILE", profile);
        visitor("CL_DEVICE_VERSION", version);
        visitor("CL_DEVICE_OPENCL_C_VERSION", openClCVersion);
        visitor("CL_DEVICE_MAX_WORK_ITEM_SIZES",
                std::vector<size_t>(maxWorkItemSizes, maxWorkItemSizes + 3));
        visitor("image_formats", imageFormats);
        if (!error.empty()) {
            visitor("error", error);
        }
        
        for (auto i = stringAttributes.begin(); i != stringAttributes.end(); ++i) {
            visitor(i->first, i->second);
        }
        for (auto i = arrayAttributes.begin(); i != arrayAttributes.end(); ++i) {
            visitor(i->first, i->second);
        }
        for (auto i = integerAttributes.begin(); i != integerAttributes.end(); ++i) {
            visitor(i->first, i->second);
        }
    }
};



class CLPlatformInfo
{
public:
    std::string vendor;
    std::string name;
    std::string version;
    std::string profile;

    std::vector<std::string> extensions;
    std::vector<CLDeviceInfo> devices;

	int clMajor;
	int clMinor;
    
    template<class F> void applyVisitor(F visitor) const {
        visitor("CL_PLATFORM_VENDOR", vendor);
        visitor("CL_PLATFORM_NAME", name);
        visitor("CL_PLATFORM_VERSION", version);
        visitor("CL_PLATFORM_PROFILE", profile);
        visitor("CL_PLATFORM_EXTENSIONS", extensions);
		visitor("CL_PLATFORM_MAJOR", clMajor);
		visitor("CL_PLATFORM_MINOR", clMinor);
        visitor("devices", devices);
    }
};



class ClInfo
{
public:
    std::string driverPath;
    std::vector<CLPlatformInfo> platforms;
    
    template<class F> void applyVisitor(F visitor) const {
        visitor("driverPath", driverPath);
        visitor("platforms", platforms);
    }
};



}

#endif  // CLINFO_H
