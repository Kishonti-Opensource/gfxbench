/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glinfo.h"

#include "jsonvisitor.h"



using namespace tfw;



EGLConfigInfo::EGLConfigInfo()
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




GLESInfo::GLESInfo()
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



GLInfoCollector::GLInfoCollector()
    : hasGL_(false)
    , hasEGL_(false)
    , hasGLES_(false)
{}



class GlJsonVisitor : public JsonVisitor
{
public:
    GlJsonVisitor(ng::JsonValue& jsonValue) : JsonVisitor(jsonValue) {}
    using JsonVisitor::operator();
    void operator()(const std::string& name, const std::vector<EGLConfigInfo>& value) {
        ng::JsonValue array;
        array.resize(0);
        for (size_t i = 0; i < value.size(); ++i) {
            ng::JsonValue object;
            applyVisitor(value.at(i), JsonVisitor(object));
            array.push_back(object);
        }
        mJsonValue[name.c_str()] = array;
    }
};



std::string GLInfoCollector::serializeGL() const
{
    if (hasGL_) {
        ng::JsonValue jsonValue;
        applyVisitor(gl_, GlJsonVisitor(jsonValue));
        return jsonValue.toString();
    } else {
        return "{}";
    }
}



std::string GLInfoCollector::serializeEGL() const
{
    if (hasEGL_) {
        ng::JsonValue jsonValue;
        applyVisitor(egl_, GlJsonVisitor(jsonValue));
        return jsonValue.toString();
    } else {
        return "{}";
    }
}



std::string GLInfoCollector::serializeGLES() const
{
    if (hasGLES_) {
        ng::JsonValue jsonValue;
        applyVisitor(gles_, GlJsonVisitor(jsonValue));
        return jsonValue.toString();
    } else {
        return "{}";
    }
}



bool GLInfoCollector::isOpenGLAvailable() const
{
    return hasGL_;
}



bool GLInfoCollector::isOpenGLESAvailable() const
{
    return hasGLES_;
}



bool GLInfoCollector::isEGLAvailable() const
{
    return hasEGL_;
}



const GLESInfo &GLInfoCollector::gl() const
{
    return gl_;
}



const EGLInfo &GLInfoCollector::egl() const
{
    return egl_;
}



const GLESInfo &GLInfoCollector::gles() const
{
    return gles_;
}
