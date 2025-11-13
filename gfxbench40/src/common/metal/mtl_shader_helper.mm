/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#import "mtl_shader_helper.h"
#import "mtl_globals.h"

#include "kcl_io.h"

namespace MetalRender {
    
id <MTLLibrary> LoadShaderLibraryFromFile(id <MTLDevice> device, const char* filename)
{
    KCL::AssetFile shaderfile(filename) ;
    
    return LoadShaderLibraryFromString(device, shaderfile.GetBuffer()) ;
}
    
    
id <MTLLibrary> LoadShaderLibraryFromString(id <MTLDevice> device, const std::string & source_str)
{
    NSError *pError = nil;
    
    MTLCompileOptions* compilerOptions  = [[MTLCompileOptions alloc] init];
    
    NSString* ns_shader_source = [NSString stringWithCString:source_str.c_str() encoding:NSUTF8StringEncoding];
    id <MTLLibrary> ShaderLibrary = [device newLibraryWithSource:ns_shader_source options:compilerOptions error:&pError];
    
    if (ShaderLibrary == nil)
    {
        NSLog(@" error => %@ ", pError ) ;
    }
    
    releaseObj(compilerOptions) ;
    releaseObj(ns_shader_source) ;
    
    return  ShaderLibrary ;
}

}

