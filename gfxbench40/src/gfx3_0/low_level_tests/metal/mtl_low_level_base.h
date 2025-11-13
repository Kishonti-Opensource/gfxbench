/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __METAL_LOW_LEVEL_TEST_BASE__
#define __METAL_LOW_LEVEL_TEST_BASE__

#include "mtl_globals.h"
#import <Metal/Metal.h>

#import "graphics/metalgraphicscontext.h"
#include "test_descriptor.h"
#include "kcl_base.h"

#import "mtl_shader_helper.h"


class MetalLowLevelTestBase
{
protected:
    MetalGraphicsContext      *m_Context;
    
    id <MTLDevice>             m_Device;
    MTLViewport                m_Viewport;
    
    id <MTLCommandQueue>       m_CommandQueue;
    
    id <MTLLibrary> m_VertexShaderLibrary;
    id <MTLLibrary> m_FragmentShaderLibrary;
    
    // Clear values
    MTLClearColor m_ClearColor;
    double m_ClearDepth;
    
    bool InitMetalContext(GraphicsContext* ctx, const TestDescriptor *settings) ;
    
    KCL::KCL_Status LoadLibrariesFromFile(const char* vs_filename, const char* fs_filename, const char* vs_header, const char* fs_header) ;
    
    void FreeShaderLibraries() ;
};






#endif //__METAL_LOW_LEVEL_TEST_BASE__


