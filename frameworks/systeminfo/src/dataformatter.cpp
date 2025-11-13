/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  dataformatter.cpp
//  GFXBench
//
//  Created by Kishonti Kft on 13/11/2013.
//
//

#include "dataformatter.h"
#include "SystemInfoCommonKeys.h"
#include "utils.h"
#include "ng/json.h"
#include <cmath>
#include <iomanip>

using namespace sysinf;

std::string coreStrings[] = {
    "single", "dual", "triple", "quad", "penta",
    "hexa", "hepta", "octa", "nona", "deca",
    "hendeca", "dodeca", "trideca", "tetradeca", "pentadeca",
    "hexadeca", "heptadeca", "octadeca", "enneadeca", "icosa"
};

std::string getNamedVidResolution(int width, int height)
{
    switch (width) {
        case 160: if(height == 120)
            return "QQVGA";

        case 240: if(height == 160)
            return "HQVGA";

        case 320: if(height == 240)
            return "QVGA";

        case 400: if(height == 240)
            return "WQVGA";

        case 480: if(height == 320)
            return "HVGA";

        case 640: if(height == 480)
            return "VGA";
        else if(height == 360)
            return "nHD";

        case 800: if(height == 480)
            return "WVGA";
        else if(height == 600)
            return "SVGA";

        case 854: if(height == 480)
            return "FWVGA";

        case 960: if(height == 640)
            return "DVGA";
        else if(height == 540)
            return "qHD";

        case 1024: if(height == 576)
            return "WSVGA";
        else if(height == 600)
            return "WSVGA";
        else if(height == 768)
            return "XGA";

        case 1360: if(height == 768)
            return "WXGA";

        case 1366: if(height == 768)
            return "WXGA";

        case 1280: if(height == 800)
            return "WXGA";
        else if(height == 1024)
            return "SXGA";
        else if(height == 720)
            return "HD";

        case 1152: if(height == 864)
            return "XGA+";

        case 1440: if(height == 900)
            return "WXGA+";

        case 1400: if(height == 1050)
            return "SXGA+";

        case 1680: if(height == 1050)
            return "WSXGA+";

        case 1600: if(height == 1200)
            return "UXGA";

        case 1920: if(height == 1200)
            return "WUXGA";
        else if(height == 1080)
            return "FHD";
        else if(height == 1088)
            return "FHD";

        case 2048: if(height == 1152)
            return "QWXGA";
        else if(height == 1536)
            return "QXGA";

        case 2560: if(height == 1600)
            return "WQXGA";
        else if(height == 2048)
            return "QSXGA";
        else if(height == 1440)
            return "QHD";

        case 3200: if(height == 2048)
            return "WQSXGA";
        else if(height == 2400)
            return "QUXGA";
        else if(height == 1800)
            return "WQXGA+";

        case 3840: if(height == 2400)
            return "WQUXGA";
        else if(height == 2160)
            return "UHD (4K)";

        case 4096: if(height == 3072)
            return "HXGA";

        case 5120: if(height == 3200)
            return "WHXGA";
        else if(height == 4096)
            return "HSXGA";

        case 6400: if(height == 4096)
            return "WHSXGA";
        else if(height == 4800)
            return "HUXGA";

        case 7680: if(height == 4800)
            return "WHUXGA";
        else if(height == 4320)
            return "UHD (8K)";

        default:
            return "";
    }
}

bool DataFormatter::AutoSetMajor(const Properties *const props, FormattedDeviceInfo *info, const std::string &key) const
{
    std::stringstream ss;
    ss << key << "/major";

    if(props->has(ss.str()))
    {
        info->SetMajor(props->get(ss.str())->getString());
        return true;
    }

    return false;
}

bool DataFormatter::AutoSetMinor(const Properties *const props, FormattedDeviceInfo *info, const std::string &key) const
{
    std::stringstream ss;
    ss << key << "/minor";

    if(props->has(ss.str()))
    {
        info->SetMinor(props->get(ss.str())->getString());
        return true;
    }

    return false;
}

bool DataFormatter::AutoSetIndexedMajor(const Properties *const props, FormattedDeviceInfo *info, const std::string &key, int index) const
{
    std::string indexedKey = CreateIndexedKey(key, "major", index);
    if(props->has(indexedKey))
    {
        info->SetMajor(props->get(indexedKey)->getString());
        return true;
    }

    return false;
}

bool DataFormatter::AutoSetIndexedMinor(const Properties *const props, FormattedDeviceInfo *info, const std::string &key, int index) const
{
    std::string indexedKey = CreateIndexedKey(key, "minor", index);
    if(props->has(indexedKey))
    {
        info->SetMinor(props->get(indexedKey)->getString());
        return true;
    }

    return false;
}

ApiStream DataFormatter::GetApiStream(const std::string api_key) const
{
    ApiStream stream;

    if (api_key == "api/cuda") {
        ApiPlatform platform;
        int deviceCount = GetIntPropWithKey(m_properties, "api/cuda/devices/count");
        for (int j = 0; j < deviceCount; j++) {
            ApiDevice device;
            std::ostringstream ss;
            ss << "api/cuda/devices/" << j;
            std::string deviceKey = ss.str();
            ss.clear(); ss.str("");
            device.name = GetStringPropWithKey(m_properties, deviceKey + "/cuDeviceGetName");
            for (sysinf::PropertyIter FeatureIter = m_properties->groupIterator(deviceKey); !FeatureIter.done(); FeatureIter.next())
            {
                std::string s = FeatureIter.name();
                size_t pos = s.rfind("/");
                sysinf::ApiInfo info;
                info.name = s.substr(pos + 1);
                info.info = GetStringPropWithKey(m_properties, FeatureIter.name());
                if (info.info != "")
                    device.infos.push_back(info);
            }
            platform.devices.push_back(device);
        }
        stream.push_back(platform);
    } else if(api_key == "api/cl") {
        // CL info needs platforms and devices to be queried
        int platformCount = GetIntPropWithKey(m_properties, "api/cl/platforms/count");
        for(int i(0); i < platformCount; i++)
        {
            ApiPlatform platform;
            std::stringstream ss;
            ss << "api/cl/platforms/" << i;
            std::string platformKey = ss.str();
            ss.clear(); ss.str("");

            platform.name = GetStringPropWithKey(m_properties, platformKey + "/" + API_CL_PLATFORM_NAME);
            int deviceCount = GetIntPropWithKey(m_properties, platformKey + "/devices/count");

            for(int j(0); j < deviceCount; j++)
            {
                ApiDevice device;
                ss << platformKey << "/devices/" << j;
                std::string deviceKey = ss.str();
                ss.clear(); ss.str("");

                device.name = GetStringPropWithKey(m_properties, deviceKey + "/" + API_CL_DEVICE_NAME);

                for(sysinf::PropertyIter FeatureIter = m_properties->groupIterator(deviceKey);!FeatureIter.done(); FeatureIter.next())
                {
                    std::string s = FeatureIter.name();
                    size_t pos = s.rfind("/");
                    sysinf::ApiInfo info;

                    info.name = s.substr(pos+1);
                    info.info = GetStringPropWithKey(m_properties, FeatureIter.name());
                    if(info.info != "")
                        device.infos.push_back(info);
                }

                platform.devices.push_back(device);
            }
            stream.push_back(platform);
        }
    } else {
        // Other than CL needs only something similar in platform/device
        ApiPlatform platform;
        platform.name = GetStringPropWithKey(m_properties, DEVICE_NAME);
        ApiDevice device;
        std::string iterator_base = api_key;
        if((api_key == API_GL) || (api_key == "api/gles")) {
            iterator_base = api_key + "/features";
            device.name = GetStringPropWithKey(m_properties, api_key + "/GL_RENDERER");
        } else if(api_key == API_METAL) {
            device.name = GetStringPropWithKey(m_properties, "api/metal/FEATURE_SET");
        }

        for(sysinf::PropertyIter FeatureIter = m_properties->groupIterator(iterator_base);!FeatureIter.done(); FeatureIter.next())
        {
            std::string s = FeatureIter.name();
            size_t pos = s.rfind("/");
            sysinf::ApiInfo info;

            info.name = s.substr(pos+1);
            info.info = GetStringPropWithKey(m_properties, FeatureIter.name());
            if(info.info != "")
                device.infos.push_back(info);
        }

        platform.devices.push_back(device);
        stream.push_back(platform);
    }

    return stream;
}

std::vector<std::string> DataFormatter::GetNotSupportedTests() const
{
    std::vector<std::string> testList;

    for(sysinf::PropertyIter testIter = m_properties->groupIterator(TEST_NOT_SUPPORTED);!testIter.done(); testIter.next())
    {
        std::string s = testIter.name();
        size_t pos = s.rfind("/");
        testList.push_back(s.substr(pos+1));
    }

    return testList;
}

std::vector<std::string> DataFormatter::GetHiddenTests() const
{
    std::vector<std::string> testList;

    for(sysinf::PropertyIter testIter = m_properties->groupIterator(TEST_NOT_SUPPORTED);!testIter.done(); testIter.next())
    {
        int hidden = GetIntPropWithKey(m_properties, testIter.name());
        if(hidden > 0) {
            std::string s = testIter.name();
            size_t pos = s.rfind("/");
            testList.push_back(s.substr(pos+1));
        }
    }

    return testList;
}

FormattedStream DataFormatter::GetFormattedStream() const
{
    FormattedStream stream;

    bool ios = m_properties->has("device/id");

    FormattedDeviceInfo device;
    device.SetName("Device");
    if(!AutoSetMajor(m_properties, &device, DEVICE))
    {
        if(ios)
        {
            device.SetMajor(GetStringPropWithKey(m_properties, DEVICE_NAME));
        }
        else
        {
            device.SetMajor(GetStringPropWithKey(m_properties, "os/build_details/GLBPD_OS_BUILD_MODEL"));
        }
    }
    if(!AutoSetMinor(m_properties, &device, DEVICE))
    {
        device.SetMinor("");
    }
    stream.push_back(device);

    FormattedDeviceInfo os;
    os.SetName("OS");
    if(!AutoSetMajor(m_properties, &os, OS))
    {
        std::string name = GetStringPropWithKey(m_properties, OS_NAME);
        std::string build = GetStringPropWithKey(m_properties, OS_BUILD);

        std::stringstream ss;
        ss << name << " " << build;
        os.SetMajor(ss.str());
    }
    if(!AutoSetMinor(m_properties, &os, OS))
    {
        os.SetMinor("");
    }
    stream.push_back(os);

    FormattedDeviceInfo display;
    display.SetName("Display");
    int displayCount = GetIntPropWithKey(m_properties, DISPLAY_COUNT);
    if(displayCount == 0) //one display or not set
    {
        if(!AutoSetMajor(m_properties, &display, DISPLAY))
        {
            std::stringstream ss;
            ss.str("");
            bool hasResolution = m_properties->has(DISPLAY_RES_X) && m_properties->has(DISPLAY_RES_Y);
            if (hasResolution)
            {
                int x_res = GetIntPropWithKey(m_properties, DISPLAY_RES_X);
                int y_res = GetIntPropWithKey(m_properties, DISPLAY_RES_Y);
                double x_dpi = GetDoublePropWithKey(m_properties, DISPLAY_DPI_X);
                double y_dpi = GetDoublePropWithKey(m_properties, DISPLAY_DPI_Y);

                int x = x_res > y_res ? x_res : y_res;
                int y = x_res > y_res ? y_res : x_res;

                x_dpi = x_dpi != 0 ? x_dpi : 1;
                y_dpi = y_dpi != 0 ? y_dpi : 1;

                float inch = powf( powf(((float)x_res/(float)x_dpi), 2.0f) + powf(((float)y_res/(float)y_dpi), 2.0f), 0.5f);

                ss << x << " x " << y <<  ", " << std::setprecision(2) << inch << "\"";
            }

            display.SetMajor(ss.str());
        }

        if(!AutoSetMinor(m_properties, &display, DISPLAY))
        {
            display.SetMinor("");
        }
        stream.push_back(display);
    }
    else //one or more displays
    {
        for(int i = 0; i < displayCount; i++)
        {
            std::stringstream ss;
            if(!AutoSetIndexedMajor(m_properties, &display, DISPLAY, i))
            {
                ss.str("");

                std::string resXKey = CreateIndexedKey(DISPLAY, DISPLAY_RES_X_POSTFIX, i);
                std::string resYKey = CreateIndexedKey(DISPLAY, DISPLAY_RES_Y_POSTFIX, i);

                bool hasResolution = m_properties->has(resXKey) && m_properties->has(resYKey);
                if(hasResolution)
                {
                    int64_t x_res = GetLongPropWithKey(m_properties, resXKey);
                    int64_t y_res = GetLongPropWithKey(m_properties, resYKey);
                    ss << x_res << "x" << y_res;
                }

                std::string diagonalKey = CreateIndexedKey(DISPLAY, DISPLAY_DIAGONAL_POSTFIX, i);
                if(m_properties->has(diagonalKey))
                {
                    if (hasResolution)
                    {
                        ss << ", ";
                    }

                    double diagonal = GetDoublePropWithKey(m_properties, diagonalKey);
                    ss << diagonal << "\"";
                }

                display.SetMajor(ss.str());
            }

            if(!AutoSetIndexedMinor(m_properties, &display, DISPLAY, i))
            {
                std::string dpiXKey = CreateIndexedKey(DISPLAY, DISPLAY_DPI_X_POSTFIX, i);
                std::string dpiYKey = CreateIndexedKey(DISPLAY, DISPLAY_DPI_Y_POSTFIX, i);
                if(m_properties->has(dpiXKey) && m_properties->has(dpiYKey))
                {
                    int x_dpi = GetIntPropWithKey(m_properties, dpiXKey);
                    int y_dpi = GetIntPropWithKey(m_properties, dpiYKey);

                    ss.str("");
                    ss << ((x_dpi > y_dpi) ? x_dpi : y_dpi) << " ppi";
                    display.SetMinor(ss.str());
                }
                else
                {
                    display.SetMinor("");
                }
            }
            stream.push_back(display);
        }
    }

	int cpuCount = GetIntPropWithKey(m_properties, CPU_COUNT);
    if(cpuCount < 1) {
        FormattedDeviceInfo cpu;
        cpu.SetName("CPU");
        if(!AutoSetMajor(m_properties, &cpu, CPU))
        {
            std::stringstream ss;
            int cores = GetIntPropWithKey(m_properties, CPU_CORES);
            int frequency = GetIntPropWithKey(m_properties, CPU_FREQUENCY);

            if(cores > 0 && cores <= 20)
            {
                ss << coreStrings[cores-1] << " core";
            }
            else
            {
                ss << cores << " core";
            }

            if(cores > 0)
            {
                ss << " CPU";
                if(frequency > 0)
                {
                    ss << " @" << frequency << "MHz";
                }
            }

            cpu.SetMajor(ss.str());
        }
        if(!AutoSetMinor(m_properties, &cpu, CPU))
        {
            cpu.SetMinor("");
        }
        stream.push_back(cpu);
    } else {
        for(int i = 0; i < cpuCount; i++)
        {
            FormattedDeviceInfo cpu;
            cpu.SetName("CPU");
            if(!AutoSetIndexedMajor(m_properties, &cpu, CPU, i))
            {
                cpu.SetMajor(GetStringPropWithKey(m_properties, CreateIndexedKey(CPU, CPU_NAME, i)));
            }
            if(!AutoSetIndexedMinor(m_properties, &cpu, CPU, i))
            {
                cpu.SetMinor("");
            }

            stream.push_back(cpu);
        }
    }

    int gpuCount = GetIntPropWithKey(m_properties, GPU_COUNT);
    for(int i = 0; i < gpuCount; i++)
    {
        FormattedDeviceInfo gpu;
        gpu.SetName("GPU");
        if(!AutoSetIndexedMajor(m_properties, &gpu, GPU, i))
        {
            gpu.SetMajor(GetStringPropWithKey(m_properties, CreateIndexedKey(GPU, GPU_NAME, i)));
        }
        if(!AutoSetIndexedMinor(m_properties, &gpu, GPU, i))
        {
            gpu.SetMinor("");
        }

        stream.push_back(gpu);
    }

    if(m_properties->has("api/cl/platforms/count"))
    {
        int platformCount = GetIntPropWithKey(m_properties, "api/cl/platforms/count");
        for(int i(0); i < platformCount; i++)
        {
            std::stringstream ss;
            ss << "api/cl/platforms/" << i;
            std::string platformKey = ss.str();
            ss.clear(); ss.str("");
            int deviceCount = GetIntPropWithKey(m_properties, platformKey + "/devices/count");
            for(int j(0); j < deviceCount; j++)
            {
                FormattedDeviceInfo clApi;
                clApi.SetName("3D API - CL");
                ss << platformKey + "/devices/" << j;
                std::string deviceKey = ss.str();
                ss.clear(); ss.str("");
                std::string deviceName = GetStringPropWithKey(m_properties, deviceKey + "/" + API_CL_DEVICE_NAME);
                std::string platformName = GetStringPropWithKey(m_properties, platformKey + "/" + API_CL_PLATFORM_NAME);
				clApi.SetMajor(deviceName);
				clApi.SetMinor(platformName);
                stream.push_back(clApi);
            }
        }
    }

    if(m_properties->has("api/cuda/devices/count"))
    {
        int deviceCount = GetIntPropWithKey(m_properties, "api/cuda/devices/count");
        for(int j = 0; j < deviceCount; j++) {
            FormattedDeviceInfo cudaApi;
            cudaApi.SetName("3D API - CUDA");
            std::stringstream ss;
            ss << "api/cuda/devices/" << j;
            std::string deviceKey = ss.str();
            std::string deviceName = GetStringPropWithKey(m_properties, deviceKey + "/cuDeviceGetName");
            cudaApi.SetMajor(deviceName);
            cudaApi.SetMinor(GetStringPropWithKey(m_properties, deviceKey + "/cuDriverGetVersion"));
            stream.push_back(cudaApi);
        }
    }

	if(m_properties->has("api/vulkan/devices/count"))
    {
        int64_t deviceCount = GetLongPropWithKey(m_properties, "api/vulkan/devices/count");

        for(int j = 0; j < deviceCount; j++)
        {
            FormattedDeviceInfo vulkanApi;
            vulkanApi.SetName("3D API - VULKAN");
            std::stringstream ss;
            ss << "api/vulkan/devices/" << j;
            std::string deviceKey = ss.str();
            std::string deviceName = GetStringPropWithKey(m_properties, deviceKey + "/major");
            vulkanApi.SetMajor(deviceName);
            vulkanApi.SetMinor(GetStringPropWithKey(m_properties, deviceKey + "/minor"));
            stream.push_back(vulkanApi);
        }
    }
    FormattedDeviceInfo threeDApi;
    threeDApi.SetName("3D API - GL");
    if(!AutoSetMajor(m_properties, &threeDApi, API_3D))
    {
        if (m_properties->has("api/gl/features/GL_VERSION")) {
            threeDApi.SetMajor(GetStringPropWithKey(m_properties, API_GL_VERSION));
        }
        if (m_properties->has("api/gles/features/GL_VERSION")) {
            threeDApi.SetMajor(GetStringPropWithKey(m_properties, "api/gles/features/GL_VERSION"));
        }
    }
    if(!AutoSetMinor(m_properties, &threeDApi, API_3D))
    {
        threeDApi.SetMinor("");
    }
    stream.push_back(threeDApi);

    if(m_properties->has("api/metal/FEATURE_SET")) {
        FormattedDeviceInfo threeDApi_Metal;
        threeDApi_Metal.SetName("3D API - METAL");
        if(!AutoSetMajor(m_properties, &threeDApi_Metal, "api/metal"))
        {
            threeDApi_Metal.SetMajor(GetStringPropWithKey(m_properties, "api/metal/FEATURE_SET"));
        }
        if(!AutoSetMinor(m_properties, &threeDApi_Metal, "api/metal"))
        {
            threeDApi_Metal.SetMinor(GetStringPropWithKey(m_properties, "api/metal/DEVICE_NAME"));
        }
        stream.push_back(threeDApi_Metal);
    }

    FormattedDeviceInfo memory;
    memory.SetName("Memory");
    if(!AutoSetMajor(m_properties, &memory, MEMORY))
    {
        std::stringstream ss;
        ss.str("");
        if(m_properties->has(MEMORY_SIZE))
        {
            int64_t size = GetLongPropWithKey(m_properties, MEMORY_SIZE);
            double mbSize = size/(1024.0*1024.0);
            double gbsize = mbSize/1024.0;
            if(gbsize >= 1)
                ss << std::setprecision(3) << gbsize << " GB";
            else
                ss << mbSize << " MB";
        }
        memory.SetMajor(ss.str());
    }
    if(!AutoSetMinor(m_properties, &memory, MEMORY))
    {
        memory.SetMinor("");
    }
    stream.push_back(memory);


    FormattedDeviceInfo storage;
    storage.SetName("Storage");
    int64_t sum = 0;
    std::stringstream storagess;
    storagess << std::setprecision(3);
    int storageCount = GetIntPropWithKey(m_properties, STORAGE_COUNT);
    if (storageCount > 1)
    {
        for(int i = 0; i < storageCount; i++)
        {
            if(m_properties->has(CreateIndexedKey(STORAGE, STORAGE_SIZE, i)))
            {
                int64_t size = GetLongPropWithKey(m_properties, CreateIndexedKey(STORAGE, STORAGE_SIZE, i));
                sum += size;
                if(i < storageCount-1)
                    storagess << size/1024.0/1024.0/1024.0 << " + ";
                else
                    storagess << size/1024.0/1024.0/1024.0;
            }
        }
    }
    else
    {
        if(m_properties->has(CreateIndexedKey(STORAGE, STORAGE_SIZE, 0)))
        {
            int64_t size = GetLongPropWithKey(m_properties, CreateIndexedKey(STORAGE, STORAGE_SIZE, 0));
            sum += size;
        }
    }

    bool indexedMajorsAreSet = true;
    for(int i = 0; i < storageCount; i++)
    {
        if(!AutoSetIndexedMajor(m_properties, &storage, STORAGE, i))
        {
            indexedMajorsAreSet = false;
        }
        if(!AutoSetIndexedMinor(m_properties, &storage, STORAGE, i))
        {
        }

        if(indexedMajorsAreSet) {
            stream.push_back(storage);
        }
    }
    if(!indexedMajorsAreSet)
    {
        if(!AutoSetMajor(m_properties, &storage, STORAGE))
        {
            std::stringstream ss;
            ss.str("");
            double mbSize = sum/1024.0/1024.0;
            double gbsize = mbSize/1024.0;
            if(gbsize >= 1)
                ss << std::setprecision(3) << gbsize << " GB";
            else
                ss << mbSize << " MB";
            storage.SetMajor(ss.str());
        }
        if(!AutoSetMinor(m_properties, &storage, STORAGE))
        {
            storage.SetMinor(storagess.str());
        }
        stream.push_back(storage);
    }

    int cameraCount = GetIntPropWithKey(m_properties, CAMERA_COUNT);
    for(int i = 0; i < cameraCount; i++)
    {
        FormattedDeviceInfo camera;

        std::string cam_type = GetStringPropWithKey(m_properties, CreateIndexedKey(CAMERA, CAMERA_TYPE, i));
        if(cam_type == "CAMERA_TYPE_FRONT")
            camera.SetName("FrontCamera");
        else if(cam_type == "CAMERA_TYPE_BACK")
            camera.SetName("BackCamera");
        else
            camera.SetName("UnkownCamera");

        if(!AutoSetIndexedMajor(m_properties, &camera, CAMERA, i))
        {
            int pic_x = GetIntPropWithKey(m_properties, CreateIndexedKey(CAMERA, CAMERA_PIC_X, i));
            int pic_y = GetIntPropWithKey(m_properties, CreateIndexedKey(CAMERA, CAMERA_PIC_Y, i));
            int vid_x = GetIntPropWithKey(m_properties, CreateIndexedKey(CAMERA, CAMERA_VID_X, i));
            int vid_y = GetIntPropWithKey(m_properties, CreateIndexedKey(CAMERA, CAMERA_VID_Y, i));

            std::string pic_res_str = "";
            if(m_properties->has(CreateIndexedKey(CAMERA, CAMERA_PIC_X, i)) && m_properties->has(CreateIndexedKey(CAMERA, CAMERA_PIC_Y, i)))
            {
                std::stringstream ss;
                ss << pic_x << " x " << pic_y;
                pic_res_str = ss.str();
            }

            std::string vid_res_str = "";
            std::string vid = getNamedVidResolution(vid_x > vid_y ? vid_x : vid_y,
                                                    vid_x > vid_y ? vid_y : vid_x);

            if(m_properties->has(CreateIndexedKey(CAMERA, CAMERA_VID_X, i)) && m_properties->has(CreateIndexedKey(CAMERA, CAMERA_VID_Y, i)))
            {
                std::stringstream ss;
                ss << vid_x << " x " << vid_y;
                vid_res_str = ss.str();
            }

            std::stringstream ss2;
            if(m_properties->has(CreateIndexedKey(CAMERA, CAMERA_PIC_MP, i)))
            {
                double pic_mp = GetDoublePropWithKey(m_properties, CreateIndexedKey(CAMERA, CAMERA_PIC_MP, i));
                if(pic_res_str != "")
                {
                    ss2 << "picture: " << std::setprecision(2) << pic_mp << "MP (" << pic_res_str << ")";
                }
                else
                {
                    ss2 << "picture: " << std::setprecision(2) << pic_mp << "MP";
                }
            }
            else
            {
                if(pic_res_str != "")
                    ss2 << "picture: " << pic_res_str;
            }

            if(m_properties->has(CreateIndexedKey(CAMERA, CAMERA_VID_MP, i)))
            {
                if(ss2.str() != "")
                    ss2 << "\n";

                double vid_mp = GetDoublePropWithKey(m_properties, CreateIndexedKey(CAMERA, CAMERA_VID_MP, i));
                if(vid_res_str != "")
                {
                    if(vid == "")
                        ss2 << "video: " << std::setprecision(2) << vid_mp << "MP (" << vid_res_str << ")";
                    else
                        ss2 << "video: " << std::setprecision(2) << vid << " (" << vid_res_str << ")";
                }
                else
                {
                    if(vid == "")
                        ss2 << "video: " << std::setprecision(2) << vid_mp << "MP";
                    else
                        ss2 << "video: " << std::setprecision(2) << vid;
                }
            }
            else
            {
                if(vid_res_str != "")
                    ss2 << "video: " << vid_res_str;
            }

            camera.SetMajor(ss2.str());
        }
        if(!AutoSetIndexedMinor(m_properties, &camera, CAMERA, i))
        {
            camera.SetMinor("");
        }

        sysinf::PropertyIter serverIter = m_properties->groupIterator(CreateIndexedKey(CAMERA, CAMERA_SERVER, i));
        if(!serverIter.done())
        {
            for(;!serverIter.done(); serverIter.next())
            {
                std::string s = serverIter.name();
                size_t pos = s.rfind("/");
                if(pos != std::string::npos)
                {
                    camera.AddFeature(s.substr(pos+1), GetBoolPropWithKey(m_properties, serverIter.name()));
                }
            }
        }
        else
        {
            if(cam_type == "CAMERA_TYPE_BACK")
            {
                if(m_properties->has(CreateIndexedKey(CAMERA, CAMERA_HAS_AUTOFOCUS, i)))
                    camera.AddFeature("autofocus", GetBoolPropWithKey(m_properties, CreateIndexedKey(CAMERA, CAMERA_HAS_AUTOFOCUS, i)));
                if(m_properties->has(CreateIndexedKey(CAMERA, CAMERA_HAS_FACE_DETECTION, i)))
                    camera.AddFeature("face detection", GetBoolPropWithKey(m_properties, CreateIndexedKey(CAMERA, CAMERA_HAS_FACE_DETECTION, i)));
                if(m_properties->has(CreateIndexedKey(CAMERA, CAMERA_HAS_FLASH, i)))
                    camera.AddFeature("flash", GetBoolPropWithKey(m_properties, CreateIndexedKey(CAMERA, CAMERA_HAS_FLASH, i)));
                if(m_properties->has(CreateIndexedKey(CAMERA, CAMERA_HAS_GEO_TAGGING, i)))
                    camera.AddFeature("geo-tagging", GetBoolPropWithKey(m_properties, CreateIndexedKey(CAMERA, CAMERA_HAS_GEO_TAGGING, i)));
                if(m_properties->has(CreateIndexedKey(CAMERA, CAMERA_HAS_HDR, i)))
                    camera.AddFeature("HDR photo", GetBoolPropWithKey(m_properties, CreateIndexedKey(CAMERA, CAMERA_HAS_HDR, i)));
                if(m_properties->has(CreateIndexedKey(CAMERA, CAMERA_HAS_TOUCH_FOCUS, i)))
                    camera.AddFeature("touch focus", GetBoolPropWithKey(m_properties, CreateIndexedKey(CAMERA, CAMERA_HAS_TOUCH_FOCUS, i)));
            }
        }

        stream.push_back(camera);
    }

    FormattedDeviceInfo battery;
    battery.SetName("Battery");
    if(!AutoSetMajor(m_properties, &battery, BATTERY))
    {
        double level = GetDoublePropWithKey(m_properties, BATTERY_LEVEL);
        level = level >= 0 ? level : 0;
        bool isCharging = GetBoolPropWithKey(m_properties, BATTERY_IS_CHARGING);
        bool isConnected = GetBoolPropWithKey(m_properties, BATTERY_IS_CONNECTED);
        std::string status = "";
        if(!isConnected)
        {
            status = "BatteryUnplugged";
        }
        else
        {
            if(!isCharging)
                status = "BatteryPlugged";
            else
                status = "BatteryCharging";
        }

        std::stringstream ss;
        ss.str("");
        if(m_properties->has(BATTERY_LEVEL))
            ss << ((int)(level * 100)) << "%, " << status;
        else
            ss << status;

        battery.SetMajor(ss.str());
    }
    if(!AutoSetMinor(m_properties, &battery, BATTERY))
    {
        std::stringstream ss;
        ss.str("");

        if(m_properties->has(BATTERY_MAH))
        {
            int64_t mah = GetLongPropWithKey(m_properties, BATTERY_MAH);
            ss << mah << " mAh";
        }
        battery.SetMinor(ss.str());
    }
    if(!(GetStringPropWithKey(m_properties, BATTERY_MAJOR) == "NotPresent"))
    {
        stream.push_back(battery);
    }


    FormattedDeviceInfo features;
    features.SetName("Features");
    if(!AutoSetMajor(m_properties, &features, FEATURES))
    {
        features.SetMajor("");
    }
    if(!AutoSetMinor(m_properties, &features, FEATURES))
    {
        features.SetMinor("");
    }

    sysinf::PropertyIter serverFeatureIter = m_properties->groupIterator(FEATURES_SERVER);
    if(!serverFeatureIter.done())
    {
        for(;!serverFeatureIter.done(); serverFeatureIter.next())
        {
            std::string s = serverFeatureIter.name();
            size_t pos = s.rfind("/");
            if(pos != std::string::npos)
            {
                features.AddFeature(s.substr(pos+1), GetLongPropWithKey(m_properties, serverFeatureIter.name()) > 0);
            }
        }
    }
    else
    {
        if(m_properties->has(FEATURES_ACCELEROMETER)) features.AddFeature("accelerometer", GetBoolPropWithKey(m_properties, FEATURES_ACCELEROMETER));
        if(m_properties->has(FEATURES_ALTIMETER)) features.AddFeature("altimeter", GetBoolPropWithKey(m_properties, FEATURES_ALTIMETER));
        if(m_properties->has(FEATURES_BAROMETER)) features.AddFeature("barometer", GetBoolPropWithKey(m_properties, FEATURES_BAROMETER));
        if(m_properties->has(FEATURES_BLUETOOTH)) features.AddFeature("bluetooth", GetBoolPropWithKey(m_properties, FEATURES_BLUETOOTH));
        if(m_properties->has(FEATURES_FRONT_CAMERA)) features.AddFeature("camera (face)", GetBoolPropWithKey(m_properties, FEATURES_FRONT_CAMERA));
        if(m_properties->has(FEATURES_BACK_CAMERA)) features.AddFeature("camera (rear)", GetBoolPropWithKey(m_properties, FEATURES_BACK_CAMERA));
        if(m_properties->has(FEATURES_COMPASS)) features.AddFeature("compass", GetBoolPropWithKey(m_properties, FEATURES_COMPASS));
        if(m_properties->has(FEATURES_GPS)) features.AddFeature("gps", GetBoolPropWithKey(m_properties, FEATURES_GPS));
        if(m_properties->has(FEATURES_GYROSCOPE)) features.AddFeature("gyroscope", GetBoolPropWithKey(m_properties, FEATURES_GYROSCOPE));
        if(m_properties->has(FEATURES_LIGHTSENSOR)) features.AddFeature("lightsensor", GetBoolPropWithKey(m_properties, FEATURES_LIGHTSENSOR));
        if(m_properties->has(FEATURES_NFC)) features.AddFeature("nfc", GetBoolPropWithKey(m_properties, FEATURES_NFC));
        if(m_properties->has(FEATURES_PEDOMETER)) features.AddFeature("pedometer", GetBoolPropWithKey(m_properties, FEATURES_PEDOMETER));
        if(m_properties->has(FEATURES_PROXIMITY)) features.AddFeature("proximity", GetBoolPropWithKey(m_properties, FEATURES_PROXIMITY));
        if(m_properties->has(FEATURES_SIMCARDS)) features.AddFeature("simcards", GetBoolPropWithKey(m_properties, FEATURES_SIMCARDS));
        if(m_properties->has(FEATURES_THEMOMETER)) features.AddFeature("thermometer", GetBoolPropWithKey(m_properties, FEATURES_THEMOMETER));
        if(m_properties->has(FEATURES_WIFI)) features.AddFeature("wifi", GetBoolPropWithKey(m_properties, FEATURES_WIFI));
    }

    stream.push_back(features);



    return stream;
}

