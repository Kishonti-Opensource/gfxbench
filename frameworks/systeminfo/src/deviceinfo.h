/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <cstdint>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace sysinf
{

inline std::string formatSize(int64_t sizeBytes)
{
    const char* const UNITS[] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB" };
    size_t UNIT_COUNT = sizeof(UNITS) / sizeof(UNITS[0]);
    int64_t temp = sizeBytes;
    const char* unit = UNITS[0];
    for (size_t i = 1; (i < UNIT_COUNT) && (temp >= 1024); ++i) {
        temp /= 1024;
        unit = UNITS[i];
    }
    std::ostringstream oss;
    oss << temp << ' ' << unit;
    return oss.str();
}



class DeviceInfo
{
public:
    DeviceInfo():
        beautified(false)
    {}
    
    bool beautified;
    std::string major;
    std::string minor;
    std::string name;
    std::string image;
    std::string manufacturer;
    std::string chipset;
    std::map<std::string, std::string> attributes;
    
    template<class F> void applyVisitor(F visitor) const
    {
        for (auto i = attributes.begin(); i != attributes.end(); ++i) {
            visitor(i->first, i->second);
        }
        visitor("major", beautified ? major : manufacturer);
        visitor("minor", beautified ? minor : name);
        visitor("image", image);
        visitor("name", name);
        visitor("manufacturer", manufacturer);
        visitor("chipset", chipset);
    }
};



class OsInfo
{
public:
    OsInfo():
        beautified(false)
    {}
    
    bool beautified;
    std::string major;
    std::string minor;
    std::string name;
    std::string longName;
    std::string shortName;
    std::string fingerprint;
    std::string arch;
    std::string build;
    std::map<std::string, std::string> attributes;
    
    template<class F> void applyVisitor(F visitor) const
    {
        for (auto i = attributes.begin(); i != attributes.end(); ++i) {
            visitor(i->first, i->second);
        }
        visitor("major", beautified ? major : longName);
        visitor("minor", beautified ? minor : build);
        visitor("name", name);
        visitor("long", longName);
        visitor("short", shortName);
        visitor("fingerprint", fingerprint);
        visitor("arch", arch);
        visitor("build", build);
    }
};



class DisplayInfo
{
public:
    DisplayInfo():
        beautified(false),
        diagonalInches(-1.0),
        widthPixels(-1),
        heightPixels(-1),
        xDpi(-1),
        yDpi(-1)
    {}
    
    bool beautified;
    std::string major;
    std::string minor;
    std::string name;
    double diagonalInches;
    int widthPixels;
    int heightPixels;
    double xDpi;
    double yDpi;
    std::map<std::string, std::string> attributes;
    
    template<class F> void applyVisitor(F visitor) const
    {
        for (auto i = attributes.begin(); i != attributes.end(); ++i) {
            visitor(i->first, i->second);
        }
        std::ostringstream minorStream;
        minorStream << widthPixels << "x" << heightPixels;
        visitor("major", beautified ? major : name);
        visitor("minor", beautified ? minor : minorStream.str());
        visitor("name", name);
        visitor("diagonal", diagonalInches);
        visitor("widthPixels", widthPixels);
        visitor("heightPixels", heightPixels);
        visitor("xDpi", xDpi);
        visitor("yDpi", yDpi);
    }
};



class CpuInfo
{
public:
    CpuInfo() :
        beautified(false),
        cores(-1),
        threads(-1),
        frequencyMHz(-1.0)
    {}
    
    bool beautified;
    std::string major;
    std::string minor;
    std::string name;
    int cores;
    int threads;
    double frequencyMHz;
    std::map<std::string, std::string> attributes;
    
    template<class F> void applyVisitor(F visitor) const
    {
        for (auto i = attributes.begin(); i != attributes.end(); ++i) {
            visitor(i->first, i->second);
        }
        std::ostringstream minorStream;
        if (cores >= 0) {
            minorStream << "Number of cores: " << cores;
        }
        if (threads >= 0) {
            if (!minorStream.str().empty()) {
                minorStream << ", ";
            }
            minorStream << "Number of logical processors: " << threads;
        }
        visitor("major", beautified ? major : name);
        visitor("minor", beautified ? minor : minorStream.str());
        visitor("name", name);
        visitor("cores", cores);
        visitor("threads", threads);
        visitor("frequency", frequencyMHz);
    }
};



class GpuInfo
{
public:
    GpuInfo():
        beautified(false),
        memory(-1)
    {}
    
    bool beautified;
    std::string major;
    std::string minor;
    std::string name;
    std::string driver;
    std::string vendor;
    std::string ids;
    int64_t memory;
    std::string pcie;
    std::map<std::string, std::string> attributes;
    
    template<class F> void applyVisitor(F visitor) const
    {
        for (auto i = attributes.begin(); i != attributes.end(); ++i) {
            visitor(i->first, i->second);
        }
        visitor("major", beautified ? major : name);
        visitor("minor", beautified ? minor : driver);
        visitor("name", name);
        visitor("driver", driver);
        visitor("vendor", vendor);
        visitor("ids", ids);
        visitor("memory", memory);
        visitor("pcie", pcie);
    }
};



class MultiGpuInfo
{
public:
    MultiGpuInfo():
        beautified(false),
        gpuCount(-1),
        isEnabled(false)
    {}
    
    bool beautified;
    std::string major;
    std::string minor;
    std::string name;
    std::string driver;
    std::string error;
    int gpuCount;
    bool isEnabled;
    
    template<class F> void applyVisitor(F visitor) const
    {
        visitor("name", beautified ? major : name);
        visitor("driver", beautified ? minor : driver);
        visitor("error", error);
        visitor("gpuCount", gpuCount);
        visitor("enabled", isEnabled);
    }
};



class MemoryInfo
{
public:
    MemoryInfo():
        beautified(false),
        sizeBytes(-1)
    {}
    
    bool beautified;
    std::string major;
    std::string minor;
    std::string details;
    int64_t sizeBytes;
    
    template<class F> void applyVisitor(F visitor) const
    {
        visitor("major", beautified ? major : formatSize(sizeBytes));
        visitor("minor", beautified ? minor : details);
        visitor("name", formatSize(sizeBytes)); // legacy
        visitor("details", details);
        visitor("size", sizeBytes);
    }
};



class StorageInfo
{
public:
    StorageInfo() :
        beautified(false),
        sizeBytes(-1),
        isRemovable(false)
    {}
    
    bool beautified;
    std::string major;
    std::string minor;
    std::string name;
    int64_t sizeBytes;
    bool isRemovable;
    std::map<std::string, std::string> attributes;
    
    template<class F> void applyVisitor(F visitor) const
    {
        for (auto i = attributes.begin(); i != attributes.end(); ++i) {
            visitor(i->first, i->second);
        }
        visitor("major", beautified ? major : name);
        visitor("minor", beautified ? minor : formatSize(sizeBytes));
        visitor("name", name);
        visitor("size", sizeBytes);
        visitor("isRemovable", isRemovable);
    }
};



class BatteryInfo
{
public:
    BatteryInfo() :
        levelRatio(-1.0),
        capacity_mAh(-1.0),
        isCharging(false),
        isConnected(false)
    {}
    
    std::string name;
    double levelRatio;
    double capacity_mAh;
    std::string technology;
    bool isCharging;
    bool isConnected;
    
    template<class F> void applyVisitor(F visitor) const
    {
        visitor("level", levelRatio);
        visitor("mah", capacity_mAh);
        visitor("technology", technology);
        visitor("isCharging", isCharging);
        visitor("isConnected", isConnected);
    }
};



class CameraInfo
{
public:
    CameraInfo() :
        beautified(false),
        pictureWidthPixels(-1),
        pictureHeightPixels(-1),
        pictureResolutionMP(-1.0),
        videoWidthPixels(-1),
        videoHeightPixels(-1),
        videoResolutionMP(-1.0),
        hasAutofocus(false),
        hasFlash(false),
        hasFaceDetection(false),
        hasHdr(false),
        hasTouchFocus(false)
    {}
    
    bool beautified;
    std::string major;
    std::string minor;
    std::string name;
    std::string type;
    int pictureWidthPixels;
    int pictureHeightPixels;
    double pictureResolutionMP;
    int videoWidthPixels;
    int videoHeightPixels;
    double videoResolutionMP;
    bool hasAutofocus;
    bool hasFlash;
    bool hasFaceDetection;
    bool hasHdr;
    bool hasTouchFocus;
    std::string flat;
    std::map<std::string, std::string> attributes;
    
    template<class F> void applyVisitor(F visitor) const
    {
        for (auto i = attributes.begin(); i != attributes.end(); ++i) {
            visitor(i->first, i->second);
        }
        visitor("major", beautified ? major : name);
        visitor("minor", beautified ? minor : "");
        visitor("name", name);
        visitor("type", type);
        visitor("pictureWidthPixels", pictureWidthPixels);
        visitor("pictureHeightPixels", pictureHeightPixels);
        visitor("pictureResolutionMP", pictureResolutionMP);
        visitor("videoWidthPixels", videoWidthPixels);
        visitor("videoHeightPixels", videoHeightPixels);
        visitor("videoResolutionMP", videoResolutionMP);
        visitor("hasAutofocus", hasAutofocus);
        visitor("hasFlash", hasFlash);
        visitor("hasFaceDetection", hasFaceDetection);
        visitor("hasHdr", hasHdr);
        visitor("hasTouchFocus", hasTouchFocus);
        visitor("flat", flat);
    }
};



class FeatureInfo
{
public:
    std::map<std::string, bool> features;
    
    template<class F> void applyVisitor(F visitor) const
    {
        for (auto i = features.begin(); i != features.end(); ++i) {
            visitor(i->first, i->second);
        }
    }
};



class SensorInfo
{
public:
    std::map<std::string, std::string> sensors;
    
    template<class F> void applyVisitor(F visitor) const
    {
        for (auto i = sensors.begin(); i != sensors.end(); ++i) {
            visitor(i->first, i->second);
        }
    }
};



}

#endif // DEVICEINFO_H
