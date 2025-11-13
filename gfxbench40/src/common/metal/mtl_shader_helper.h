/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_SHADER_HELPER_H
#define MTL_SHADER_HELPER_H


#include <string>
#include <Metal/Metal.h>

namespace MetalRender {
    id <MTLLibrary> LoadShaderLibraryFromFile(id <MTLDevice> device, const char* filename) ;
    id <MTLLibrary> LoadShaderLibraryFromString(id <MTLDevice> device, const std::string & source_str);
}

#endif  // MTL_SHADER_HELPER_H
