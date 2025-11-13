/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#import <Foundation/Foundation.h>
#import "mtl_low_level_base.h"

#include "kcl_io.h"

 
bool MetalLowLevelTestBase::InitMetalContext(GraphicsContext* ctx, const TestDescriptor *settings)
{
    m_Context = (MetalGraphicsContext*)ctx;
    
    {
        // find a usable Device
        m_Device = m_Context->getDevice();
        
        MTLViewport viewport = {0.0, 0.0, static_cast<double>(settings->m_viewport_width), static_cast<double>(settings->m_viewport_height), 0.0, 1.0};
        m_Viewport = viewport ;
    }
        
    return true ;
}


KCL::KCL_Status MetalLowLevelTestBase::LoadLibrariesFromFile(const char* vs_filename, const char* fs_filename, const char* vs_header, const char* fs_header)
{
    std::string vs_shader_source = "#define TYPE_VERTEX 1\n" ;
    std::string fs_shader_source = "#define TYPE_FRAGMENT 1\n" ;
    
    vs_shader_source += vs_header ;
    fs_shader_source += fs_header ;
    
    KCL::AssetFile float_header_file("shaders_mtl/common/float_header.h") ;
    KCL::AssetFile vs_shader_source_file(vs_filename) ;
    KCL::AssetFile fs_shader_source_file(fs_filename) ;
    
    vs_shader_source += float_header_file.GetBuffer() ;
    vs_shader_source += vs_shader_source_file.GetBuffer() ;
    m_VertexShaderLibrary = MetalRender::LoadShaderLibraryFromString(m_Device,vs_shader_source) ;
    
    if(!m_VertexShaderLibrary) { NSLog(@">> ERROR: Failed creating a shared library!"); return KCL::KCL_TESTERROR_SHADER_ERROR; }
    
    fs_shader_source += float_header_file.GetBuffer() ;
    fs_shader_source += fs_shader_source_file.GetBuffer() ;
    m_FragmentShaderLibrary = MetalRender::LoadShaderLibraryFromString(m_Device,fs_shader_source) ;
    
    if(!m_FragmentShaderLibrary) { NSLog(@">> ERROR: Failed creating a shared library!"); return KCL::KCL_TESTERROR_SHADER_ERROR; }


    return KCL::KCL_TESTERROR_NOERROR;
}


void MetalLowLevelTestBase::FreeShaderLibraries()
{
    releaseObj(m_VertexShaderLibrary) ;
    releaseObj(m_FragmentShaderLibrary) ;
}

