/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "systeminfogateway.h"

#include "SystemInfoCommonKeys.h"
#include "dataformatter.h"
#include "systeminfo.h"
#include "deviceinfocollector.h"
#include "glinfocollector.h"
#include "metalinfocollector.h"
#include "vulkaninfocollector.h"

#ifndef __APPLE__
#include "clinfocollector.h"
#include "cudainfocollector.h"
#endif

#include "keyvaluevisitor.h"

#if defined(_WIN32)
    #include "windowsdeviceinfocollector.h"
    #include "directxinfocollector.h"
#elif defined(__APPLE__)
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
        #include "iosdeviceinfocollector.h"
    #else
        #include "osxdeviceinfocollector.h"
    #endif
#elif defined(__ANDROID__)
    #include "androiddeviceinfocollector.h"
#elif defined(__linux__)
    #include "linuxdeviceinfocollector.h"
#else
    #include "nulldeviceinfocollector.h"
#endif

#include <algorithm>
#include <sstream>
#include <string>



class AttributeVisitor
{
public:
    AttributeVisitor(SystemInfoItem& item):
        mItem(&item)
    {}
    void operator()(const std::string& name, const std::string& value) {
        if (name == "major") {
            mItem->setMajor(value);
        } else if (name == "minor") {
            mItem->setMinor(value);
        } else {
            mItem->addAttribute(name, Variant(value));
        }
    }
    template<class T> void operator()(const std::string& name, const T& value) {
        if (name == "major") {
            std::ostringstream oss;
            oss << value;
            mItem->setMajor(oss.str());
        } else if (name == "minor") {
            std::ostringstream oss;
            oss << value;
            mItem->setMinor(oss.str());
        } else {
            mItem->addAttribute(name, Variant(value));
        }
    }
    template<class T> void operator()(const std::string& name, const std::vector<T>& value) {
        std::ostringstream oss;
        if (!value.empty()) {
            oss << value.front();
        }
        for (size_t i = 1; i < value.size(); ++i) {
            oss << '\n' << value.at(i);
        }
        if (name == "major") {
            mItem->setMajor(oss.str());
        } else if (name == "minor") {
            mItem->setMinor(oss.str());
        } else {
            mItem->addAttribute(name, oss.str());
        }
    }
#ifndef __APPLE__
    void operator()(const std::string&, const std::vector<sysinf::CLImageFormat>&) {}
    void operator()(const std::string&, const std::vector<sysinf::CLDeviceInfo>&) {}
#endif
    void operator()(const std::string&, const std::vector<sysinf::EGLConfigInfo>&) {}
private:
    SystemInfoItem* mItem;
};



class ItemVisitor
{
public:
    ItemVisitor(std::vector<SystemInfoItem>& items, const std::string& imagePath):
        mItems(&items),
        mImagePath(imagePath)
    {}
    template<class T> void operator()(const std::string& name, const T& value) {
        SystemInfoItem item;
        item.setName(name);
        value.applyVisitor(AttributeVisitor(item));
        item.setRowId(mItems->size());
        item.setImagePath(mImagePath);
        mItems->push_back(item);
    }
    template<class T> void operator()(const std::string& name, const std::vector<T>& value) {
        for (auto i = value.begin(); i != value.end(); ++i) {
            operator()(name, *i);
        }
    }
    void operator()(const std::string& name, const std::vector<sysinf::CameraInfo>& value) {
        for (auto i = value.begin(); i != value.end(); ++i) {
            SystemInfoItem item;
            item.setName((i->type == "CAMERA_TYPE_FRONT") ? "frontCamera" : "backCamera");
            i->applyVisitor(AttributeVisitor(item));
            item.setMajor("");
            item.setRowId(mItems->size());
            item.setImagePath(mImagePath);
            mItems->push_back(item);
        }
    }
    void operator()(const std::string&, const sysinf::BatteryInfo& batteryInfo) {
        SystemInfoItem item;
        item.setRowId(mItems->size());
        item.setImagePath(mImagePath);
        item.setName("battery");
        item.setMajor(batteryInfo.technology);
        std::ostringstream oss;
        oss << (batteryInfo.levelRatio * 100) << "%, ";
        oss << (batteryInfo.isConnected ? "connected" : "disconnected");
        item.setMinor(oss.str());
        batteryInfo.applyVisitor(AttributeVisitor(item));
        mItems->push_back(item);
    }
	void operator()(const std::string&, sysinf::VulkanInfo vulkanInfo) {
		operator()("Vulkan device", vulkanInfo.devices);
	}
#ifdef WIN32
	void operator()(const std::string&, sysinf::DirectxInfo directxInfo) {
		operator()("DirectX 11 device", directxInfo.devices);
	}
	void operator()(const std::string&, sysinf::Directx12Info directx12Info) {
		operator()("DirectX 12 device", directx12Info.dx12_devices);
	}
#endif
    void operator()(const std::string&, sysinf::MetalInfo metalInfo) {
        operator()("Metal device", metalInfo.devices);
    }
    
#ifndef __APPLE__
    void operator()(const std::string&, sysinf::CudaInfo cudaInfo) {
        operator()("Cuda device", cudaInfo.devices);
    }

    void operator()(const std::string&, sysinf::ClInfo clInfo) {
        for (size_t i = 0; i < clInfo.platforms.size(); ++i) {
            sysinf::CLPlatformInfo platformInfo = clInfo.platforms.at(i);
            for (size_t j = 0; j < platformInfo.devices.size(); ++j) {
                const sysinf::CLDeviceInfo& deviceInfo = platformInfo.devices.at(j);
                SystemInfoItem item;
                item.setName("OpenCL device");
                item.setMajor(deviceInfo.name);
                item.setMinor(platformInfo.name);
                platformInfo.applyVisitor(AttributeVisitor(item));
                deviceInfo.applyVisitor(AttributeVisitor(item));
                item.setRowId(mItems->size());
                item.setImagePath(mImagePath);
                mItems->push_back(item);
            }
        }
    }
#endif
private:
    std::vector<SystemInfoItem>* mItems;
    std::string mImagePath;
};



class SystemInfoGateway::Private
{
public:
    sysinf::SystemInfo systemInfo;
};



SystemInfoGateway::SystemInfoGateway():
    d(new Private)
{}



SystemInfoGateway::~SystemInfoGateway()
{}



void SystemInfoGateway::collectSystemInfo()
{
#if defined(_WIN32)
    sysinf::WindowsDeviceInfoCollector deviceInfoCollector;
    sysinf::collectDirectxInfo(d->systemInfo);

#elif defined(__APPLE__)
    #if TARGET_OS_IPHONE
        sysinf::IosDeviceInfoCollector deviceInfoCollector;
    #else
        sysinf::OsxDeviceInfoCollector deviceInfoCollector;
    #endif
#elif defined(__ANDROID__)
    sysinf::AndroidDeviceInfoCollector deviceInfoCollector;
#elif defined(__linux__)
    sysinf::LinuxDeviceInfoCollector deviceInfoCollector;
#else
    sysinf::NullDeviceInfoCollector deviceInfoCollector;
#endif

    deviceInfoCollector.collectAll(d->systemInfo);
    sysinf::collectMetalInfo(d->systemInfo);
#ifndef __APPLE__
    sysinf::collectClInfo(d->systemInfo);
    sysinf::collectCudaInfo(d->systemInfo);
    sysinf::collectVulkanInfo(d->systemInfo);
#endif

#if defined(__APPLE__) && !TARGET_OS_IPHONE
    // On macOS disable GL info collector to avoid crash BUT GLinfo is required for community version on iOS
#else
    sysinf::collectGlInfo(d->systemInfo);
#endif
}



sysinf::SystemInfo SystemInfoGateway::getSystemInfo() const
{
    return d->systemInfo;
}



namespace {
    std::string getLastComponent(const std::string& s) {
        std::size_t found = s.rfind("/");
        if (found != std::string::npos)
            return s.substr(found + 1);

        return s;
    }
}



void SystemInfoGateway::updateSystemInfo(const sysinf::Properties& properties) {
    std::ostringstream indexedBase;

    d->systemInfo.deviceInfo.beautified    = true;
    d->systemInfo.deviceInfo.major         = properties.getString("device/major", d->systemInfo.deviceInfo.major);
    d->systemInfo.deviceInfo.minor         = properties.getString("device/minor", d->systemInfo.deviceInfo.minor);
    d->systemInfo.deviceInfo.name          = properties.getString("device/name", d->systemInfo.deviceInfo.name);
    d->systemInfo.deviceInfo.manufacturer  = properties.getString("device/manufacturer", d->systemInfo.deviceInfo.manufacturer);
    d->systemInfo.deviceInfo.chipset       = properties.getString("device/chipset", d->systemInfo.deviceInfo.chipset);
    d->systemInfo.deviceInfo.image         = properties.getString("device/image", d->systemInfo.deviceInfo.image);
    for(sysinf::PropertyIter iter = properties.groupIterator("device"); !iter.done(); iter.next())
    {
        std::string name = getLastComponent(iter.name());
        if(name != "name" &&
           name != "manufacturer" &&
           name != "chipset" &&
           name != "image") {
            d->systemInfo.deviceInfo.attributes.insert( std::pair<std::string,std::string>(name, iter.value().getString()) );
        }
    }


    d->systemInfo.osInfo.beautified        = true;
    d->systemInfo.osInfo.major             = properties.getString("os/major", d->systemInfo.osInfo.major);
    d->systemInfo.osInfo.minor             = properties.getString("os/minor", d->systemInfo.osInfo.minor);
    d->systemInfo.osInfo.name              = properties.getString("os/name", d->systemInfo.osInfo.name);
    d->systemInfo.osInfo.longName          = properties.getString("os/long", d->systemInfo.osInfo.longName);
    d->systemInfo.osInfo.shortName         = properties.getString("os/short", d->systemInfo.osInfo.shortName);
    d->systemInfo.osInfo.fingerprint       = properties.getString("os/fingerprint", d->systemInfo.osInfo.fingerprint);
    d->systemInfo.osInfo.arch              = properties.getString("os/arch", d->systemInfo.osInfo.arch);
    d->systemInfo.osInfo.build             = properties.getString("os/build", d->systemInfo.osInfo.build);
    for(sysinf::PropertyIter iter = properties.groupIterator("os"); !iter.done(); iter.next())
    {
        std::string name = getLastComponent(iter.name());
        if(name != "name" &&
           name != "long" &&
           name != "short" &&
           name != "fingerprint" &&
           name != "arch" &&
           name != "build") {
            d->systemInfo.osInfo.attributes.insert( std::pair<std::string,std::string>(name, iter.value().getString()) );
        }
    }


    size_t displayCount = static_cast<size_t>(properties.getInt("display/count"));
    for (size_t i = d->systemInfo.displayInfo.size(); i < displayCount; ++i) {
        d->systemInfo.displayInfo.push_back(sysinf::DisplayInfo());
    }
    for (size_t i = 0; i < displayCount; ++i) {
        indexedBase.str(std::string());
        indexedBase << "display/" << i << "/";

        d->systemInfo.displayInfo.at(i).beautified    = true;

        if(properties.has("display/major")) {
            d->systemInfo.displayInfo.at(i).major = properties.getString("display/major", d->systemInfo.displayInfo.at(i).major);
        } else {
            d->systemInfo.displayInfo.at(i).major = properties.getString(indexedBase.str() + "major", d->systemInfo.displayInfo.at(i).major);
        }

        if(properties.has("display/minor")) {
            d->systemInfo.displayInfo.at(i).minor = properties.getString("display/minor", d->systemInfo.displayInfo.at(i).minor);
        } else {
            d->systemInfo.displayInfo.at(i).minor = properties.getString(indexedBase.str() + "minor", d->systemInfo.displayInfo.at(i).minor);
        }

        d->systemInfo.displayInfo.at(i).name = properties.getString(indexedBase.str() + "name", d->systemInfo.displayInfo.at(i).name);
        d->systemInfo.displayInfo.at(i).diagonalInches = properties.getDouble(indexedBase.str() + "diagonal", d->systemInfo.displayInfo.at(i).diagonalInches);
        d->systemInfo.displayInfo.at(i).widthPixels = properties.getInt(indexedBase.str() + "res/x",d->systemInfo.displayInfo.at(i).widthPixels);
        d->systemInfo.displayInfo.at(i).heightPixels = properties.getInt(indexedBase.str() + "res/y", d->systemInfo.displayInfo.at(i).heightPixels);
        d->systemInfo.displayInfo.at(i).xDpi = properties.getDouble(indexedBase.str() + "dpi/x", d->systemInfo.displayInfo.at(i).xDpi);
        d->systemInfo.displayInfo.at(i).yDpi = properties.getDouble(indexedBase.str() + "dpi/y", d->systemInfo.displayInfo.at(i).yDpi);

        indexedBase.str(std::string());
        indexedBase << "display/" << i;
        for(sysinf::PropertyIter iter = properties.groupIterator(indexedBase.str()); !iter.done(); iter.next())
        {
            std::string name = getLastComponent(iter.name());
            if(name != "name" &&
               name != "diagonal" &&
               name != "res/x" &&
               name != "res/y" &&
               name != "dpi/x" &&
               name != "dpi/y") {
                d->systemInfo.displayInfo.at(i).attributes.insert( std::pair<std::string,std::string>(name, iter.value().getString()) );
            }
        }
    }


    size_t cpuCount = static_cast<size_t>(properties.getLong("cpu/count"));
    for (size_t i = d->systemInfo.cpuInfo.size(); i < cpuCount; ++i) {
        d->systemInfo.cpuInfo.push_back(sysinf::CpuInfo());
    }
    for (size_t i = 0; i < cpuCount; ++i) {
        indexedBase.str(std::string());
        indexedBase << "cpu/" << i << "/";

        d->systemInfo.cpuInfo.at(i).beautified    = true;

        if(properties.has("cpu/major")) {
            d->systemInfo.cpuInfo.at(i).major = properties.getString("cpu/major", d->systemInfo.cpuInfo.at(i).major);
        } else {
            d->systemInfo.cpuInfo.at(i).major = properties.getString(indexedBase.str() + "major", d->systemInfo.cpuInfo.at(i).major);
        }

        if(properties.has("cpu/minor")) {
            d->systemInfo.cpuInfo.at(i).minor = properties.getString("cpu/minor", d->systemInfo.cpuInfo.at(i).minor);
        } else {
            d->systemInfo.cpuInfo.at(i).minor = properties.getString(indexedBase.str() + "minor", d->systemInfo.cpuInfo.at(i).minor);
        }

        d->systemInfo.cpuInfo.at(i).name = properties.getString(indexedBase.str() + "name", d->systemInfo.cpuInfo.at(i).name);
        d->systemInfo.cpuInfo.at(i).cores = properties.getInt(indexedBase.str() + "cores", d->systemInfo.cpuInfo.at(i).cores);
        d->systemInfo.cpuInfo.at(i).threads = properties.getInt(indexedBase.str() + "threads", d->systemInfo.cpuInfo.at(i).threads);
        d->systemInfo.cpuInfo.at(i).frequencyMHz = properties.getDouble(indexedBase.str() + "frequencyMHz", d->systemInfo.cpuInfo.at(i).frequencyMHz);

        indexedBase.str(std::string());
        indexedBase << "cpu/" << i;
        for(sysinf::PropertyIter iter = properties.groupIterator(indexedBase.str()); !iter.done(); iter.next())
        {
            std::string name = getLastComponent(iter.name());
            if(name != "name" &&
               name != "cores" &&
               name != "threads" &&
               name != "frequencyMHz") {
                d->systemInfo.cpuInfo.at(i).attributes.insert( std::pair<std::string,std::string>(name, iter.value().getString()) );
            }
        }
    }


    size_t gpuCount = static_cast<size_t>(properties.getLong("gpu/count"));
    for (size_t i = d->systemInfo.gpuInfo.size(); i < gpuCount; ++i) {
        d->systemInfo.gpuInfo.push_back(sysinf::GpuInfo());
    }
    for (size_t i = 0; i < gpuCount; ++i) {
        indexedBase.str(std::string());
        indexedBase << "gpu/" << i << "/";

        d->systemInfo.gpuInfo.at(i).beautified    = true;

        if(properties.has("gpu/major")) {
            d->systemInfo.gpuInfo.at(i).major = properties.getString("gpu/major", d->systemInfo.gpuInfo.at(i).major);
        } else {
            d->systemInfo.gpuInfo.at(i).major = properties.getString(indexedBase.str() + "major", d->systemInfo.gpuInfo.at(i).major);
        }

        if(properties.has("gpu/minor")) {
            d->systemInfo.gpuInfo.at(i).minor = properties.getString("gpu/minor", d->systemInfo.gpuInfo.at(i).minor);
        } else {
            d->systemInfo.gpuInfo.at(i).minor = properties.getString(indexedBase.str() + "minor", d->systemInfo.gpuInfo.at(i).minor);
        }

        d->systemInfo.gpuInfo.at(i).name = properties.getString(indexedBase.str() + "name", d->systemInfo.gpuInfo.at(i).name);
        d->systemInfo.gpuInfo.at(i).driver = properties.getString(indexedBase.str() + "driver", d->systemInfo.gpuInfo.at(i).driver);
        d->systemInfo.gpuInfo.at(i).vendor = properties.getString(indexedBase.str() + "vendor", d->systemInfo.gpuInfo.at(i).vendor);
        d->systemInfo.gpuInfo.at(i).ids = properties.getString(indexedBase.str() + "ids", d->systemInfo.gpuInfo.at(i).ids);
        d->systemInfo.gpuInfo.at(i).memory = properties.getLong(indexedBase.str() + "memory", d->systemInfo.gpuInfo.at(i).memory);
        d->systemInfo.gpuInfo.at(i).pcie = properties.getString(indexedBase.str() + "pcie", d->systemInfo.gpuInfo.at(i).pcie);

        indexedBase.str(std::string());
        indexedBase << "gpu/" << i;
        for(sysinf::PropertyIter iter = properties.groupIterator(indexedBase.str()); !iter.done(); iter.next())
        {
            std::string name = getLastComponent(iter.name());
            if(name != "name" &&
               name != "driver" &&
               name != "vendor" &&
               name != "ids" &&
               name != "memory" &&
               name != "pcie") {
                d->systemInfo.gpuInfo.at(i).attributes.insert( std::pair<std::string,std::string>(name, iter.value().getString()) );
            }
        }
    }


    if(d->systemInfo.hasMultiGpuInfo) {
        //TODO multiGpu
    }


    d->systemInfo.memoryInfo.beautified        = true;
    d->systemInfo.memoryInfo.major             = properties.getString("memory/major", d->systemInfo.memoryInfo.major);
    d->systemInfo.memoryInfo.minor             = properties.getString("memory/minor", d->systemInfo.memoryInfo.minor);
    d->systemInfo.memoryInfo.details           = properties.getString("memory/details", d->systemInfo.memoryInfo.details);
    d->systemInfo.memoryInfo.sizeBytes         = properties.getLong("memory/size", d->systemInfo.memoryInfo.sizeBytes);


    size_t storageCount = static_cast<size_t>(properties.getLong("storage/count"));
    for (size_t i = d->systemInfo.storageInfo.size(); i < storageCount; ++i) {
        d->systemInfo.storageInfo.push_back(sysinf::StorageInfo());
    }
    for (size_t i = 0; i < storageCount; ++i) {
        indexedBase.str(std::string());
        indexedBase << "storage/" << i << "/";

        d->systemInfo.storageInfo.at(i).beautified    = true;

        if(properties.has("storage/major")) {
            d->systemInfo.storageInfo.at(i).major = properties.getString("storage/major", d->systemInfo.storageInfo.at(i).major);
        } else {
            d->systemInfo.storageInfo.at(i).major = properties.getString(indexedBase.str() + "major", d->systemInfo.storageInfo.at(i).major);
        }

        if(properties.has("storage/minor")) {
            d->systemInfo.storageInfo.at(i).minor = properties.getString("storage/minor", d->systemInfo.storageInfo.at(i).minor);
        } else {
            d->systemInfo.storageInfo.at(i).minor = properties.getString(indexedBase.str() + "minor", d->systemInfo.storageInfo.at(i).minor);
        }

        d->systemInfo.storageInfo.at(i).name = properties.getString(indexedBase.str() + "name", d->systemInfo.storageInfo.at(i).name);
        d->systemInfo.storageInfo.at(i).sizeBytes = properties.getLong(indexedBase.str() + "size", d->systemInfo.storageInfo.at(i).sizeBytes);
        d->systemInfo.storageInfo.at(i).isRemovable = properties.getBool(indexedBase.str() + "is/removable", d->systemInfo.storageInfo.at(i).isRemovable);

        indexedBase.str(std::string());
        indexedBase << "storage/" << i;
        for(sysinf::PropertyIter iter = properties.groupIterator(indexedBase.str()); !iter.done(); iter.next())
        {
            std::string name = getLastComponent(iter.name());
            if(name != "name" &&
               name != "size" &&
               name != "is/removable") {
                d->systemInfo.storageInfo.at(i).attributes.insert( std::pair<std::string,std::string>(name, iter.value().getString()) );
            }
        }
    }


    size_t batteryCount = static_cast<size_t>(properties.getLong("battery/count"));
    for (size_t i = d->systemInfo.batteryInfo.size(); i < batteryCount; ++i) {
        d->systemInfo.batteryInfo.push_back(sysinf::BatteryInfo());
    }
    for (size_t i = 0; i < batteryCount; ++i) {
        indexedBase.str(std::string());
        indexedBase << "battery/" << i << "/";

        d->systemInfo.batteryInfo.at(i).name = properties.getString(indexedBase.str() + "name", d->systemInfo.batteryInfo.at(i).name);
        d->systemInfo.batteryInfo.at(i).capacity_mAh = properties.getDouble(indexedBase.str() + "mah", d->systemInfo.batteryInfo.at(i).capacity_mAh);
        d->systemInfo.batteryInfo.at(i).technology = properties.getString(indexedBase.str() + "technology", d->systemInfo.batteryInfo.at(i).technology);
    }


    size_t cameraCount = static_cast<size_t>(properties.getLong("camera/count"));
    for (size_t i = d->systemInfo.cameraInfo.size(); i < cameraCount; ++i) {
        d->systemInfo.cameraInfo.push_back(sysinf::CameraInfo());
    }
    for (size_t i = 0; i < cameraCount; ++i) {
        indexedBase.str(std::string());
        indexedBase << "camera/" << i << "/";

        d->systemInfo.cameraInfo.at(i).beautified = true;
        d->systemInfo.cameraInfo.at(i).major = properties.getString(indexedBase.str() + "major", d->systemInfo.cameraInfo.at(i).major);
        d->systemInfo.cameraInfo.at(i).minor = properties.getString(indexedBase.str() + "minor", d->systemInfo.cameraInfo.at(i).minor);
        d->systemInfo.cameraInfo.at(i).name = properties.getString(indexedBase.str() + "name", d->systemInfo.cameraInfo.at(i).name);
        d->systemInfo.cameraInfo.at(i).type = properties.getString(indexedBase.str() + "type", d->systemInfo.cameraInfo.at(i).type);

        d->systemInfo.cameraInfo.at(i).pictureWidthPixels = properties.getInt(indexedBase.str() + "pic/x", d->systemInfo.cameraInfo.at(i).pictureWidthPixels);
        d->systemInfo.cameraInfo.at(i).pictureHeightPixels = properties.getInt(indexedBase.str() + "pic/y", d->systemInfo.cameraInfo.at(i).pictureHeightPixels);
        d->systemInfo.cameraInfo.at(i).pictureResolutionMP = properties.getDouble(indexedBase.str() + "pic/mp", d->systemInfo.cameraInfo.at(i).pictureResolutionMP);

        d->systemInfo.cameraInfo.at(i).videoWidthPixels = properties.getInt(indexedBase.str() + "vid/x", d->systemInfo.cameraInfo.at(i).videoWidthPixels);
        d->systemInfo.cameraInfo.at(i).videoHeightPixels = properties.getInt(indexedBase.str() + "vid/y", d->systemInfo.cameraInfo.at(i).videoHeightPixels);
        d->systemInfo.cameraInfo.at(i).videoResolutionMP = properties.getDouble(indexedBase.str() + "vid/mp", d->systemInfo.cameraInfo.at(i).videoResolutionMP);

        d->systemInfo.cameraInfo.at(i).hasAutofocus = properties.getBool(indexedBase.str() + "has/autofocus", d->systemInfo.cameraInfo.at(i).hasAutofocus);
        d->systemInfo.cameraInfo.at(i).hasFlash = properties.getBool(indexedBase.str() + "has/face_detection", d->systemInfo.cameraInfo.at(i).hasFlash);
        d->systemInfo.cameraInfo.at(i).hasFaceDetection = properties.getBool(indexedBase.str() + "has/flash", d->systemInfo.cameraInfo.at(i).hasFaceDetection);
        d->systemInfo.cameraInfo.at(i).hasHdr = properties.getBool(indexedBase.str() + "has/hdr", d->systemInfo.cameraInfo.at(i).hasHdr);
        d->systemInfo.cameraInfo.at(i).hasTouchFocus = properties.getBool(indexedBase.str() + "has/touch_focus", d->systemInfo.cameraInfo.at(i).hasTouchFocus);

        d->systemInfo.cameraInfo.at(i).flat = properties.getString(indexedBase.str() + "flat", d->systemInfo.cameraInfo.at(i).flat);
    }

#ifndef TARGET_OS_IPHONE
    for(sysinf::PropertyIter iter = properties.groupIterator("features/server"); !iter.done(); iter.next())
    {
        d->systemInfo.hasFeatureInfo = true;
        std::string name = getLastComponent(iter.name());
        d->systemInfo.featureInfo.features.insert( std::pair<std::string, bool>(name, iter.value().getBool()) );
    }
#endif

    for(sysinf::PropertyIter iter = properties.groupIterator("sensor/server"); !iter.done(); iter.next())
    {
        d->systemInfo.hasSensorInfo = true;
        std::string name = getLastComponent(iter.name());
        d->systemInfo.sensorInfo.sensors.insert( std::pair<std::string,std::string>(name, iter.value().getString()) );
    }
}



sysinf::Properties SystemInfoGateway::getProperties() const
{
    sysinf::Properties properties;
    sysinf::KeyValueVisitor propertiesVisitor(properties);
    d->systemInfo.applyVisitor(propertiesVisitor);
#ifndef __APPLE__
    std::vector<Configuration> configurations = clConfigurations();
    properties.setLong(sysinf::API_CL + "/" + sysinf::API_CL_CONFIGURATION_COUNT,
            configurations.size());
    for (size_t i = 0; i < configurations.size(); ++i) {
        const Configuration& configuration = configurations.at(i);
        std::ostringstream prefixStream;
        prefixStream << sysinf::API_CL + "/" + sysinf::API_CL_CONFIGURATIONS << "/" << i << "/";
        std::string prefix = prefixStream.str();
        properties.setString(prefix + sysinf::API_CL_CONFIGURATION_NAME, configuration.name());
        properties.setString(prefix + sysinf::API_CL_CONFIGURATION_TYPE, configuration.type());
    }
#endif
    return properties;
}



std::vector<SystemInfoItem> SystemInfoGateway::getItems(const std::string& imagePath) const
{
    std::vector<SystemInfoItem> items;
    ItemVisitor itemVisitor(items, imagePath);
    d->systemInfo.applyVisitor(itemVisitor);
    return items;
}



std::vector<Configuration> SystemInfoGateway::getConfigurations() const
{
    std::vector<Configuration> configurations;
#ifndef __APPLE__
    std::vector<Configuration> configurationsCL = clConfigurations();
    std::vector<Configuration> configurationsCU = cuConfigurations();
#else
    std::vector<Configuration> configurationsCL;
    std::vector<Configuration> configurationsCU;
#endif
    std::vector<Configuration> configurationsGPU = graphicsConfiguration();

    configurations.insert(configurations.end(), configurationsCL.begin(), configurationsCL.end());
    configurations.insert(configurations.end(), configurationsCU.begin(), configurationsCU.end());
    configurations.insert(configurations.end(), configurationsGPU.begin(), configurationsGPU.end());

    return configurations;
}


std::vector<Configuration> SystemInfoGateway::graphicsConfiguration() const
{
    std::vector<Configuration> configs;
    std::vector<std::string> features;
    if (!d->systemInfo.batteryInfo.empty()) {
        features.push_back("battery");
    }

    if (d->systemInfo.hasGLES) {
        std::vector<tfw::ApiDefinition> apiDefinitions;
        tfw::ApiDefinition esVersion;
        esVersion.setType(tfw::ApiDefinition::ES);
        esVersion.setMajor(d->systemInfo.glesInfo.majorVersion);
        esVersion.setMinor(d->systemInfo.glesInfo.minorVersion);
        esVersion.setExtensions(d->systemInfo.glesInfo.extensions);
        apiDefinitions.push_back(esVersion);

        Configuration configuration;
        configuration.setName(d->systemInfo.glesInfo.renderer);
        configuration.setType("GPU");
        configuration.setApiDefinitions(apiDefinitions);
        configuration.setFeatures(features);
        configs.push_back(configuration);
    }

    if (d->systemInfo.hasGL ) {
        std::vector<tfw::ApiDefinition> apiDefinitions;
        tfw::ApiDefinition glVersion;
        glVersion.setType(tfw::ApiDefinition::GL);
        glVersion.setMajor(d->systemInfo.glInfo.majorVersion);
        glVersion.setMinor(d->systemInfo.glInfo.minorVersion);
        glVersion.setExtensions(d->systemInfo.glInfo.extensions);
        apiDefinitions.push_back(glVersion);

        Configuration configuration;
        configuration.setName(d->systemInfo.glInfo.renderer);
        configuration.setType("GPU");
        configuration.setApiDefinitions(apiDefinitions);
        configuration.setFeatures(features);
        configs.push_back(configuration);
    }

#ifdef _WIN32
    if (d->systemInfo.hasDirectx) {
        tfw::ApiDefinition dx;
        dx.setType(tfw::ApiDefinition::DX);

        for (size_t i = 0; i < d->systemInfo.directxInfo.devices.size(); ++i)
        {
			std::vector<tfw::ApiDefinition> apiDefinitions;

            Configuration configuration;
			std::string cardName = d->systemInfo.directxInfo.devices[i].name;
			dx.setMajor(d->systemInfo.directxInfo.devices[i].majorVersion);
			dx.setMinor(d->systemInfo.directxInfo.devices[i].minorVersion);
			dx.setDeviceId(cardName + ";" + std::to_string(d->systemInfo.directxInfo.devices[i].luid));
            dx.setDeviceIndex(static_cast<int>(i));
			apiDefinitions.push_back(dx);
            configuration.setName(cardName);
            configuration.setType("GPU");
            configuration.setApiDefinitions(apiDefinitions);
            configuration.setFeatures(features);
            configs.push_back(configuration);
        }
    }
	if (d->systemInfo.hasDirectx12) {
		tfw::ApiDefinition dx12;
		dx12.setType(tfw::ApiDefinition::DX12);

		for (size_t i = 0; i < d->systemInfo.directx12Info.dx12_devices.size(); ++i)
		{
			const sysinf::DirectxDeviceInfo& dx12_device = d->systemInfo.directx12Info.dx12_devices[i];

			std::vector<tfw::ApiDefinition> apiDefinitions;
			Configuration configuration;

			dx12.setMajor(dx12_device.majorVersion);
			dx12.setMinor(dx12_device.minorVersion);
            dx12.setDeviceId(dx12_device.name + ";" + std::to_string(dx12_device.luid));
            dx12.setDeviceIndex(static_cast<int>(i));
			apiDefinitions.push_back(dx12);
			configuration.setName(dx12_device.name);
			configuration.setType("GPU");
			configuration.setApiDefinitions(apiDefinitions);
			configuration.setFeatures(features);
			configs.push_back(configuration);
		}
	}
#endif

    if (d->systemInfo.hasVulkan ) {
        tfw::ApiDefinition vulkan;
        vulkan.setType(tfw::ApiDefinition::VULKAN);

		for (size_t i = 0; i < d->systemInfo.vulkanInfo.devices.size(); ++i)
		{
			std::vector<tfw::ApiDefinition> apiDefinitions;

			std::string cardName = d->systemInfo.vulkanInfo.devices[i].card_name;
			std::stringstream cardNameWithLuid;
			cardNameWithLuid << cardName << ";" << d->systemInfo.vulkanInfo.devices[i].luid;

			vulkan.setMajor(d->systemInfo.vulkanInfo.devices[i].major_vulkan_version);
			vulkan.setMinor(d->systemInfo.vulkanInfo.devices[i].minor_vulkan_version);
			vulkan.setDeviceId(cardNameWithLuid.str());
            vulkan.setDeviceIndex(static_cast<int>(i));

			apiDefinitions.push_back(vulkan);

			Configuration configuration;
			configuration.setName(cardName);
			configuration.setType("GPU");
			configuration.setApiDefinitions(apiDefinitions);
			configuration.setFeatures(features);
			configs.push_back(configuration);
		}
    }

    if (d->systemInfo.hasMetal) {
        tfw::ApiDefinition metalVersion;
        metalVersion.setType(tfw::ApiDefinition::METAL);

        for(size_t i = 0; i < d->systemInfo.metalInfo.devices.size(); i++)
        {
            std::vector<tfw::ApiDefinition> apiDefinitions;

            //metalVersion.fromVersionString(properties.getString(sysinf::API_METAL_VERSION_STRING));
            metalVersion.fromVersionString("1.0"); // FIXME
            metalVersion.setDeviceId(d->systemInfo.metalInfo.devices[i].deviceName);
            metalVersion.setDeviceIndex(static_cast<int>(i));
            apiDefinitions.push_back(metalVersion);

            if (!d->systemInfo.metalInfo.devices[i].isMobile // The feature check is only relevant on mobile
                || d->systemInfo.memoryInfo.sizeBytes > 1610612736) // >1.5GB
            {
                features.push_back("memory2gb");
            }

            if (d->systemInfo.metalInfo.devices[i].tessellationSupport)
            {
                features.push_back("tessellation");
            }

            Configuration configuration;
            configuration.setName(d->systemInfo.metalInfo.devices[i].deviceName);
            configuration.setType("GPU");
            configuration.setApiDefinitions(apiDefinitions);
            configuration.setFeatures(features);
            configs.push_back(configuration);
        }
    }

    return configs;
}


std::map<std::string, std::string> SystemInfoGateway::extractUnsupportedTests(
        const sysinf::Properties& properties) const
{
    std::map<std::string, std::string> result;
    for (sysinf::PropertyIter i = properties.groupIterator(sysinf::TEST_RESTRICTION_REASON);
        !i.done(); i.next())
    {
        std::string testId = i.name().substr(sysinf::TEST_RESTRICTION_REASON.size() + 1);
        std::string::iterator end = std::find(testId.begin(), testId.end(), '/');
        testId = testId.substr(0, end - testId.begin());
        result[testId] = i.value().getString();
    }
    return result;
}












/* Abandon all hope, ye who enter here */

/* Copied from CompuBench source */
#ifndef __APPLE__
#include "CL/clew.h"
using namespace std;

struct configTemplate {
    string name;
    string vendor;
    std::string type;
    tfw::ApiDefinition api;

    bool singleContext;
    bool enabled;
    std::string comment;

    int platformIndex;

    cl_platform_id platformid;
    vector<cl_device_id> deviceids;
};

struct Exception {
    static void Check(cl_int) {}
};

#define CHECK_CL(x) x;
#define DEALLOC_ARR(pointer) if ((pointer) != nullptr) { delete[] (pointer); (pointer) = nullptr; }

namespace cb {
    namespace cl {

        std::string trimString(const std::string& str, const std::string& whitespace = " \t\r\n")
        {
            size_t strBegin = str.find_first_not_of(whitespace);
            if (strBegin == std::string::npos)
                return "";

            size_t strEnd = str.find_last_not_of(whitespace);
            size_t strRange = strEnd - strBegin + 1;

            return str.substr(strBegin, strRange);
        }

        std::string reduceString(const std::string& str, const std::string& fill = " ", const std::string& whitespace = " \t\r\n")
        {
            std::string result = trimString(str, whitespace);

            size_t beginSpace = result.find_first_of(whitespace);
            while (beginSpace != std::string::npos)
            {
                size_t endSpace = result.find_first_not_of(whitespace, beginSpace);
                size_t range = endSpace - beginSpace;

                result.replace(beginSpace, range, fill);

                size_t newStart = beginSpace + fill.length();
                beginSpace = result.find_first_of(whitespace, newStart);
            }

            return result;
        }

        string GetString(cl_device_id device, cl_device_info param_name)
        {
            size_t ret;
            CHECK_CL(clGetDeviceInfo(device, param_name, 0, NULL, &ret))
                char *value = new char[ret];
            CHECK_CL(clGetDeviceInfo(device, param_name, ret, value, NULL))
                string str(value);
            DEALLOC_ARR(value)
                return reduceString(str);
        }

        cl_ulong GetUlong(cl_device_id device, cl_device_info param_name)
        {
            cl_ulong value;
            CHECK_CL(clGetDeviceInfo(device, param_name, sizeof(cl_ulong), &value, NULL))
                return value;
        }

        cl_uint GetUint(cl_device_id device, cl_device_info param_name)
        {
            cl_uint value;
            CHECK_CL(clGetDeviceInfo(device, param_name, sizeof(cl_uint), &value, NULL))
                return value;
        }

        cl_bool GetBool(cl_device_id device, cl_device_info param_name)
        {
            cl_bool value;
            CHECK_CL(clGetDeviceInfo(device, param_name, sizeof(cl_bool), &value, NULL))
                return value;
        }

        size_t GetSize(cl_device_id device, cl_device_info param_name)
        {
            size_t value;
            CHECK_CL(clGetDeviceInfo(device, param_name, sizeof(size_t), &value, NULL))
            return value;
        }

        bool IsCPU(cl_device_id device)
        {
            cl_device_type type;
            CHECK_CL(clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL))
            return (type & CL_DEVICE_TYPE_CPU) != 0;
        }

        bool IsGPU(cl_device_id device)
        {
            cl_device_type type;
            CHECK_CL(clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL))
            return (type & CL_DEVICE_TYPE_GPU) != 0;
        }

        bool IsACC(cl_device_id device)
        {
            cl_device_type type;
            CHECK_CL(clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL))
            return (type & CL_DEVICE_TYPE_ACCELERATOR) != 0;
        }
    }
}



std::string GetDeviceType(cl_device_id device)
{
    std::string deviceType;

    if (cb::cl::IsCPU(device)) {
        deviceType = "CPU";
    }
    else if (cb::cl::IsGPU(device))
    {
        if (cb::cl::GetBool(device, CL_DEVICE_HOST_UNIFIED_MEMORY))
            deviceType = "iGPU";
        else
            deviceType = "dGPU";
    }
    else if (cb::cl::IsACC(device))
    {
        deviceType = "ACC";
    }

    return deviceType;
}



std::vector<Configuration> SystemInfoGateway::clConfigurations() const
{
    if (!clewInitialized()) {
        if (clewInit() != 0) {
            return std::vector<Configuration>();
        }
    }

	vector<configTemplate> devicesSamePlatform;

    cl_uint platformCount = 0;
    cl_int status;

    status = clGetPlatformIDs(0, nullptr, &platformCount);
    Exception::Check(status);
    cl_platform_id *platform_ids = new cl_platform_id[platformCount];
    status = clGetPlatformIDs(platformCount, platform_ids, nullptr);
    Exception::Check(status);

    for (cl_uint i = 0; i < platformCount; ++i)
    {
        cl_uint deviceCount = 0;
        status = clGetDeviceIDs(platform_ids[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceCount);
        Exception::Check(status);
        cl_device_id *device_ids = new cl_device_id[deviceCount];
        status = clGetDeviceIDs(platform_ids[i], CL_DEVICE_TYPE_ALL, deviceCount, device_ids, nullptr);
        Exception::Check(status);

        /// single-device configs
        for (cl_uint j = 0; j < deviceCount; ++j)
        {
            string deviceNameStr;
            string deviceVendorStr;
            string deviceOCLVersion;

            {
                size_t infoBufferLength;
                status = clGetDeviceInfo(device_ids[j], CL_DEVICE_NAME, 0, NULL, &infoBufferLength);
                Exception::Check(status);
                char* infoBuffer = new char[infoBufferLength];
                status = clGetDeviceInfo(device_ids[j], CL_DEVICE_NAME, infoBufferLength, infoBuffer, nullptr);
                Exception::Check(status);
                deviceNameStr = cb::cl::reduceString(std::string(infoBuffer));
                delete[] infoBuffer;
            }

			{
				size_t infoBufferLength;
				status = clGetDeviceInfo(device_ids[j], CL_DEVICE_VENDOR, 0, NULL, &infoBufferLength);
				Exception::Check(status);
				char* infoBuffer = new char[infoBufferLength];
				status = clGetDeviceInfo(device_ids[j], CL_DEVICE_VENDOR, infoBufferLength, infoBuffer, nullptr);
				Exception::Check(status);
				deviceVendorStr = cb::cl::reduceString(std::string(infoBuffer));
				delete[] infoBuffer;
			}

			{
				size_t infoBufferLength;
				status = clGetDeviceInfo(device_ids[j], CL_DEVICE_VERSION, 0, NULL, &infoBufferLength);
				Exception::Check(status);
				char* infoBuffer = new char[infoBufferLength];
				status = clGetDeviceInfo(device_ids[j], CL_DEVICE_VERSION, infoBufferLength, infoBuffer, nullptr);
				Exception::Check(status);
				deviceOCLVersion = cb::cl::reduceString(std::string(infoBuffer));
				delete[] infoBuffer;
			}

            configTemplate currentDevice;
            currentDevice.name = deviceNameStr;
            currentDevice.vendor = deviceVendorStr;
            currentDevice.type = GetDeviceType(device_ids[j]);
            currentDevice.enabled = true;
            currentDevice.platformIndex = i;
            currentDevice.platformid = platform_ids[i];
            currentDevice.deviceids.push_back(device_ids[j]);
            currentDevice.api.setType(tfw::ApiDefinition::CL);
            currentDevice.api.fromVersionString(deviceOCLVersion);
            currentDevice.singleContext = true;

            if (deviceVendorStr.find("GenuineIntel") != string::npos)
            {
                currentDevice.comment = "STATUS_INTEL_ON_AMD";
                currentDevice.enabled = false;
            }

            if (deviceOCLVersion.find("OpenCL 1.0") != string::npos)
            {
                currentDevice.comment = "STATUS_OCL10";
                currentDevice.enabled = false;
            }

			{
			    std::string vendorUpper = deviceVendorStr;
			    for (size_t i = 0; i < strlen(vendorUpper.c_str()); i++)
			        vendorUpper[i] = toupper(vendorUpper[i]);

			    if (vendorUpper.find("NVIDIA") == string::npos && deviceOCLVersion.find("OpenCL 1.1") != string::npos)
			    {
			        currentDevice.comment = "STATUS_OCL_OLDDRIVER";
			        currentDevice.enabled = false;
			    }
			}

			currentDevice.name = currentDevice.vendor + ": " + currentDevice.name;

            //#if defined(HAVE_EGL)
			//#endif

			// skip OpenCL CPU devices
			if(cb::cl::IsCPU(device_ids[j]) == false)
				devicesSamePlatform.push_back(currentDevice);
        }
        delete[] device_ids;
    }
    delete[] platform_ids;

	tfw::ApiDefinition apiDefinitionGL;

	{
		std::vector<Configuration> gpuConfigs = graphicsConfiguration();
		for (unsigned int n = 0; n < gpuConfigs.size(); ++n)
		{
			std::vector<tfw::ApiDefinition> apiDefinitions = gpuConfigs[n].ApiDefinitions();

			// it seems ApiDefinitions() return only one element vector
			if(apiDefinitions.size() >= 1)
			{
				if(apiDefinitions[0].type() == tfw::ApiDefinition::GL || apiDefinitions[0].type() == tfw::ApiDefinition::ES)
				{
					apiDefinitionGL = apiDefinitions[0];
					break;
				}
			}
		}
	}

    vector<Configuration> configurations;
    for (unsigned int m = 0; m < devicesSamePlatform.size(); ++m)
    {
		std::vector<tfw::ApiDefinition> apiDefinitions;
		apiDefinitions.push_back(apiDefinitionGL);

		apiDefinitions.push_back(devicesSamePlatform[m].api);

		std::vector<std::string> features;
		if (devicesSamePlatform[m].singleContext) {
            features.push_back("single_context");
        }

		Configuration configuration;
		configuration.setName(devicesSamePlatform[m].name);
		configuration.setType(devicesSamePlatform[m].type);
        configuration.setApiDefinitions(apiDefinitions);
		configuration.setFeatures(features);

		configuration.setEnabled(devicesSamePlatform[m].enabled);
		configuration.setErrorString(devicesSamePlatform[m].comment);

		configurations.push_back(configuration);
    }

    return configurations;
}

std::vector<Configuration> SystemInfoGateway::cuConfigurations() const
{
    std::vector<Configuration> configurations;

    if (d->systemInfo.hasCuda)
    {
		tfw::ApiDefinition apiDefinitionGL;

		std::vector<Configuration> gpuConfigs = graphicsConfiguration();
		for (unsigned int n = 0; n < gpuConfigs.size(); ++n)
		{
			std::vector<tfw::ApiDefinition> apiDefinitions = gpuConfigs[n].ApiDefinitions();

			// it seems ApiDefinitions() return only one element vector
			if(apiDefinitions.size() >= 1)
			{
				if(apiDefinitions[0].type() == tfw::ApiDefinition::GL || apiDefinitions[0].type() == tfw::ApiDefinition::ES)
				{
					apiDefinitionGL = apiDefinitions[0];
					break;
				}
			}
		}

        for(unsigned int devicesId = 0; devicesId < d->systemInfo.cudaInfo.devices.size(); ++devicesId)
        {
			std::vector<tfw::ApiDefinition> apiDefinitions;
			apiDefinitions.push_back(apiDefinitionGL);

			std::string name = d->systemInfo.cudaInfo.devices[devicesId].name;

			tfw::ApiDefinition cudaVersion;
            cudaVersion.setType(tfw::ApiDefinition::CUDA);

			std::stringstream ss;
			ss << d->systemInfo.cudaInfo.devices[devicesId].ccMajor;
			ss << ".";
			ss << d->systemInfo.cudaInfo.devices[devicesId].ccMinor;

			std::string ssStr = ss.str();

            cudaVersion.fromVersionString(ss.str());
			apiDefinitions.push_back(cudaVersion);

            std::vector<std::string> features;

            Configuration configuration;
            configuration.setName(name);
            configuration.setType("GPU");
            configuration.setApiDefinitions(apiDefinitions);
            configuration.setFeatures(features);

			if (d->systemInfo.cudaInfo.devices[devicesId].ccMajor < 3)
			{
				configuration.setEnabled(false);
				configuration.setErrorString("CompuBench requires Compute Capability 3.0 or higher");
				configuration.setErrorString("STATUS_CUDA_LOWER_30");
			}

            configurations.push_back(configuration);
		}
    }

    return configurations;
}
#endif // __APPLE__

std::vector<Configuration> SystemInfoGateway::mtlConfigurations() const
{
    std::vector<Configuration> configurations;

    if (d->systemInfo.hasMetal)
    {
        tfw::ApiDefinition apiDefinitionMetal;

        std::vector<Configuration> gpuConfigs = graphicsConfiguration();
        for (unsigned int n = 0; n < gpuConfigs.size(); ++n)
        {
            std::vector<tfw::ApiDefinition> apiDefinitions = gpuConfigs[n].ApiDefinitions();

            // it seems ApiDefinitions() return only one element vector
            if(apiDefinitions.size() >= 1)
            {
                if(apiDefinitions[0].type() == tfw::ApiDefinition::METAL)
                {
                    apiDefinitionMetal = apiDefinitions[0];
                    break;
                }
            }
        }

        for(unsigned int devicesId = 0; devicesId < d->systemInfo.metalInfo.devices.size(); ++devicesId)
        {
            std::vector<tfw::ApiDefinition> apiDefinitions;
            apiDefinitions.push_back(apiDefinitionMetal);

            std::string name = d->systemInfo.metalInfo.devices[devicesId].deviceName;

            tfw::ApiDefinition metalVersion;
            metalVersion.setType(tfw::ApiDefinition::METAL);
            metalVersion.fromVersionString("1.0");
            apiDefinitions.push_back(metalVersion);

            std::vector<std::string> features;

            Configuration configuration;
            configuration.setName(name);
            configuration.setType("GPU");
            configuration.setApiDefinitions(apiDefinitions);
            configuration.setFeatures(features);

            configurations.push_back(configuration);
        }
    }

    return configurations;
}
