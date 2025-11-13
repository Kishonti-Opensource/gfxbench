/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLINFO_H
#define GLINFO_H

#include <cstdint>
#include <string>
#include <vector>

namespace sysinf
{



class GLESInfo
{
public:
    std::string vendor;
    std::string version;
    std::string renderer;
    std::string shadingLanguageVersion;
    std::vector<std::string> extensions;
    int32_t majorVersion;
    int32_t minorVersion;
    
    int32_t aliasedLineWidthRange[2];
    int32_t aliasedPointSizeRange[2];
    int32_t maxViewportDims[2];
    int32_t maxComputeWorkGroupCount[3];
    int32_t maxComputeWorkGroupSize[3];
    std::string implementationColorReadType;
    std::string implementationColorReadFormat;
    std::vector<std::string> compressedTextureFormats;
    std::vector<std::string> programBinaryFormats;
    std::vector<std::string> shaderBinaryFormats;
    std::vector<std::pair<std::string, int64_t> > maxi;
    bool shaderCompiler;
    
    GLESInfo()
    : majorVersion(0)
    , minorVersion(0)
    , shaderCompiler(false)
    {
        for (int i = 0; i < 3; ++i)
        {
            maxComputeWorkGroupCount[i] = -1;
            maxComputeWorkGroupSize[i] = -1;
        }
        for (int i = 0; i < 2; ++i)
        {
            aliasedLineWidthRange[i] = -1;
            aliasedPointSizeRange[i] = -1;
            maxViewportDims[i] = -1;
        }
    }
    
    template<class F> void applyVisitor(F visitor) const {
        visitor("major", vendor);
        visitor("minor", version);
        visitor("GL_VENDOR", vendor);
        visitor("GL_VERSION", version);
        visitor("GL_RENDERER", renderer);
        visitor("GL_EXTENSIONS", extensions);
        
        visitor("GL_ALIASED_LINE_WIDTH_RANGE",
            std::vector<int32_t>(aliasedLineWidthRange, aliasedLineWidthRange + 2));
        visitor("GL_ALIASED_POINT_SIZE_RANGE",
            std::vector<int32_t>(aliasedPointSizeRange, aliasedPointSizeRange + 2));
        visitor("GL_MAX_VIEWPORT_DIMS",
            std::vector<int32_t>(maxViewportDims, maxViewportDims + 2));
        visitor("GL_NUM_COMPRESSED_TEXTURE_FORMATS", compressedTextureFormats.size());
        visitor("GL_COMPRESSED_TEXTURE_FORMATS", compressedTextureFormats);
        visitor("GL_NUM_SHADER_BINARY_FORMATS", shaderBinaryFormats.size());
        visitor("GL_SHADER_BINARY_FORMATS", shaderBinaryFormats);
        
        if (majorVersion == 3 && minorVersion == 0) {
            visitor("GL_SHADING_LANGUAGE_VERSION", shadingLanguageVersion);
            visitor("GL_NUM_PROGRAM_BINARY_FORMATS", programBinaryFormats.size());
            visitor("GL_PROGRAM_BINARY_FORMATS", programBinaryFormats);
        }
        if (majorVersion == 3 && minorVersion == 1) {
            visitor("GL_MAX_COMPUTE_WORK_GROUP_COUNT",
                std::vector<int32_t>(maxComputeWorkGroupCount, maxComputeWorkGroupCount + 3));
            visitor("GL_MAX_COMPUTE_WORK_GROUP_SIZE",
                std::vector<int32_t>(maxComputeWorkGroupSize, maxComputeWorkGroupSize + 3));
        }
        for (size_t i = 0; i < maxi.size(); ++i) {
            visitor(maxi[i].first, maxi[i].second);
        }
        visitor("GL_SHADER_COMPILER", shaderCompiler);
        // GL_IMPLEMENTATION_COLOR_READ_TYPE/FORMAT might change per config
        //visitor("GL_IMPLEMENTATION_COLOR_READ_TYPE", implementationColorReadType);
        //visitor("GL_IMPLEMENTATION_COLOR_READ_FORMAT", implementationColorReadFormat);
    }
};



class EGLConfigInfo
{
public:
    int32_t alphaSize;
    int32_t alphaMaskSize;
    bool bindToTextureRgb;
    bool bindToTextureRgba;
    int32_t blueSize;
    int32_t bufferSize;
    std::string colorBufferType;
    std::string configCaveat;
    int32_t configId;
    std::string conformant;
    int32_t depthSize;
    int32_t greenSize;
    int32_t level;
    int32_t luminanceSize;
    int32_t maxPBufferWidth;
    int32_t maxPBufferHeight;
    int32_t maxPBufferPixels;
    int32_t maxSwapInterval;
    int32_t minSwapInterval;
    bool nativeRenderable;
    int32_t nativeVisualId;
    int32_t nativeVisualType;
    int32_t redSize;
    std::string renderableType;
    int32_t sampleBuffers;
    int32_t samples;
    int32_t stencilSize;
    std::string surfaceType;
    std::string transparentType;
    int32_t transparentRedValue;
    int32_t transparentGreenValue;
    int32_t transparentBlueValue;
    
    EGLConfigInfo()
    : alphaSize(-1)
    , alphaMaskSize(-1)
    , bindToTextureRgb(false)
    , bindToTextureRgba(false)
    , blueSize(-1)
    , bufferSize(-1)
    , configId(-1)
    , depthSize(-1)
    , greenSize(-1)
    , level(-1)
    , luminanceSize(-1)
    , maxPBufferWidth(-1)
    , maxPBufferHeight(-1)
    , maxPBufferPixels(-1)
    , maxSwapInterval(-1)
    , minSwapInterval(-1)
    , nativeRenderable(false)
    , nativeVisualId(-1)
    , nativeVisualType(-1)
    , redSize(-1)
    , sampleBuffers(-1)
    , samples(-1)
    , stencilSize(-1)
    , transparentRedValue(-1)
    , transparentGreenValue(-1)
    , transparentBlueValue(-1)
    {}
    
    template<class F> void applyVisitor(F visitor) const {
        visitor("EGL_ALPHA_SIZE", alphaSize);
        visitor("EGL_ALPHA_MASK_SIZE", alphaMaskSize);
        visitor("EGL_BIND_TO_TEXTURE_RGB", bindToTextureRgb);
        visitor("EGL_BIND_TO_TEXTURE_RGBA", bindToTextureRgba);
        visitor("EGL_BLUE_SIZE", blueSize);
        visitor("EGL_BUFFER_SIZE", bufferSize);
        visitor("EGL_COLOR_BUFFER_TYPE", colorBufferType);
        visitor("EGL_CONFIG_CAVEAT", configCaveat);
        visitor("EGL_CONFIG_ID", configId);
        visitor("EGL_CONFORMANT", conformant);
        visitor("EGL_DEPTH_SIZE", depthSize);
        visitor("EGL_GREEN_SIZE", greenSize);
        visitor("EGL_LEVEL", level);
        visitor("EGL_LUMINANCE_SIZE", luminanceSize);
        visitor("EGL_MAX_PBUFFER_WIDTH", maxPBufferWidth);
        visitor("EGL_MAX_PBUFFER_HEIGHT", maxPBufferHeight);
        visitor("EGL_MAX_PBUFFER_PIXELS", maxPBufferPixels);
        visitor("EGL_MAX_SWAP_INTERVAL", maxSwapInterval);
        visitor("EGL_MIN_SWAP_INTERVAL", minSwapInterval);
        visitor("EGL_NATIVE_RENDERABLE", nativeRenderable);
        visitor("EGL_NATIVE_VISUAL_ID", nativeVisualId);
        visitor("EGL_NATIVE_VISUAL_TYPE", nativeVisualType);
        visitor("EGL_RED_SIZE", redSize);
        visitor("EGL_RENDERABLE_TYPE", renderableType);
        visitor("EGL_SAMPLE_BUFFERS", sampleBuffers);
        visitor("EGL_SAMPLES", samples);
        visitor("EGL_STENCIL_SIZE", stencilSize);
        visitor("EGL_SURFACE_TYPE", surfaceType);
        visitor("EGL_TRANSPARENT_TYPE", transparentType);
        visitor("EGL_TRANSPARENT_RED_VALUE", transparentRedValue);
        visitor("EGL_TRANSPARENT_GREEN_VALUE", transparentGreenValue);
        visitor("EGL_TRANSPARENT_BLUE_VALUE", transparentBlueValue);
    }
};



class EGLInfo
{
public:
    std::string vendor;
    std::string version;
    std::vector<std::string> clientApis;
    std::vector<std::string> extensions;
    std::vector<EGLConfigInfo> configs;
    
    template<class F> void applyVisitor(F visitor) const {
        visitor("major", vendor);
        visitor("minor", version);
        visitor("EGL_VENDOR", vendor);
        visitor("EGL_VERSION", version);
        visitor("EGL_CLIENT_APIS", clientApis);
        visitor("EGL_EXTENSIONS", extensions);
        visitor("configs", configs);
    }
};



}

#endif  // GLINFO_H
