/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef EGL_GRAPHICS_CONTEXT_H_
#define EGL_GRAPHICS_CONTEXT_H_

#include "glformat.h"
#include "graphicscontext.h"
#include "graphicswindow.h"
#include <stdint.h>
#include <vector>

#ifdef DISPLAY_PROTOCOL_WAYLAND
#include <wayland-egl.h>
#endif

#ifdef HAVE_EPOXY
#include "epoxy/egl.h"
#else
#include <EGL/egl.h>
#endif


class EGLGraphicsContext : public GraphicsContext
{
public:
    EGLGraphicsContext();
    EGLGraphicsContext(const tfw::GLFormat &format);
    virtual ~EGLGraphicsContext();
    void setUseDefaultChooseConfig(bool useDefaultChooseConfig);
    void setFormat(const tfw::GLFormat &format);
    const tfw::GLFormat &format() const;
    bool initWindowSurface(EGLNativeWindowType window);
#ifdef DISPLAY_PROTOCOL_WAYLAND
    bool initWaylandWindowSurface(EGLNativeDisplayType dspy, EGLNativeWindowType wndw);
#endif
    bool initVulkanWindowSurface(EGLNativeWindowType window);
    bool initPBufferSurface(int32_t width, int32_t height);
    void destroy();
    bool isValid();
    bool setContextVersion(int32_t version_major, int32_t version_minor);

    int surfaceWidth() const { return surfaceWidth_; }
    int surfaceHeight() const { return surfaceHeight_; }
    int selectedConfig() const { return selectedConfig_; }
    const char *clientApis() const;
    const char *vendor() const;
    const char *extensions() const;
    const char *version() const;
    void setVerboseLogging(bool enable) { verboseLogging_ = enable; }
    EGLDisplay displayHandle() { return display_; }
    EGLContext contextHandle() { return context_; }

    // from GraphicsContext
    virtual bool makeCurrent();
    virtual bool detachThread();
    virtual bool swapBuffers();
    virtual GraphicsType type();
    virtual int versionMajor();
    virtual int versionMinor();
    virtual bool hasFlag(int flag);

    EGLNativeWindowType getWindow() { return window_; }

private:
    EGLConfig chooseConfigDefault(const EGLint *attrs = 0) const;
    EGLConfig chooseConfig() const;
    EGLConfig chooseBestMatchingConfig() const;
    bool updateVersionFromContext();
    void initContextAttribs();
    bool isGLESCompatible(int32_t major, int32_t minor) const;

    EGLNativeWindowType window_;
    EGLDisplay display_;
    EGLConfig config_;
    EGLSurface surface_;
    EGLContext context_;
    int32_t versionMajor_;
    int32_t versionMinor_;
    tfw::GLFormat format_;

    int surfaceWidth_;
    int surfaceHeight_;
    int selectedConfig_;
    bool useDefaultChooseConfig_;
    bool ispbuffer_;
    bool verboseLogging_;
    std::vector<EGLint> contextAttribs_;
    int eglVersionMajor_;
    int eglVersionMinor_;
    bool vulkan_;
};


#endif  // EGL_GRAPHICS_CONTEXT_H_
