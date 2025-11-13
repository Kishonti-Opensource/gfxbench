/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef METALINFO_H
#define METALINFO_H

#include <string>
#include <vector>

namespace sysinf
{



class MetalDeviceInfo
{
public:
    MetalDeviceInfo(): 
        programmableBlendingSupport(false),
        pvrtcSupport(false),
        etcSupport(false),
        bcSupport(false),
        cubemapTextureArraySupport(false),
        textureBarrierSupport(false),
        layeredRenderingSupport(false),
        samplerComparisonFunctionSupport(false),
        countingOcclusionQuerySupport(false),
        baseVertexSupport(false),
        indirectDrawingSupport(false),
        indirectProcessingSupport(false),
        msaaDepthResolveSupport(false),
        astcSupport(false),
	    tessellationSupport(false),
        metalPerformanceShaderSupport(false) {}
    std::string featureSet;
    std::string deviceName;
    bool programmableBlendingSupport;
    bool pvrtcSupport;
    bool etcSupport;
    bool bcSupport;
    bool cubemapTextureArraySupport;
    bool textureBarrierSupport;
    bool layeredRenderingSupport;
    bool samplerComparisonFunctionSupport;
    bool countingOcclusionQuerySupport;
    bool baseVertexSupport;
    bool indirectDrawingSupport;
    bool indirectProcessingSupport;
    bool msaaDepthResolveSupport;
    bool astcSupport;
	bool tessellationSupport;
    bool metalPerformanceShaderSupport;
    
    bool isLowPower;
    bool isHeadless;
    bool isMobile;
    
    std::vector<std::pair<std::string, int> > attributes;
    
    template<class F> void applyVisitor(F visitor) const {
        for (auto i = attributes.begin(); i != attributes.end(); ++i) {
            visitor(i->first, i->second);
        }
        visitor("major", featureSet);
        visitor("minor", deviceName);
        visitor("FEATURE_SET", featureSet);
        visitor("DEVICE_NAME", deviceName);
        visitor("PROGRAMMABLE_BLENDING_SUPPORT", programmableBlendingSupport);
        visitor("PVRTC_SUPPORT", pvrtcSupport);
        visitor("ETC_SUPPORT", etcSupport);
        visitor("BC_SUPPORT", bcSupport);
        visitor("CUBEMAP_TEXTURE_ARRAY_SUPPORT", cubemapTextureArraySupport);
        visitor("TEXTURE_BARRIER_SUPPORT", textureBarrierSupport);
        visitor("LAYERED_RENDERING_SUPPORT", layeredRenderingSupport);
        visitor("SAMPLER_COMPARISON_FUNCTION_SUPPORT", samplerComparisonFunctionSupport);
        visitor("COUNTING_OCCLUSION_QUERY_SUPPORT", countingOcclusionQuerySupport);
        visitor("BASE_VERTEX_SUPPORT", baseVertexSupport);
        visitor("INDIRECT_DRAWING_SUPPORT", indirectDrawingSupport);
        visitor("INDIRECT_PROCESSING_SUPPORT", indirectProcessingSupport);
        visitor("MSAA_DEPTH_RESOLVE_SUPPORT", msaaDepthResolveSupport);
        visitor("ASTC_SUPPORT", astcSupport);
        visitor("METAL_PERFORMANCE_SHADER_SUPPORT", metalPerformanceShaderSupport);
        visitor("isLowPower", isLowPower);
        visitor("isHeadless", isHeadless);
        visitor("isMobile", isMobile);
    }
};

class MetalInfo
{
public:
    std::vector<MetalDeviceInfo> devices;
    
    template<class F> void applyVisitor(F visitor) const {
        visitor("devices", devices);
    }
};
    
    
    
}

#endif  // METALINFO_H
