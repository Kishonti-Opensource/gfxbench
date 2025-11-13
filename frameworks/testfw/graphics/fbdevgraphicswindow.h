/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef FBDEVGRAPHICSWINDOW_H_
#define FBDEVGRAPHICSWINDOW_H_

#include "graphics/nullgraphicswindow.h"

class FBDEVGraphicsWindow : public NullGraphicsWindow
{
public:
    FBDEVGraphicsWindow(int width, int height, bool shouldClose = false)
        : NullGraphicsWindow(width, height, shouldClose)
    {
        if(width < 1 || height < 1)
        {
            width = 1280;
            height = 720;
        }

        fb_.w = width;
        fb_.h = height;
    }
    virtual void *handle()
    {
        return &fb_;
    }
private:
    struct fbdev
    {
        uint16_t w;
        uint16_t h;
    } fb_;
};

#endif  // FBDEVGRAPHICSWINDOW_H_
