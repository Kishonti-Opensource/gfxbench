/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "clinfo.h"
#include "jsonvisitor.h"
#include "CL/clew.h"
#include "ng/log.h"
#include "ng/macro_utils.h"
#include <iterator>
#include <algorithm>

using namespace tfw;

namespace
{

std::string getPlatformInfoString(cl_platform_id id, cl_platform_info info)
{
    size_t bufferLength = 0;
    requireex(CL_SUCCESS == clGetPlatformInfo(id, info, 0, NULL, &bufferLength));
    std::vector<char> buffer(bufferLength);
    requireex(CL_SUCCESS == clGetPlatformInfo(id, info, buffer.size(), buffer.data(), NULL));
    return std::string(buffer.data());
}

std::string getDeviceInfoString(cl_device_id id, cl_device_info info)
{
    size_t bufferLength = 0;
    requireex(CL_SUCCESS == clGetDeviceInfo(id, info, 0, NULL, &bufferLength));
    std::vector<char> buffer(bufferLength);
    requireex(CL_SUCCESS == clGetDeviceInfo(id, info, buffer.size(), buffer.data(), NULL));
    return std::string(buffer.data());
}

extern "C" void CL_CALLBACK errorFn(const char *errinfo, const void *, size_t, void *)
{
    NGLOG_ERROR("OpenCL error callback: %s", errinfo);
}

NG_TABLE_START(TBL_CL_ERRORS)
    NG_TABLE_ITEM0(CL_SUCCESS)
    NG_TABLE_ITEM0(CL_DEVICE_NOT_FOUND)
    NG_TABLE_ITEM0(CL_DEVICE_NOT_AVAILABLE)
    NG_TABLE_ITEM0(CL_COMPILER_NOT_AVAILABLE)
    NG_TABLE_ITEM0(CL_MEM_OBJECT_ALLOCATION_FAILURE)
    NG_TABLE_ITEM0(CL_OUT_OF_RESOURCES)
    NG_TABLE_ITEM0(CL_OUT_OF_HOST_MEMORY)
    NG_TABLE_ITEM0(CL_PROFILING_INFO_NOT_AVAILABLE)
    NG_TABLE_ITEM0(CL_MEM_COPY_OVERLAP)
    NG_TABLE_ITEM0(CL_IMAGE_FORMAT_MISMATCH)
    NG_TABLE_ITEM0(CL_IMAGE_FORMAT_NOT_SUPPORTED)
    NG_TABLE_ITEM0(CL_BUILD_PROGRAM_FAILURE)
    NG_TABLE_ITEM0(CL_MAP_FAILURE)
    NG_TABLE_ITEM0(CL_MISALIGNED_SUB_BUFFER_OFFSET)
    NG_TABLE_ITEM0(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
    NG_TABLE_ITEM0(CL_COMPILE_PROGRAM_FAILURE)
    NG_TABLE_ITEM0(CL_LINKER_NOT_AVAILABLE)
    NG_TABLE_ITEM0(CL_LINK_PROGRAM_FAILURE)
    NG_TABLE_ITEM0(CL_DEVICE_PARTITION_FAILED)
    NG_TABLE_ITEM0(CL_KERNEL_ARG_INFO_NOT_AVAILABLE)
    NG_TABLE_ITEM0(CL_INVALID_VALUE)
    NG_TABLE_ITEM0(CL_INVALID_DEVICE_TYPE)
    NG_TABLE_ITEM0(CL_INVALID_PLATFORM)
    NG_TABLE_ITEM0(CL_INVALID_DEVICE)
    NG_TABLE_ITEM0(CL_INVALID_CONTEXT)
    NG_TABLE_ITEM0(CL_INVALID_QUEUE_PROPERTIES)
    NG_TABLE_ITEM0(CL_INVALID_COMMAND_QUEUE)
    NG_TABLE_ITEM0(CL_INVALID_HOST_PTR)
    NG_TABLE_ITEM0(CL_INVALID_MEM_OBJECT)
    NG_TABLE_ITEM0(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
    NG_TABLE_ITEM0(CL_INVALID_IMAGE_SIZE)
    NG_TABLE_ITEM0(CL_INVALID_SAMPLER)
    NG_TABLE_ITEM0(CL_INVALID_BINARY)
    NG_TABLE_ITEM0(CL_INVALID_BUILD_OPTIONS)
    NG_TABLE_ITEM0(CL_INVALID_PROGRAM)
    NG_TABLE_ITEM0(CL_INVALID_PROGRAM_EXECUTABLE)
    NG_TABLE_ITEM0(CL_INVALID_KERNEL_NAME)
    NG_TABLE_ITEM0(CL_INVALID_KERNEL_DEFINITION)
    NG_TABLE_ITEM0(CL_INVALID_KERNEL)
    NG_TABLE_ITEM0(CL_INVALID_ARG_INDEX)
    NG_TABLE_ITEM0(CL_INVALID_ARG_VALUE)
    NG_TABLE_ITEM0(CL_INVALID_ARG_SIZE)
    NG_TABLE_ITEM0(CL_INVALID_KERNEL_ARGS)
    NG_TABLE_ITEM0(CL_INVALID_WORK_DIMENSION)
    NG_TABLE_ITEM0(CL_INVALID_WORK_GROUP_SIZE)
    NG_TABLE_ITEM0(CL_INVALID_WORK_ITEM_SIZE)
    NG_TABLE_ITEM0(CL_INVALID_GLOBAL_OFFSET)
    NG_TABLE_ITEM0(CL_INVALID_EVENT_WAIT_LIST)
    NG_TABLE_ITEM0(CL_INVALID_EVENT)
    NG_TABLE_ITEM0(CL_INVALID_OPERATION)
    NG_TABLE_ITEM0(CL_INVALID_GL_OBJECT)
    NG_TABLE_ITEM0(CL_INVALID_BUFFER_SIZE)
    NG_TABLE_ITEM0(CL_INVALID_MIP_LEVEL)
    NG_TABLE_ITEM0(CL_INVALID_GLOBAL_WORK_SIZE)
    NG_TABLE_ITEM0(CL_INVALID_PROPERTY)
    NG_TABLE_ITEM0(CL_INVALID_IMAGE_DESCRIPTOR)
    NG_TABLE_ITEM0(CL_INVALID_COMPILER_OPTIONS)
    NG_TABLE_ITEM0(CL_INVALID_LINKER_OPTIONS)
    NG_TABLE_ITEM0(CL_INVALID_DEVICE_PARTITION_COUNT)
    NG_TABLE_ITEM0(CL_INVALID_PIPE_SIZE)
    NG_TABLE_ITEM0(CL_INVALID_DEVICE_QUEUE)
NG_TABLE_END(TBL_CL_ERRORS)


NG_TABLE_START(TBL_DEVICE_TYPE)
    NG_TABLE_ITEM0(CL_DEVICE_TYPE_CPU)
    NG_TABLE_ITEM0(CL_DEVICE_TYPE_GPU)
    NG_TABLE_ITEM0(CL_DEVICE_TYPE_ACCELERATOR)
    NG_TABLE_ITEM0(CL_DEVICE_TYPE_DEFAULT)
NG_TABLE_END(TBL_DEVICE_TYPE)

NG_TABLE_START(TBL_MEM_CACHE_TYPE)
    NG_TABLE_ITEM0(CL_NONE)
    NG_TABLE_ITEM0(CL_READ_ONLY_CACHE)
    NG_TABLE_ITEM0(CL_READ_WRITE_CACHE)
NG_TABLE_END(TBL_MEM_CACHE_TYPE)

NG_TABLE_START(TBL_LOCAL_MEM_TYPE)
    NG_TABLE_ITEM0(CL_LOCAL)
    NG_TABLE_ITEM0(CL_GLOBAL)
NG_TABLE_END(TBL_LOCAL_MEM_TYPE)

NG_TABLE_START(TBL_IMG_CHANNEL_ORDER)
    NG_TABLE_ITEM0(CL_R)
    NG_TABLE_ITEM0(CL_A)
    NG_TABLE_ITEM0(CL_RG)
    NG_TABLE_ITEM0(CL_RA)
    NG_TABLE_ITEM0(CL_RGB)
    NG_TABLE_ITEM0(CL_RGBA)
    NG_TABLE_ITEM0(CL_BGRA)
    NG_TABLE_ITEM0(CL_ARGB)
    NG_TABLE_ITEM0(CL_INTENSITY)
    NG_TABLE_ITEM0(CL_LUMINANCE)
    NG_TABLE_ITEM0(CL_Rx)
    NG_TABLE_ITEM0(CL_RGx)
    NG_TABLE_ITEM0(CL_RGBx)
    NG_TABLE_ITEM0(CL_DEPTH)
    NG_TABLE_ITEM0(CL_DEPTH_STENCIL)
    NG_TABLE_ITEM0(CL_sRGB)
    NG_TABLE_ITEM0(CL_sRGBx)
    NG_TABLE_ITEM0(CL_sRGBA)
    NG_TABLE_ITEM0(CL_sBGRA)
    NG_TABLE_ITEM0(CL_ABGR)
NG_TABLE_END(TBL_IMG_CHANNEL_ORDER)

NG_TABLE_START(TBL_IMG_DATA_TYPE)
    NG_TABLE_ITEM0(CL_SNORM_INT8)
    NG_TABLE_ITEM0(CL_SNORM_INT8)
    NG_TABLE_ITEM0(CL_SNORM_INT16)
    NG_TABLE_ITEM0(CL_UNORM_INT8)
    NG_TABLE_ITEM0(CL_UNORM_INT16)
    NG_TABLE_ITEM0(CL_UNORM_SHORT_565)
    NG_TABLE_ITEM0(CL_UNORM_SHORT_555)
    NG_TABLE_ITEM0(CL_UNORM_INT_101010)
    NG_TABLE_ITEM0(CL_SIGNED_INT8)
    NG_TABLE_ITEM0(CL_SIGNED_INT16)
    NG_TABLE_ITEM0(CL_SIGNED_INT32)
    NG_TABLE_ITEM0(CL_UNSIGNED_INT8)
    NG_TABLE_ITEM0(CL_UNSIGNED_INT16)
    NG_TABLE_ITEM0(CL_UNSIGNED_INT32)
    NG_TABLE_ITEM0(CL_HALF_FLOAT)
    NG_TABLE_ITEM0(CL_FLOAT)
    NG_TABLE_ITEM0(CL_UNORM_INT24)
NG_TABLE_END(TBL_IMG_DATA_TYPE)

NG_TABLE_START(TBL_MEM_FLAGS)
    NG_TABLE_ITEM0(CL_MEM_READ_ONLY)
    NG_TABLE_ITEM0(CL_MEM_WRITE_ONLY)
    NG_TABLE_ITEM0(CL_MEM_READ_WRITE)
    NG_TABLE_ITEM0(CL_MEM_USE_HOST_PTR)
    NG_TABLE_ITEM0(CL_MEM_ALLOC_HOST_PTR)
    NG_TABLE_ITEM0(CL_MEM_COPY_HOST_PTR)
NG_TABLE_END(TBL_MEM_FLAGS)

NG_TABLE_START(TBL_MEM_OBJECT_TYPE)
    NG_TABLE_ITEM0(CL_MEM_OBJECT_BUFFER)
    NG_TABLE_ITEM0(CL_MEM_OBJECT_IMAGE2D)
    NG_TABLE_ITEM0(CL_MEM_OBJECT_IMAGE3D)
NG_TABLE_END(TBL_MEM_OBJECT_TYPE)

}

CLInfoCollector::CLInfoCollector()
{}

void CLInfoCollector::collect()
{
    cl_int infoStatus;
    int err = clewInit(0, &driverPath_);
    if (err)
    {
        NGLOG_DEBUG("clewInit failed: %s", err);
        return;
    }

    // get OpenCL platform count
    cl_uint numPlatforms = 0;
    infoStatus = clGetPlatformIDs( 0, NULL, &numPlatforms );
    if (infoStatus != CL_SUCCESS)
    {
        NGLOG_WARN("clGetPlatformIDs returned %s (platform #: %s)", infoStatus, numPlatforms );
    }

    std::vector<cl_platform_id> platformIDs(numPlatforms);
    if (numPlatforms > 0)
    {
        infoStatus = clGetPlatformIDs(static_cast<cl_uint>( platformIDs.size() ), platformIDs.data(), NULL);
        assert(infoStatus == CL_SUCCESS);
    }

    for(size_t p = 0; p < platformIDs.size(); ++p)
    {
        CLPlatformInfo platform;
        platform.vendor = getPlatformInfoString(platformIDs[p], CL_PLATFORM_VENDOR);
        platform.name = getPlatformInfoString(platformIDs[p], CL_PLATFORM_NAME);
        platform.version = getPlatformInfoString(platformIDs[p], CL_PLATFORM_VERSION);
        platform.profile = getPlatformInfoString(platformIDs[p], CL_PLATFORM_PROFILE);
        std::istringstream extensions(getPlatformInfoString(platformIDs[p], CL_PLATFORM_EXTENSIONS));
        std::copy(std::istream_iterator<std::string>(extensions),
                  std::istream_iterator<std::string>(),
                  std::back_inserter <std::vector<std::string> >(platform.extensions));
        std::sort(platform.extensions.begin(), platform.extensions.end());

        // get device count
        cl_uint numDevices;
        infoStatus = clGetDeviceIDs( platformIDs[p], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices );
        assert(!infoStatus);

#if TARGET_OS_IPHONE
        // Seems on iOS devices it returns 2 devices, but crashes on querying the second one.
        numDevices = 1;
#endif
        std::vector<cl_device_id> deviceIDs(numDevices);
        infoStatus = clGetDeviceIDs(platformIDs[p], CL_DEVICE_TYPE_ALL, static_cast<cl_uint>( deviceIDs.size() ), deviceIDs.data(), NULL);
        assert(!infoStatus);

        for(size_t d = 0; d < deviceIDs.size(); ++d )
        {
            CLDeviceInfo device;
            device.vendor = getDeviceInfoString(deviceIDs[d], CL_DEVICE_VENDOR);
            device.name = getDeviceInfoString(deviceIDs[d], CL_DEVICE_NAME);
            device.version = getDeviceInfoString(deviceIDs[d], CL_DEVICE_VERSION);
            device.openclCVersion = getDeviceInfoString(deviceIDs[d], CL_DEVICE_OPENCL_C_VERSION);
            device.driverVersion = getDeviceInfoString(deviceIDs[d], CL_DRIVER_VERSION);
            cl_bool canCompile;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_COMPILER_AVAILABLE, sizeof(canCompile), &canCompile, NULL);
            assert(!infoStatus);
            device.compilerAvailable = canCompile != 0;

            cl_uint addressbits;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_ADDRESS_BITS, sizeof(addressbits), &addressbits, NULL);
            assert(!infoStatus);
            device.deviceAddressBits = addressbits;

            cl_device_type device_type;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_TYPE, sizeof(device_type), &device_type, NULL);
            assert(!infoStatus);
            device.deviceType = TBL_DEVICE_TYPE.bitmask(static_cast<long>(device_type));

            cl_uint maxCompUnits;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxCompUnits), &maxCompUnits, NULL);
            assert(!infoStatus);
            device.maxComputeUnits = maxCompUnits;

            cl_uint maxClockFreq;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(maxClockFreq), &maxClockFreq, NULL);
            assert(!infoStatus);
            device.maxClockFrequency = maxClockFreq;

            cl_ulong maxGlobalMemory;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(maxGlobalMemory), &maxGlobalMemory, NULL);
            assert(!infoStatus);
            device.deviceGlobalMemSize = maxGlobalMemory;

            cl_ulong maxMemAlloc;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(maxMemAlloc), &maxMemAlloc, NULL);
            assert(!infoStatus);
            device.maxMemAllocSize = maxMemAlloc;

            cl_device_mem_cache_type mem_cache_type;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(mem_cache_type), &mem_cache_type, NULL);
            assert(!infoStatus);
            device.globalMemCacheType = TBL_MEM_CACHE_TYPE(mem_cache_type);

            cl_ulong memCacheSize;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(memCacheSize), &memCacheSize, NULL);
            assert(!infoStatus);
            device.globalMemCacheSize = memCacheSize;

            cl_uint memCacheLineSize;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(memCacheLineSize), &memCacheLineSize, NULL);
            assert(!infoStatus);
            device.globalMemCachelineSize = memCacheLineSize;

            cl_device_local_mem_type local_mem_type;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_LOCAL_MEM_TYPE, sizeof(local_mem_type), &local_mem_type, NULL);
            assert(!infoStatus);
            device.localMemType = TBL_LOCAL_MEM_TYPE(local_mem_type);

            cl_ulong maxLocalMemory;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(maxLocalMemory), &maxLocalMemory, NULL);
            assert(!infoStatus);
            device.localMemSize = maxLocalMemory;

            cl_ulong maxConstantBufferSize;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(maxConstantBufferSize), &maxConstantBufferSize, NULL);
            assert(!infoStatus);
            device.maxConstantBufferSize = maxConstantBufferSize;

            cl_uint maxWorkItemDims;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(maxWorkItemDims), &maxWorkItemDims, NULL);
            assert(!infoStatus);
            device.maxWorkItemDimensions = maxWorkItemDims;

            size_t maxWorkItemSizes[3];
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(maxWorkItemSizes), &maxWorkItemSizes, NULL);
            assert(!infoStatus);
            for (int i = 0; i < 3; ++i) {
                device.maxWorkItemSizes[i] = maxWorkItemSizes[i];
            }

            size_t maxWorkGroupSize;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWorkGroupSize), &maxWorkGroupSize, NULL);
            assert(!infoStatus);
            device.maxWorkGorupSize = maxWorkGroupSize;

            cl_bool hasImageSupport;
            infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_IMAGE_SUPPORT, sizeof(hasImageSupport), &hasImageSupport, NULL);
            assert(!infoStatus);
            device.hasImageSupport = hasImageSupport != 0;

            if(hasImageSupport)
            {
                size_t imageSize;

                infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(imageSize), &imageSize, NULL);
                assert(!infoStatus);
                device.image2DMaxSize[0] = imageSize;
                infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(imageSize), &imageSize, NULL);
                assert(!infoStatus);
                device.image2DMaxSize[1] = imageSize;

                infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(imageSize), &imageSize, NULL);
                assert(!infoStatus);
                device.image3DMaxSize[0] = imageSize;
                infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(imageSize), &imageSize, NULL);
                assert(!infoStatus);
                device.image3DMaxSize[1] = imageSize;
                infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(imageSize), &imageSize, NULL);
                assert(!infoStatus);
                device.image3DMaxSize[2] = imageSize;

                cl_uint maxReadImageArgs;
                infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(maxReadImageArgs), &maxReadImageArgs, NULL);
                assert(!infoStatus);
                device.maxReadImageArgs = maxReadImageArgs;

                cl_uint maxWriteImageArgs;
                infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(maxWriteImageArgs), &maxWriteImageArgs, NULL);
                assert(!infoStatus);
                device.maxWriteImageArgs = maxWriteImageArgs;

                cl_uint maxSamplers;
                infoStatus = clGetDeviceInfo(deviceIDs[d], CL_DEVICE_MAX_SAMPLERS, sizeof(maxSamplers), &maxSamplers, NULL);
                assert(!infoStatus);
                device.maxSamplers = maxSamplers;
            }

            int level = 1;
            if (level > 0)
            {
                int err = -1;
                cl_context_properties ctx_props[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platformIDs[p]), 0 };
                cl_context ctx = clCreateContext(ctx_props, 1, &deviceIDs[d], errorFn, NULL, &err);
                if (err == CL_SUCCESS)
                {
                    cl_mem_flags flags[] = { CL_MEM_READ_WRITE, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY };
                    cl_mem_object_type obj_types[] = { CL_MEM_OBJECT_IMAGE2D, CL_MEM_OBJECT_IMAGE3D };
                    for (size_t i = 0; i < sizeof(flags)/sizeof(flags[0]); ++i)
                    {
                        for (size_t j = 0; j < sizeof(obj_types)/sizeof(obj_types[0]); ++j)
                        {
                            cl_mem_flags flag = flags[i];
                            cl_mem_object_type obj_type = obj_types[j];
                            cl_uint numImageFormats = 0;
                            cl_int status = clGetSupportedImageFormats(ctx, flag, obj_type, 0, NULL, &numImageFormats);
                            if (numImageFormats > 0 && status == CL_SUCCESS)
                            {
                                std::vector<cl_image_format> imageFormats(numImageFormats);
                                status = clGetSupportedImageFormats(ctx, flag, obj_type, (cl_uint)imageFormats.size(), imageFormats.data(), &numImageFormats);
                                requireex(status == CL_SUCCESS);
                                for (size_t k = 0; k < imageFormats.size(); ++k)
                                {
                                    cl_image_format fmt = imageFormats[k];
                                    CLImageFormat f =
                                    {
                                        TBL_MEM_FLAGS.bitmask((long)flag),
                                        TBL_MEM_OBJECT_TYPE(obj_type),
                                        TBL_IMG_CHANNEL_ORDER(fmt.image_channel_order),
                                        TBL_IMG_DATA_TYPE(fmt.image_channel_data_type)
                                    };
                                    device.imageFormats.push_back(f);
                                }
                            }
                        }
                    }
                    clReleaseContext(ctx);
                }
                else
                {
                    device.error = "clCreateContext: " + TBL_CL_ERRORS(err);
                }
            }

            std::istringstream deviceExtensions (getDeviceInfoString(deviceIDs[d], CL_DEVICE_EXTENSIONS));
            std::copy(std::istream_iterator<std::string>(deviceExtensions),
                      std::istream_iterator<std::string>(),
                      std::back_inserter <std::vector<std::string> >(device.extensions));
            std::sort(device.extensions.begin(), device.extensions.end());
            platform.devices.push_back(device);
        }
        platforms_.push_back(platform);
    }
}


size_t CLInfoCollector::platformCount() const
{
    return platforms_.size();
}


const CLPlatformInfo &CLInfoCollector::platform(int i) const
{
    return platforms_.at(i);
}


bool CLInfoCollector::isOpenCLAvailable() const
{
    // TODO:
    return !platforms_.empty();
}

class ClJsonVisitor : public JsonVisitor
{
public:
    ClJsonVisitor(ng::JsonValue& jsonValue) : JsonVisitor(jsonValue) {}
    using JsonVisitor::operator();
    void operator()(const std::string& name, const std::vector<CLDeviceInfo>& value) {
        ng::JsonValue array;
        array.resize(0);
        for (size_t i = 0; i < value.size(); ++i) {
            ng::JsonValue object;
            applyVisitor(value.at(i), ClJsonVisitor(object));
            array.push_back(object);
        }
        mJsonValue[name.c_str()] = array;
    }
    void operator()(const std::string& name, const std::vector<CLImageFormat>& value) {
        ng::JsonValue array;
        array.resize(0);
        for (size_t i = 0; i < value.size(); ++i) {
            ng::JsonValue object;
            applyVisitor(value.at(i), ClJsonVisitor(object));
            array.push_back(object);
        }
        mJsonValue[name.c_str()] = array;
    }
};

std::string CLInfoCollector::serialize() const
{
    std::string json = "{}";
    NGLOG_DEBUG("OpenCL available %s", isOpenCLAvailable());
    if (isOpenCLAvailable()) {
        ng::JsonValue root;
        ng::JsonValue jp;
        jp.resize(0);
        for(size_t i = 0; i < platforms_.size(); ++i) {
            ng::JsonValue p;
            applyVisitor(platform(static_cast<int>(i)), ClJsonVisitor(p));
            jp.push_back(p);
        }
        root["platforms"] = jp;
        root["driver_path"] = driverPath_;
        json = root.toString();
    }
    return json;
}

std::string CLInfoCollector::driverPath() const
{
    return driverPath_;
}
