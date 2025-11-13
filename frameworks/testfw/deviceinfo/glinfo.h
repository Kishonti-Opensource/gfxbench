/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLINFO_H_
#define GLINFO_H_

#include <cstdint>
#include <string>
#include <vector>

namespace tfw
{



class GLESInfo
{
public:
    GLESInfo();
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
};

template<class F> void applyVisitor(const GLESInfo& info, F visitor) {
    visitor("GL_VERSION", info.version);
    visitor("GL_VENDOR", info.vendor);
    visitor("GL_RENDERER", info.renderer);
    visitor("GL_EXTENSIONS", info.extensions);

    visitor("GL_ALIASED_LINE_WIDTH_RANGE", std::vector<int32_t>(
            info.aliasedLineWidthRange, info.aliasedLineWidthRange + 2));
    visitor("GL_ALIASED_POINT_SIZE_RANGE", std::vector<int32_t>(
            info.aliasedPointSizeRange, info.aliasedPointSizeRange + 2));
    visitor("GL_MAX_VIEWPORT_DIMS", std::vector<int32_t>(
            info.maxViewportDims, info.maxViewportDims + 2));
    visitor("GL_COMPRESSED_TEXTURE_FORMATS", info.compressedTextureFormats);
    visitor("GL_SHADER_BINARY_FORMATS", info.shaderBinaryFormats);

    if (info.majorVersion == 3 && info.minorVersion == 0) {
        visitor("GL_SHADING_LANGUAGE_VERSION", info.shadingLanguageVersion);
        visitor("GL_PROGRAM_BINARY_FORMATS", info.programBinaryFormats);
    }
    if (info.majorVersion == 3 && info.minorVersion == 1) {
        visitor("GL_MAX_COMPUTE_WORK_GROUP_COUNT", std::vector<int32_t>(
                info.maxComputeWorkGroupCount, info.maxComputeWorkGroupCount + 3));
        visitor("GL_MAX_COMPUTE_WORK_GROUP_SIZE", std::vector<int32_t>(
                info.maxComputeWorkGroupSize, info.maxComputeWorkGroupSize + 3));
    }
    for (size_t i = 0; i < info.maxi.size(); ++i) {
        visitor(info.maxi[i].first, info.maxi[i].second);
    }
    visitor("GL_SHADER_COMPILER", info.shaderCompiler);
    // GL_IMPLEMENTATION_COLOR_READ_TYPE/FORMAT might change per config
    //visitor("GL_IMPLEMENTATION_COLOR_READ_TYPE", implementationColorReadType);
    //visitor("GL_IMPLEMENTATION_COLOR_READ_FORMAT", implementationColorReadFormat);
}



class EGLConfigInfo
{
public:
    EGLConfigInfo();
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
};

template<class F> void applyVisitor(const EGLConfigInfo& info, F visitor) {
    visitor("EGL_ALPHA_SIZE", info.alphaSize);
    visitor("EGL_ALPHA_MASK_SIZE", info.alphaMaskSize);
    visitor("EGL_BIND_TO_TEXTURE_RGB", info.bindToTextureRgb);
    visitor("EGL_BIND_TO_TEXTURE_RGBA", info.bindToTextureRgba);
    visitor("EGL_BLUE_SIZE", info.blueSize);
    visitor("EGL_BUFFER_SIZE", info.bufferSize);
    visitor("EGL_COLOR_BUFFER_TYPE", info.colorBufferType);
    visitor("EGL_CONFIG_CAVEAT", info.configCaveat);
    visitor("EGL_CONFIG_ID", info.configId);
    visitor("EGL_CONFORMANT", info.conformant);
    visitor("EGL_DEPTH_SIZE", info.depthSize);
    visitor("EGL_GREEN_SIZE", info.greenSize);
    visitor("EGL_LEVEL", info.level);
    visitor("EGL_LUMINANCE_SIZE", info.luminanceSize);
    visitor("EGL_MAX_PBUFFER_WIDTH", info.maxPBufferWidth);
    visitor("EGL_MAX_PBUFFER_HEIGHT", info.maxPBufferHeight);
    visitor("EGL_MAX_PBUFFER_PIXELS", info.maxPBufferPixels);
    visitor("EGL_MAX_SWAP_INTERVAL", info.maxSwapInterval);
    visitor("EGL_MIN_SWAP_INTERVAL", info.minSwapInterval);
    visitor("EGL_NATIVE_RENDERABLE", info.nativeRenderable);
    visitor("EGL_NATIVE_VISUAL_ID", info.nativeVisualId);
    visitor("EGL_NATIVE_VISUAL_TYPE", info.nativeVisualType);
    visitor("EGL_RED_SIZE", info.redSize);
    visitor("EGL_RENDERABLE_TYPE", info.renderableType);
    visitor("EGL_SAMPLE_BUFFERS", info.sampleBuffers);
    visitor("EGL_SAMPLES", info.samples);
    visitor("EGL_STENCIL_SIZE", info.stencilSize);
    visitor("EGL_SURFACE_TYPE", info.surfaceType);
    visitor("EGL_TRANSPARENT_TYPE", info.transparentType);
    visitor("EGL_TRANSPARENT_RED_VALUE", info.transparentRedValue);
    visitor("EGL_TRANSPARENT_GREEN_VALUE", info.transparentGreenValue);
    visitor("EGL_TRANSPARENT_BLUE_VALUE", info.transparentBlueValue);
}



class EGLInfo
{
public:
    std::string vendor;
    std::string version;
    std::vector<std::string> clientApis;
    std::vector<std::string> extensions;
    std::vector<EGLConfigInfo> configs;
};

template<class F> void applyVisitor(const EGLInfo& info, F visitor) {
    visitor("EGL_VENDOR", info.vendor);
    visitor("EGL_VERSION", info.version);
    visitor("EGL_CLIENT_APIS", info.clientApis);
    visitor("EGL_EXTENSIONS", info.extensions);
    visitor("configs", info.configs);
}



class GLInfoCollector
{
public:
    GLInfoCollector();
    void collect();

    bool isOpenGLESAvailable() const;
    bool isOpenGLAvailable() const;
    bool isEGLAvailable() const;
    const GLESInfo &gl() const;
    const EGLInfo &egl() const;
    const GLESInfo &gles() const;
    std::string serializeGL() const;
    std::string serializeEGL() const;
    std::string serializeGLES() const;

private:
    bool hasGL_;
    bool hasEGL_;
    bool hasGLES_;
    GLESInfo gl_;
    EGLInfo egl_;
    GLESInfo gles_;
};

}

#endif  // GLINFO_H_
