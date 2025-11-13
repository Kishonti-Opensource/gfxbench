/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "clinfocollector.h"
#include "jsonvisitor.h"
#include "CL/clew.h"
#include "ng/log.h"
#include "ng/macro_utils.h"
#include <iterator>
#include <algorithm>

using namespace sysinf;

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
    std::string buffer;
    size_t bufferLength = 0;
    cl_int status = clGetDeviceInfo(id, info, 0, NULL, &bufferLength);
    if ((status != CL_SUCCESS) || (bufferLength == 0)) {
        return std::string();
    }
    buffer.resize(bufferLength);
    clGetDeviceInfo(id, info, buffer.size(), &buffer[0], &bufferLength);
    if ((status != CL_SUCCESS) || (bufferLength == 0)) {
        return std::string();
    }
    buffer.pop_back();
    return buffer;
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

//NG_TABLE_START(TBL_FP_CONFIG)
//    NG_TABLE_ITEM0(CL_FP_DENORM)
//    NG_TABLE_ITEM0(CL_FP_INF_NAN)
//    NG_TABLE_ITEM0(CL_FP_ROUND_TO_NEAREST)
//    NG_TABLE_ITEM0(CL_FP_ROUND_TO_ZERO)
//    NG_TABLE_ITEM0(CL_FP_ROUND_TO_INF)
//    NG_TABLE_ITEM0(CL_FP_FMA)
//    NG_TABLE_ITEM0(CL_FP_SOFT_FLOAT)
//    NG_TABLE_ITEM0(CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT)
//NG_TABLE_END(TBL_FP_CONFIG)
//
//NG_TABLE_START(TBL_EXEC_CAPABILITIES)
//    NG_TABLE_ITEM0(CL_EXEC_KERNEL)
//    NG_TABLE_ITEM0(CL_EXEC_NATIVE_KERNEL)
//NG_TABLE_END(TBL_EXEC_CAPABILITIES)
//
//NG_TABLE_START(TBL_QUEUE_PROPERTIES)
//    NG_TABLE_ITEM0(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
//    NG_TABLE_ITEM0(CL_QUEUE_PROFILING_ENABLE)
//NG_TABLE_END(TBL_QUEUE_PROPERTIES)
//
//NG_TABLE_START(TBL_SVM_CAPABILITES)
//    NG_TABLE_ITEM0(CL_DEVICE_SVM_COARSE_GRAIN_BUFFER)
//    NG_TABLE_ITEM0(CL_DEVICE_SVM_FINE_GRAIN_BUFFER)
//    NG_TABLE_ITEM0(CL_DEVICE_SVM_FINE_GRAIN_SYSTEM)
//    NG_TABLE_ITEM0(CL_DEVICE_SVM_ATOMICS)
//NG_TABLE_END(TBL_SVM_CAPABILITES)
//
//NG_TABLE_START(TBL_PARTITION_PROPERTY)
//    NG_TABLE_ITEM0(CL_DEVICE_PARTITION_EQUALLY)
//    NG_TABLE_ITEM0(CL_DEVICE_PARTITION_BY_COUNTS)
//    NG_TABLE_ITEM0(CL_DEVICE_PARTITION_BY_COUNTS_LIST_END)
//    NG_TABLE_ITEM0(CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN)
//NG_TABLE_END(TBL_PARTITION_PROPERTY)

void getDeviceInteger(
        cl_device_id device,
        cl_device_info info,
        const std::string& name,
        std::map<std::string, uint64_t>& attributes)
{
    uint64_t buffer = 0;
    size_t actualSize = 0;
    cl_int infoStatus = clGetDeviceInfo(device, info, sizeof(buffer), &buffer, &actualSize);
    if ((infoStatus == CL_SUCCESS) && (actualSize > 0)) {
        attributes[name] = buffer;
    }
}
#define ADD_DEVICE_INTEGER(X) getDeviceInteger(deviceId, X, #X, device.integerAttributes)

//void getDeviceBitfield(
//    cl_device_id device,
//    cl_device_info info,
//    const std::string& name,
//    ::ng::macro_utils::Table table,
//    std::map<std::string, std::vector<std::string> >& attributes)
//{
//    uint64_t buffer = 0;
//    size_t actualSize = 0;
//    cl_int infoStatus = clGetDeviceInfo(device, info, sizeof(buffer), &buffer, &actualSize);
//    if ((infoStatus == CL_SUCCESS) && (actualSize > 0)) {
//        attributes[name] = table.bitmaskArray(static_cast<long>(buffer));
//    }
//}
//#define ADD_DEVICE_BITFIELD(X, T) getDeviceBitfield(deviceId, X, #X, T, device.arrayAttributes)

}



void sysinf::collectClInfo(SystemInfo &systemInfo)
{
    systemInfo.hasCl = false;

    std::map<std::string, std::string>::iterator it;
    it = systemInfo.osInfo.attributes.find("build_details/GLBPD_OS_BUILD_BRAND");
    if(it != systemInfo.osInfo.attributes.end() && it->second.find("VEGA") != std::string::npos) {
        return;
    }
    
    cl_int infoStatus;
    int err = clewInit(0, &systemInfo.clInfo.driverPath);
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

		std::stringstream ssMajor;
		std::stringstream ssMinor;
		ssMajor << platform.version[7];
		ssMinor << platform.version[9];

		ssMajor >> platform.clMajor;
		ssMinor >> platform.clMinor;

        std::istringstream extensions(getPlatformInfoString(platformIDs[p], CL_PLATFORM_EXTENSIONS));
        std::copy(std::istream_iterator<std::string>(extensions),
                  std::istream_iterator<std::string>(),
                  std::back_inserter <std::vector<std::string> >(platform.extensions));

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
            cl_device_id deviceId = deviceIDs[d];
            size_t actualSize;
            CLDeviceInfo device;
            device.name = getDeviceInfoString(deviceId, CL_DEVICE_NAME);

            device.vendor = getDeviceInfoString(deviceId, CL_DEVICE_VENDOR);
            device.version = getDeviceInfoString(deviceId, CL_DEVICE_VERSION);
            device.openClCVersion = getDeviceInfoString(deviceId, CL_DEVICE_OPENCL_C_VERSION);
            device.driverVersion = getDeviceInfoString(deviceId, CL_DRIVER_VERSION);
            device.profile = getDeviceInfoString(deviceId, CL_DEVICE_PROFILE);

            size_t maxWorkItemSizes[3];
            infoStatus = clGetDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(maxWorkItemSizes), &maxWorkItemSizes, nullptr);
            assert(!infoStatus);
            for (int i = 0; i < 3; ++i) {
                device.maxWorkItemSizes[i] = maxWorkItemSizes[i];
            }




            //cl_device_partition_property partitionProperties[16];
            //infoStatus = clGetDeviceInfo(deviceId, CL_DEVICE_PARTITION_PROPERTIES, sizeof(partitionProperties), &partitionProperties, &actualSize);
            //if (infoStatus == CL_SUCCESS) {
            //    std::vector<std::string> properties;
            //    for (size_t i = 0; i < actualSize / sizeof(cl_device_partition_property); ++i) {
            //        properties.push_back(TBL_PARTITION_PROPERTY(static_cast<int>(partitionProperties[i])));
            //    }
            //    device.arrayAttributes["CL_DEVICE_PARTITION_PROPERTIES"] = properties;
            //}

            cl_device_type type;
            infoStatus = clGetDeviceInfo(deviceId, CL_DEVICE_TYPE, sizeof(type), &type, &actualSize);
            if ((infoStatus == CL_SUCCESS) && (actualSize > 0)) {
                device.type = TBL_DEVICE_TYPE(static_cast<int>(type));
            }

            cl_device_mem_cache_type mem_cache_type;
            infoStatus = clGetDeviceInfo(deviceId, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(mem_cache_type), &mem_cache_type, &actualSize);
            if ((infoStatus == CL_SUCCESS) && (actualSize > 0)) {
                device.stringAttributes["CL_DEVICE_GLOBAL_MEM_CACHE_TYPE"] = TBL_MEM_CACHE_TYPE(mem_cache_type);
            }

            cl_device_local_mem_type local_mem_type;
            infoStatus = clGetDeviceInfo(deviceId, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(local_mem_type), &local_mem_type, &actualSize);
            if ((infoStatus == CL_SUCCESS) && (actualSize > 0)) {
                device.stringAttributes["CL_DEVICE_LOCAL_MEM_TYPE"] = TBL_LOCAL_MEM_TYPE(local_mem_type);
            }

            std::istringstream deviceExtensions(getDeviceInfoString(deviceIDs[d], CL_DEVICE_EXTENSIONS));
            std::vector<std::string> extensions;
            std::copy(std::istream_iterator<std::string>(deviceExtensions),
                std::istream_iterator<std::string>(),
                std::back_inserter <std::vector<std::string> >(extensions));
            device.arrayAttributes["CL_DEVICE_EXTENSIONS"] = extensions;

            // std::istringstream deviceBuiltInKernels(getDeviceInfoString(deviceId, CL_DEVICE_BUILT_IN_KERNELS));
            // std::vector<std::string> builtInKernels;
            // std::copy(std::istream_iterator<std::string>(deviceBuiltInKernels),
            //     std::istream_iterator<std::string>(),
            //     std::back_inserter <std::vector<std::string> >(builtInKernels));
            // device.arrayAttributes["CL_DEVICE_BUILT_IN_KERNELS"] = builtInKernels;

            //ADD_DEVICE_BITFIELD(CL_DEVICE_HALF_FP_CONFIG, TBL_FP_CONFIG);
            //ADD_DEVICE_BITFIELD(CL_DEVICE_SINGLE_FP_CONFIG, TBL_FP_CONFIG);
            //ADD_DEVICE_BITFIELD(CL_DEVICE_DOUBLE_FP_CONFIG, TBL_FP_CONFIG);
            //ADD_DEVICE_BITFIELD(CL_DEVICE_EXECUTION_CAPABILITIES, TBL_EXEC_CAPABILITIES);
            //ADD_DEVICE_BITFIELD(CL_DEVICE_SVM_CAPABILITIES, TBL_EXEC_CAPABILITIES);
            //ADD_DEVICE_BITFIELD(CL_DEVICE_QUEUE_PROPERTIES, TBL_QUEUE_PROPERTIES);
            //ADD_DEVICE_BITFIELD(CL_DEVICE_QUEUE_ON_HOST_PROPERTIES, TBL_QUEUE_PROPERTIES);
            //ADD_DEVICE_BITFIELD(CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES, TBL_EXEC_CAPABILITIES);

            ADD_DEVICE_INTEGER(CL_DEVICE_VENDOR_ID);
            ADD_DEVICE_INTEGER(CL_DEVICE_MAX_COMPUTE_UNITS);
            ADD_DEVICE_INTEGER(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
            ADD_DEVICE_INTEGER(CL_DEVICE_MAX_WORK_GROUP_SIZE);
            ADD_DEVICE_INTEGER(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR);
            ADD_DEVICE_INTEGER(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT);
            ADD_DEVICE_INTEGER(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT);
            ADD_DEVICE_INTEGER(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG);
            ADD_DEVICE_INTEGER(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT);
            ADD_DEVICE_INTEGER(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE);
            ADD_DEVICE_INTEGER(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF);
            ADD_DEVICE_INTEGER(CL_DEVICE_MAX_CLOCK_FREQUENCY);
            ADD_DEVICE_INTEGER(CL_DEVICE_ADDRESS_BITS);
            ADD_DEVICE_INTEGER(CL_DEVICE_MAX_READ_IMAGE_ARGS);
            ADD_DEVICE_INTEGER(CL_DEVICE_MAX_WRITE_IMAGE_ARGS);
            ADD_DEVICE_INTEGER(CL_DEVICE_MAX_MEM_ALLOC_SIZE);
            ADD_DEVICE_INTEGER(CL_DEVICE_IMAGE2D_MAX_WIDTH);
            ADD_DEVICE_INTEGER(CL_DEVICE_IMAGE2D_MAX_HEIGHT);
            ADD_DEVICE_INTEGER(CL_DEVICE_IMAGE3D_MAX_WIDTH);
            ADD_DEVICE_INTEGER(CL_DEVICE_IMAGE3D_MAX_HEIGHT);
            ADD_DEVICE_INTEGER(CL_DEVICE_IMAGE3D_MAX_DEPTH);
            ADD_DEVICE_INTEGER(CL_DEVICE_IMAGE_SUPPORT);
            ADD_DEVICE_INTEGER(CL_DEVICE_MAX_PARAMETER_SIZE);
            ADD_DEVICE_INTEGER(CL_DEVICE_MAX_SAMPLERS);
            ADD_DEVICE_INTEGER(CL_DEVICE_MEM_BASE_ADDR_ALIGN);
            ADD_DEVICE_INTEGER(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE);
            ADD_DEVICE_INTEGER(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE);
            ADD_DEVICE_INTEGER(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE);
            ADD_DEVICE_INTEGER(CL_DEVICE_GLOBAL_MEM_SIZE);
            ADD_DEVICE_INTEGER(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE);
            ADD_DEVICE_INTEGER(CL_DEVICE_MAX_CONSTANT_ARGS);
            ADD_DEVICE_INTEGER(CL_DEVICE_LOCAL_MEM_SIZE);
            ADD_DEVICE_INTEGER(CL_DEVICE_ERROR_CORRECTION_SUPPORT);
            ADD_DEVICE_INTEGER(CL_DEVICE_PROFILING_TIMER_RESOLUTION);
            ADD_DEVICE_INTEGER(CL_DEVICE_ENDIAN_LITTLE);
            ADD_DEVICE_INTEGER(CL_DEVICE_AVAILABLE);
            ADD_DEVICE_INTEGER(CL_DEVICE_COMPILER_AVAILABLE);
            ADD_DEVICE_INTEGER(CL_DEVICE_HOST_UNIFIED_MEMORY);
            //ADD_DEVICE_INTEGER(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR);
            //ADD_DEVICE_INTEGER(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT);
            //ADD_DEVICE_INTEGER(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT);
            //ADD_DEVICE_INTEGER(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG);
            //ADD_DEVICE_INTEGER(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT);
            //ADD_DEVICE_INTEGER(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE);
            //ADD_DEVICE_INTEGER(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF);
            //ADD_DEVICE_INTEGER(CL_DEVICE_LINKER_AVAILABLE);
            //ADD_DEVICE_INTEGER(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE);
            //ADD_DEVICE_INTEGER(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE);
            //ADD_DEVICE_INTEGER(CL_DEVICE_PARTITION_MAX_SUB_DEVICES);
            //ADD_DEVICE_INTEGER(CL_DEVICE_PARTITION_AFFINITY_DOMAIN);
            //ADD_DEVICE_INTEGER(CL_DEVICE_PARTITION_TYPE);
            //ADD_DEVICE_INTEGER(CL_DEVICE_PREFERRED_INTEROP_USER_SYNC);
            //ADD_DEVICE_INTEGER(CL_DEVICE_PRINTF_BUFFER_SIZE);
            //ADD_DEVICE_INTEGER(CL_DEVICE_IMAGE_PITCH_ALIGNMENT);
            //ADD_DEVICE_INTEGER(CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT);
            //ADD_DEVICE_INTEGER(CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS);
            //ADD_DEVICE_INTEGER(CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE);
            //ADD_DEVICE_INTEGER(CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE);
            //ADD_DEVICE_INTEGER(CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE);
            //ADD_DEVICE_INTEGER(CL_DEVICE_MAX_ON_DEVICE_QUEUES);
            //ADD_DEVICE_INTEGER(CL_DEVICE_MAX_ON_DEVICE_EVENTS);
            //ADD_DEVICE_INTEGER(CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE);
            //ADD_DEVICE_INTEGER(CL_DEVICE_MAX_PIPE_ARGS);
            //ADD_DEVICE_INTEGER(CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS);
            //ADD_DEVICE_INTEGER(CL_DEVICE_PIPE_MAX_PACKET_SIZE);
            //ADD_DEVICE_INTEGER(CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT);
            //ADD_DEVICE_INTEGER(CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT);
            //ADD_DEVICE_INTEGER(CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT);

            int level = 0;
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

            platform.devices.push_back(device);
        }
        systemInfo.clInfo.platforms.push_back(platform);
    }
    
    systemInfo.hasCl = true;
}
