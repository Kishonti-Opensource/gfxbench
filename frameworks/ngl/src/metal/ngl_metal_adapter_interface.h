/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __NGL_METAL_ADAPTER_H__
#define __NGL_METAL_ADAPTER_H__

#include <Metal/Metal.h>
#include <stdint.h>

class NGL_metal_adapter_interface
{
public:
    NGL_metal_adapter_interface()
    : device(nil)
    , commandQueue(nil)
    , system_depth(0)
    , macos_use_subpass(false)
    {
    }
    virtual ~NGL_metal_adapter_interface()
    {
    }
    
    virtual id <MTLTexture> GetBackBuffer() = 0;
    virtual MTLPixelFormat GetBackBufferPixelFormat() = 0;
    
    id <MTLDevice> device;
    id <MTLCommandQueue> commandQueue;
    uint32_t system_depth;
    bool macos_use_subpass;
};

#endif  // __NGL_METAL_ADAPTER_H__

