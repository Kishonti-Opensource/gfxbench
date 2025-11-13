/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "graphics/anwgraphicswindow.h"
#include "ng/require.h"

using namespace android;

ANWGraphicsWindow::ANWGraphicsWindow()
    : shouldClose_(false)
    , width_(0)
    , height_(0)
{}

void ANWGraphicsWindow::create(int width, int height, const std::string &title, int32_t pixelFormat )
{
    struct XXX_DisplayInfo : public DisplayInfo {
        /*
           Make DisplayInfo struct larger to prevent stack overflow caused by binary incompatibility between versions. I'm going to hell for this.
           see commits: ANDROID/frameworks/native
             5773d3f5b2
             dd3cb84cfb
             c666cae2d5
             4125036157
             ...
        */
        char xxx[1024];
    } dpyinfo;
    composerClient_ = new SurfaceComposerClient;
    require(NO_ERROR == composerClient_->initCheck());

    if (width > 0 && height > 0)
    {
        width_ = width;
        height_ = height;
    }
    else
    {
        sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain);
        require(NO_ERROR == SurfaceComposerClient::getDisplayInfo(display, &dpyinfo));

        width_ = dpyinfo.w;
        height_ = dpyinfo.h;
        if (dpyinfo.orientation == 1 || dpyinfo.orientation == 3) {
            std::swap(width_, height_);
        }
    }
    surfaceControl_ = composerClient_->createSurface(String8(title.c_str()), width_, height_, pixelFormat, 0);
    require(surfaceControl_ != NULL);
    require(surfaceControl_->isValid());
    SurfaceComposerClient::openGlobalTransaction();
    require(NO_ERROR == surfaceControl_->setLayer(0x7FFFFFFF));
    require(NO_ERROR == surfaceControl_->show());
    SurfaceComposerClient::closeGlobalTransaction();
}


ANativeWindow *ANWGraphicsWindow::handle()
{
    sp<ANativeWindow> window = surfaceControl_->getSurface();
    return window.get();
}

int ANWGraphicsWindow::width()
{
    return width_;
}

int ANWGraphicsWindow::height()
{
    return height_;
}
