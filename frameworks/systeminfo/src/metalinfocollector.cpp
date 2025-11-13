/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "metalinfocollector.h"

#if __APPLE__
#   include <Metal/Metal.h>
#   include <TargetConditionals.h>
#endif

using namespace sysinf;

#if __APPLE__

extern id <MTLDevice> MTLCreateSystemDefaultDevice(void) WEAK_IMPORT_ATTRIBUTE;
#if TARGET_OS_OSX
extern NSArray <id<MTLDevice>> *MTLCopyAllDevices(void) WEAK_IMPORT_ATTRIBUTE;
#endif

#define ADD_ATTRIBUTE(metalInfo, name, value) metalInfo.attributes.push_back(std::pair<std::string, int>(name, value))

#endif

void sysinf::collectMetalInfo(SystemInfo& systemInfo)
{    
#if __APPLE__
    systemInfo.hasMetal = false;

    if(MTLCreateSystemDefaultDevice() == nullptr)
    {
        return;
    }
    
    NSArray <id<MTLDevice>>* allDevices = nil;
#if TARGET_OS_IPHONE
    allDevices = [NSArray arrayWithObject:MTLCreateSystemDefaultDevice()];
#elif TARGET_OS_MAC
    allDevices = MTLCopyAllDevices();
#endif
    uint32_t deviceCount = (uint32_t)allDevices.count;
    
    systemInfo.metalInfo.devices.resize(deviceCount);
    for(int i = 0; i < deviceCount; i++)
    {
        const id<MTLDevice> device = allDevices[i];
        MetalDeviceInfo& deviceInfo = systemInfo.metalInfo.devices[i];
        
        deviceInfo.featureSet.clear();
        deviceInfo.deviceName.clear();
        deviceInfo.astcSupport = false;
        deviceInfo.bcSupport = false;
        deviceInfo.attributes.clear();
        
        systemInfo.hasMetal = true;
        deviceInfo.deviceName = [device.name UTF8String];
		
        ADD_ATTRIBUTE(deviceInfo, "MAX_VERTEX_ATTRIBUTES", 31);
        ADD_ATTRIBUTE(deviceInfo, "MAX_ENTRIES_BUFFER", 31);
        ADD_ATTRIBUTE(deviceInfo, "MAX_ENTRIES_SAMPLERSTATE", 16);
        ADD_ATTRIBUTE(deviceInfo, "MAX_DATA_BLOCK_LENGTH_BYTES", 4096);
        ADD_ATTRIBUTE(deviceInfo, "MAX_COMPONENTS_FROM_VERTEX_TO_FRAGMENT", 60);
        ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_1D_2D_LAYERS", 2048);
        ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_3D_LAYERS", 2048);
        ADD_ATTRIBUTE(deviceInfo, "MAX_POINT_SIZE", 511);
        ADD_ATTRIBUTE(deviceInfo, "MAX_VISIBILITY_QUERY_OFFSET_BYTES", 65528);
        ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_3D_DIMENSION", 2048);
        ADD_ATTRIBUTE(deviceInfo, "MAX_BUFFER_LENGTH_MEGABYTES", 256);
        
#if TARGET_OS_OSX

		deviceInfo.isMobile = false;
		deviceInfo.isLowPower = [device isLowPower];
		deviceInfo.isHeadless = [device isHeadless];

        if (@available(macOS 10.15, *))
        {
            if ( [device supportsFamily:MTLGPUFamilyApple5 ]) // A12
            {
                deviceInfo.featureSet = "MTLGPUFamilyApple5";
                deviceInfo.programmableBlendingSupport = true;
                deviceInfo.pvrtcSupport = true;
                deviceInfo.etcSupport = true;
                deviceInfo.bcSupport = [ device supportsBCTextureCompression ];
                deviceInfo.cubemapTextureArraySupport = true;
                deviceInfo.textureBarrierSupport = false; // texture barriers ?
                deviceInfo.layeredRenderingSupport = true; // layered rendering
                deviceInfo.samplerComparisonFunctionSupport = true; //
                deviceInfo.countingOcclusionQuerySupport = true; // Counting occlusion query
                deviceInfo.baseVertexSupport = true; // Base vertex drawing
                deviceInfo.indirectDrawingSupport = true; // indirect draw & dispatch argument
                deviceInfo.indirectProcessingSupport = true; // ?
                deviceInfo.msaaDepthResolveSupport = false; // MSAA depth resolve
                deviceInfo.astcSupport = true;
                deviceInfo.tessellationSupport = true;
                deviceInfo.metalPerformanceShaderSupport = false;
                // Function arguments:
                ADD_ATTRIBUTE(deviceInfo, "TEXTURE_TO_BUFFER_COPY_ALIGMENT_BYTES", 256);
                
                ADD_ATTRIBUTE(deviceInfo, "MAX_ENTRIES_TEXTURE", 96);
                ADD_ATTRIBUTE(deviceInfo, "MAX_THREADS_PER_THREADGROUP", 1024);
                ADD_ATTRIBUTE(deviceInfo, "MAX_THREADGROUP_MEM_ALLOC_BYTES", 32768);
                
                ADD_ATTRIBUTE(deviceInfo, "MIN_PARAMETER_BUFFER_OFFSET_BYTES", 256);
                
                ADD_ATTRIBUTE(deviceInfo, "MAX_CUBEMAPS_IN_ARRAY", 341);
                ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_ATTACHMENT_PER_PASSDESC", 8);
                // Resources:
                ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_1D_WIDTH", 16384);
                ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_2D_WIDTH_HEIGHT", 16384);
                ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_CUBEMAP_WIDTH_HEIGHT", 16384);
            }
        }
        else if([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v2])
		{
			deviceInfo.featureSet = "MTLFeatureSet_macOS_GPUFamily1_v2";
			deviceInfo.programmableBlendingSupport = false;
			deviceInfo.pvrtcSupport = false;
			deviceInfo.etcSupport = false;
			deviceInfo.bcSupport = true;
			deviceInfo.cubemapTextureArraySupport = true;
			deviceInfo.textureBarrierSupport = true;
			deviceInfo.layeredRenderingSupport = true;
			deviceInfo.samplerComparisonFunctionSupport = true;
			deviceInfo.countingOcclusionQuerySupport = true;
			deviceInfo.baseVertexSupport = true;
			deviceInfo.indirectDrawingSupport = true;
			deviceInfo.indirectProcessingSupport = true;
			deviceInfo.msaaDepthResolveSupport = false;
			deviceInfo.astcSupport = false;
			deviceInfo.tessellationSupport = true;
			deviceInfo.metalPerformanceShaderSupport = false;
			ADD_ATTRIBUTE(deviceInfo, "TEXTURE_TO_BUFFER_COPY_ALIGMENT_BYTES", 256);
			ADD_ATTRIBUTE(deviceInfo, "MAX_ENTRIES_TEXTURE", 128);
			ADD_ATTRIBUTE(deviceInfo, "MAX_THREADS_PER_THREADGROUP", 1024);
			ADD_ATTRIBUTE(deviceInfo, "MAX_THREADGROUP_MEM_ALLOC_BYTES", 32768);
			ADD_ATTRIBUTE(deviceInfo, "MIN_PARAMETER_BUFFER_OFFSET_BYTES", 256);
			ADD_ATTRIBUTE(deviceInfo, "MAX_CUBEMAPS_IN_ARRAY", 341);
			ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_ATTACHMENT_PER_PASSDESC", 8);
			ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_1D_WIDTH", 16384);
			ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_2D_WIDTH_HEIGHT", 16384);
			ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_CUBEMAP_WIDTH_HEIGHT", 16384);
		}
		else if([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v1])
		{
			deviceInfo.featureSet = "MTLFeatureSet_macOS_GPUFamily1_v1";
            deviceInfo.programmableBlendingSupport = false;
            deviceInfo.pvrtcSupport = false;
            deviceInfo.etcSupport = false;
            deviceInfo.bcSupport = true;
            deviceInfo.cubemapTextureArraySupport = true;
            deviceInfo.textureBarrierSupport = true;
            deviceInfo.layeredRenderingSupport = true;
            deviceInfo.samplerComparisonFunctionSupport = true;
            deviceInfo.countingOcclusionQuerySupport = true;
            deviceInfo.baseVertexSupport = true;
            deviceInfo.indirectDrawingSupport = true;
            deviceInfo.indirectProcessingSupport = true;
            deviceInfo.msaaDepthResolveSupport = false;
            deviceInfo.astcSupport = false;
            deviceInfo.metalPerformanceShaderSupport = false;
            ADD_ATTRIBUTE(deviceInfo, "TEXTURE_TO_BUFFER_COPY_ALIGMENT_BYTES", 256);
            ADD_ATTRIBUTE(deviceInfo, "MAX_ENTRIES_TEXTURE", 128);
            ADD_ATTRIBUTE(deviceInfo, "MAX_THREADS_PER_THREADGROUP", 1024);
            ADD_ATTRIBUTE(deviceInfo, "MAX_THREADGROUP_MEM_ALLOC_BYTES", 32768);
            ADD_ATTRIBUTE(deviceInfo, "MIN_PARAMETER_BUFFER_OFFSET_BYTES", 256);
            ADD_ATTRIBUTE(deviceInfo, "MAX_CUBEMAPS_IN_ARRAY", 341);
            ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_ATTACHMENT_PER_PASSDESC", 8);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_1D_WIDTH", 16384);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_2D_WIDTH_HEIGHT", 16384);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_CUBEMAP_WIDTH_HEIGHT", 16384);
        }
        
#elif TARGET_OS_IPHONE

		deviceInfo.isMobile = true;
		deviceInfo.isLowPower = false;
		deviceInfo.isHeadless = false;

		if([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v2])
		{
			deviceInfo.featureSet = "MTLFeatureSet_iOS_GPUFamily3_v2";
			deviceInfo.programmableBlendingSupport = true;
			deviceInfo.pvrtcSupport = true;
			deviceInfo.etcSupport = true;
			deviceInfo.bcSupport = false;
			deviceInfo.cubemapTextureArraySupport = false;
			deviceInfo.textureBarrierSupport = false;
			deviceInfo.layeredRenderingSupport = false;
			deviceInfo.samplerComparisonFunctionSupport = true;
			deviceInfo.countingOcclusionQuerySupport = true;
			deviceInfo.baseVertexSupport = true;
			deviceInfo.indirectDrawingSupport = true;
			deviceInfo.indirectProcessingSupport = true;
			deviceInfo.msaaDepthResolveSupport = true;
			deviceInfo.astcSupport = true;
			deviceInfo.tessellationSupport = true;
			deviceInfo.metalPerformanceShaderSupport = true;
			ADD_ATTRIBUTE(deviceInfo, "NEW_TEXTURE_FROM_BUFFER_ALIGNMENT_BYTES", 16);
			ADD_ATTRIBUTE(deviceInfo, "TEXTURE_TO_BUFFER_COPY_ALIGMENT_BYTES", 16);
			ADD_ATTRIBUTE(deviceInfo, "MAX_ENTRIES_TEXTURE", 31);
			ADD_ATTRIBUTE(deviceInfo, "MAX_THREADS_PER_THREADGROUP", 512);
			ADD_ATTRIBUTE(deviceInfo, "MAX_THREADGROUP_MEM_ALLOC_BYTES", 16384);
			ADD_ATTRIBUTE(deviceInfo, "MIN_PARAMETER_BUFFER_OFFSET_BYTES", 4);
			ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_ATTACHMENT_PER_PASSDESC", 8);
			ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_OUT_PER_SAMPLE_PER_PASS", 32);
			ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_1D_WIDTH", 16384);
			ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_2D_WIDTH_HEIGHT", 16384);
			ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_CUBEMAP_WIDTH_HEIGHT", 16384);
		}
		else if([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1])
		{
			deviceInfo.featureSet = "MTLFeatureSet_iOS_GPUFamily3_v1";
            deviceInfo.programmableBlendingSupport = true;
            deviceInfo.pvrtcSupport = true;
            deviceInfo.etcSupport = true;
            deviceInfo.bcSupport = false;
            deviceInfo.cubemapTextureArraySupport = false;
            deviceInfo.textureBarrierSupport = false;
            deviceInfo.layeredRenderingSupport = false;
            deviceInfo.samplerComparisonFunctionSupport = true;
            deviceInfo.countingOcclusionQuerySupport = true;
            deviceInfo.baseVertexSupport = true;
            deviceInfo.indirectDrawingSupport = true;
            deviceInfo.indirectProcessingSupport = true;
            deviceInfo.msaaDepthResolveSupport = true;
            deviceInfo.astcSupport = true;
            deviceInfo.metalPerformanceShaderSupport = true;
            ADD_ATTRIBUTE(deviceInfo, "NEW_TEXTURE_FROM_BUFFER_ALIGNMENT_BYTES", 16);
            ADD_ATTRIBUTE(deviceInfo, "TEXTURE_TO_BUFFER_COPY_ALIGMENT_BYTES", 16);
            ADD_ATTRIBUTE(deviceInfo, "MAX_ENTRIES_TEXTURE", 31);
            ADD_ATTRIBUTE(deviceInfo, "MAX_THREADS_PER_THREADGROUP", 512);
            ADD_ATTRIBUTE(deviceInfo, "MAX_THREADGROUP_MEM_ALLOC_BYTES", 16384);
            ADD_ATTRIBUTE(deviceInfo, "MIN_PARAMETER_BUFFER_OFFSET_BYTES", 4);
            ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_ATTACHMENT_PER_PASSDESC", 8);
            ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_OUT_PER_SAMPLE_PER_PASS", 32);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_1D_WIDTH", 16384);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_2D_WIDTH_HEIGHT", 16384);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_CUBEMAP_WIDTH_HEIGHT", 16384);
        }
		else if([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v2])
		{
			deviceInfo.featureSet = "MTLFeatureSet_iOS_GPUFamily2_v2";
            deviceInfo.programmableBlendingSupport = true;
            deviceInfo.pvrtcSupport = true;
            deviceInfo.etcSupport = true;
            deviceInfo.bcSupport = false;
            deviceInfo.cubemapTextureArraySupport = false;
            deviceInfo.textureBarrierSupport = false;
            deviceInfo.layeredRenderingSupport = false;
            deviceInfo.samplerComparisonFunctionSupport = false;
            deviceInfo.countingOcclusionQuerySupport = false;
            deviceInfo.baseVertexSupport = false;
            deviceInfo.indirectDrawingSupport = false;
            deviceInfo.indirectProcessingSupport = false;
            deviceInfo.msaaDepthResolveSupport = false;
            deviceInfo.astcSupport = true;
            deviceInfo.metalPerformanceShaderSupport = true;
            ADD_ATTRIBUTE(deviceInfo, "NEW_TEXTURE_FROM_BUFFER_ALIGNMENT_BYTES", 64);
            ADD_ATTRIBUTE(deviceInfo, "TEXTURE_TO_BUFFER_COPY_ALIGMENT_BYTES", 64);
            ADD_ATTRIBUTE(deviceInfo, "MAX_ENTRIES_TEXTURE", 31);
            ADD_ATTRIBUTE(deviceInfo, "MAX_THREADS_PER_THREADGROUP", 512);
            ADD_ATTRIBUTE(deviceInfo, "MAX_THREADGROUP_MEM_ALLOC_BYTES", 16384);
            ADD_ATTRIBUTE(deviceInfo, "MIN_PARAMETER_BUFFER_OFFSET_BYTES", 4);
            ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_ATTACHMENT_PER_PASSDESC", 8);
            ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_OUT_PER_SAMPLE_PER_PASS", 32);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_1D_WIDTH", 8192);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_2D_WIDTH_HEIGHT", 8192);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_CUBEMAP_WIDTH_HEIGHT", 8192);
        }
		else if([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1])
		{
			deviceInfo.featureSet = "MTLFeatureSet_iOS_GPUFamily2_v1";
            deviceInfo.programmableBlendingSupport = true;
            deviceInfo.pvrtcSupport = true;
            deviceInfo.etcSupport = true;
            deviceInfo.bcSupport = false;
            deviceInfo.cubemapTextureArraySupport = false;
            deviceInfo.textureBarrierSupport = false;
            deviceInfo.layeredRenderingSupport = false;
            deviceInfo.samplerComparisonFunctionSupport = false;
            deviceInfo.countingOcclusionQuerySupport = false;
            deviceInfo.baseVertexSupport = false;
            deviceInfo.indirectDrawingSupport = false;
            deviceInfo.indirectProcessingSupport = false;
            deviceInfo.msaaDepthResolveSupport = false;
            deviceInfo.astcSupport = true;
            deviceInfo.metalPerformanceShaderSupport = true;
            ADD_ATTRIBUTE(deviceInfo, "NEW_TEXTURE_FROM_BUFFER_ALIGNMENT_BYTES", 64);
            ADD_ATTRIBUTE(deviceInfo, "TEXTURE_TO_BUFFER_COPY_ALIGMENT_BYTES", 64);
            ADD_ATTRIBUTE(deviceInfo, "MAX_ENTRIES_TEXTURE", 31);
            ADD_ATTRIBUTE(deviceInfo, "MAX_THREADS_PER_THREADGROUP", 512);
            ADD_ATTRIBUTE(deviceInfo, "MAX_THREADGROUP_MEM_ALLOC_BYTES", 16384);
            ADD_ATTRIBUTE(deviceInfo, "MIN_PARAMETER_BUFFER_OFFSET_BYTES", 4);
            ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_ATTACHMENT_PER_PASSDESC", 8);
            ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_OUT_PER_SAMPLE_PER_PASS", 32);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_1D_WIDTH", 4096);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_2D_WIDTH_HEIGHT", 4096);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_CUBEMAP_WIDTH_HEIGHT", 4096);
        }
		else if([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v2])
		{
			deviceInfo.featureSet = "MTLFeatureSet_iOS_GPUFamily1_v2";
            deviceInfo.programmableBlendingSupport = true;
            deviceInfo.pvrtcSupport = true;
            deviceInfo.etcSupport = true;
            deviceInfo.bcSupport = false;
            deviceInfo.cubemapTextureArraySupport = false;
            deviceInfo.textureBarrierSupport = false;
            deviceInfo.layeredRenderingSupport = false;
            deviceInfo.samplerComparisonFunctionSupport = false;
            deviceInfo.countingOcclusionQuerySupport = false;
            deviceInfo.baseVertexSupport = false;
            deviceInfo.indirectDrawingSupport = false;
            deviceInfo.indirectProcessingSupport = false;
            deviceInfo.msaaDepthResolveSupport = false;
            deviceInfo.astcSupport = false;
            deviceInfo.metalPerformanceShaderSupport = false;
            ADD_ATTRIBUTE(deviceInfo, "NEW_TEXTURE_FROM_BUFFER_ALIGNMENT_BYTES", 64);
            ADD_ATTRIBUTE(deviceInfo, "TEXTURE_TO_BUFFER_COPY_ALIGMENT_BYTES", 64);
            ADD_ATTRIBUTE(deviceInfo, "MAX_ENTRIES_TEXTURE", 31);
            ADD_ATTRIBUTE(deviceInfo, "MAX_THREADS_PER_THREADGROUP", 512);
            ADD_ATTRIBUTE(deviceInfo, "MAX_THREADGROUP_MEM_ALLOC_BYTES", 16384);
            ADD_ATTRIBUTE(deviceInfo, "MIN_PARAMETER_BUFFER_OFFSET_BYTES", 4);
            ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_ATTACHMENT_PER_PASSDESC", 4);
            ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_OUT_PER_SAMPLE_PER_PASS", 16);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_1D_WIDTH", 8192);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_2D_WIDTH_HEIGHT", 8192);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_CUBEMAP_WIDTH_HEIGHT", 8192);
        }
		else if([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v1])
		{
			deviceInfo.featureSet = "MTLFeatureSet_iOS_GPUFamily1_v1";
            deviceInfo.programmableBlendingSupport = true;
            deviceInfo.pvrtcSupport = true;
            deviceInfo.etcSupport = true;
            deviceInfo.bcSupport = false;
            deviceInfo.cubemapTextureArraySupport = false;
            deviceInfo.textureBarrierSupport = false;
            deviceInfo.layeredRenderingSupport = false;
            deviceInfo.samplerComparisonFunctionSupport = false;
            deviceInfo.countingOcclusionQuerySupport = false;
            deviceInfo.baseVertexSupport = false;
            deviceInfo.indirectDrawingSupport = false;
            deviceInfo.indirectProcessingSupport = false;
            deviceInfo.msaaDepthResolveSupport = false;
            deviceInfo.astcSupport = false;
            deviceInfo.metalPerformanceShaderSupport = false;
            ADD_ATTRIBUTE(deviceInfo, "NEW_TEXTURE_FROM_BUFFER_ALIGNMENT_BYTES", 64);
            ADD_ATTRIBUTE(deviceInfo, "TEXTURE_TO_BUFFER_COPY_ALIGMENT_BYTES", 64);
            ADD_ATTRIBUTE(deviceInfo, "MAX_ENTRIES_TEXTURE", 31);
            ADD_ATTRIBUTE(deviceInfo, "MAX_THREADS_PER_THREADGROUP", 512);
            ADD_ATTRIBUTE(deviceInfo, "MAX_THREADGROUP_MEM_ALLOC_BYTES", 16384);
            ADD_ATTRIBUTE(deviceInfo, "MIN_PARAMETER_BUFFER_OFFSET_BYTES", 4);
            ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_ATTACHMENT_PER_PASSDESC", 4);
            ADD_ATTRIBUTE(deviceInfo, "MAX_COLOR_OUT_PER_SAMPLE_PER_PASS", 16);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_1D_WIDTH", 4096);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_2D_WIDTH_HEIGHT", 4096);
            ADD_ATTRIBUTE(deviceInfo, "MAX_TEXTURE_CUBEMAP_WIDTH_HEIGHT", 4096);
        }
#endif

    }
#endif
    
}
