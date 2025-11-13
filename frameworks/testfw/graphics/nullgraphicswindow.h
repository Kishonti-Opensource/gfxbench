/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NULLGRAPHICSWINDOW_H_
#define NULLGRAPHICSWINDOW_H_

#include "graphics/graphicswindow.h"

class NullGraphicsWindow : public GraphicsWindow
{
public:
    NullGraphicsWindow(int width, int height, bool shouldClose = false)
        : shouldClose_(shouldClose)
        , width_(width)
        , height_(height)
    {
        if(width_ < 1 || height_ < 1)
        {
            width_ = 1280;
            height_ = 720;
        }
    }
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
    virtual int width()
    {
        return width_;
    }
    virtual int height()
    {
        return height_;
    }
    virtual void *handle()
    {
        return 0;
    }
private:
    bool shouldClose_;
    int width_;
    int height_;
};

#endif  // NULLGRAPHICSWINDOW_H_
