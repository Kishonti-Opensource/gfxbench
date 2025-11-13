/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef ANWGRAPHICSWINDOW_H_
#define ANWGRAPHICSWINDOW_H_

#include "graphics/graphicswindow.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-length-array"
#pragma clang diagnostic ignored "-Wextra-semi"
#pragma clang diagnostic ignored "-Wvariadic-macros"
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#pragma clang diagnostic ignored "-Wmismatched-tags"
#endif

#include <gui/SurfaceComposerClient.h>
#include <gui/SurfaceControl.h>

//#include <utils/UniquePtr.h> // in nougat doesn't have this one

#include <ui/DisplayInfo.h>
#include <gui/Surface.h>
#include <gui/ISurfaceComposer.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <string>

class ANWGraphicsWindow : public GraphicsWindow
{
public:
    ANWGraphicsWindow();
    void create(int width, int height, const std::string &title, int32_t pixelFormat );
    virtual bool shouldClose()
    {
        return shouldClose_;
    }
    virtual void pollEvents()
    {
    }
    virtual void requestClose()
    {
        shouldClose_ = true;
    }
    ANativeWindow *handle();
    virtual int width();
    virtual int height();

private:
    bool shouldClose_;
    android::sp<android::SurfaceComposerClient> composerClient_;
    android::sp<android::SurfaceControl> surfaceControl_;
    int width_;
    int height_;
};

#endif  // ANWGRAPHICSWINDOW_H_
