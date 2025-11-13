/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "clinfo.h"

using namespace tfw;


CLDeviceInfo::CLDeviceInfo()
: compilerAvailable(false)
, deviceAddressBits(0)
, maxComputeUnits(0)
, maxClockFrequency(0)
, deviceGlobalMemSize(0)
, maxMemAllocSize(0)
, globalMemCacheSize(0)
, globalMemCachelineSize(0)
, localMemSize(0)
, maxConstantBufferSize(0)
, maxWorkItemDimensions(0)
, maxWorkGorupSize(0)
, hasImageSupport(false)
, maxReadImageArgs(0)
, maxWriteImageArgs(0)
, maxSamplers(0)
{
    for(int i = 0; i < 3; ++i) {
        maxWorkItemSizes[i] = 0;
    }
    for(int i = 0; i < 2; ++i) {
        image2DMaxSize[i] = 0;
    }
    for(int i = 0; i < 3; ++i) {
        image3DMaxSize[i] = 0;
    }
}

CLPlatformInfo::CLPlatformInfo()
{
}
