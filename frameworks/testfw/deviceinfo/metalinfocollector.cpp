/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "metalinfo.h"

#include "jsonvisitor.h"

using namespace tfw;



MetalInfoCollector::MetalInfoCollector():
    hasMetal_(false)
{}



bool MetalInfoCollector::isMetalAvailable() const
{
    return hasMetal_;
}



const MetalInfo& MetalInfoCollector::metal() const
{
    return metal_;
}



std::string MetalInfoCollector::serialize() const
{
    if (!hasMetal_) {
        return "{}";
    }
    ng::JsonValue root;
    applyVisitor(metal_, JsonVisitor(root));
    return root.toString();
}



#if __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
        #define HAS_METAL
    #endif
#endif



#ifndef HAS_METAL

void MetalInfoCollector::collect()
{}

#else

#include <Metal/Metal.h>
extern id <MTLDevice> MTLCreateSystemDefaultDevice(void) WEAK_IMPORT_ATTRIBUTE;



#define ADD_ATTRIBUTE(name, value) metal_.attributes.push_back(std::pair<std::string, int>(name, value))

void MetalInfoCollector::collect()
{
    id <MTLDevice> device = MTLCreateSystemDefaultDevice();
    if([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1]) {
        hasMetal_ = true;
        metal_.name = "MTLFeatureSet_iOS_GPUFamily2_v1";
        metal_.astcSupport = true;
        ADD_ATTRIBUTE("MAX_COLOR_ATTACHMENT_PER_PASSDESC", 8);
        ADD_ATTRIBUTE("MAX_COLOR_OUT_PER_SAMPLE_PER_PASS", 32);
        ADD_ATTRIBUTE("MIN_ATTACHMENT_SIZE", 32);
        ADD_ATTRIBUTE("THREADGROUP_MEMORY_ALLOC_SIZE_INC", 16);
        ADD_ATTRIBUTE("MAX_THREADGROUP_MEM_ALLOC", 16384);
        ADD_ATTRIBUTE("MAX_TEXTURE_DESC_HEIGHT_WIDTH", 4096);
        ADD_ATTRIBUTE("MAX_TEXTURE_DESC_DEPTH", 2048);
        ADD_ATTRIBUTE("MAX_ENTRIES_BUFFER", 31);
        ADD_ATTRIBUTE("MAX_ENTRIES_TEXTURE", 31);
        ADD_ATTRIBUTE("MAX_ENTRIES_SAMPLERSTATE", 16);
    } else if([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v1]) {
        hasMetal_ = true;
        metal_.name = "MTLFeatureSet_iOS_GPUFamily1_v1";
        metal_.astcSupport = false;
        ADD_ATTRIBUTE("MAX_COLOR_ATTACHMENT_PER_PASSDESC", 4);
        ADD_ATTRIBUTE("MAX_COLOR_OUT_PER_SAMPLE_PER_PASS", 16);
        ADD_ATTRIBUTE("MIN_ATTACHMENT_SIZE", 32);
        ADD_ATTRIBUTE("THREADGROUP_MEMORY_ALLOC_SIZE_INC", 16);
        ADD_ATTRIBUTE("MAX_THREADGROUP_MEM_ALLOC", 16384);
        ADD_ATTRIBUTE("MAX_TEXTURE_DESC_HEIGHT_WIDTH", 4096);
        ADD_ATTRIBUTE("MAX_TEXTURE_DESC_DEPTH", 2048);
        ADD_ATTRIBUTE("MAX_ENTRIES_BUFFER", 31);
        ADD_ATTRIBUTE("MAX_ENTRIES_TEXTURE", 31);
        ADD_ATTRIBUTE("MAX_ENTRIES_SAMPLERSTATE", 16);
    }
}

#endif
