/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef XGRAPHICSWINDOW_H_
#define XGRAPHICSWINDOW_H_

#include <string>

#include <X11/Xlib.h>
#include "graphics/graphicswindow.h"

class XGraphicsWindow : public GraphicsWindow
{
public:
    XGraphicsWindow();
    virtual ~XGraphicsWindow();
    bool create(int width, int height, const std::string &title);
    virtual void pollEvents();
    virtual bool shouldClose();
    virtual void requestClose();
    virtual int width();
    virtual int height();
    Window handle();
    void destroy();
private:
    Display *display_;
    Window window_;
    int width_;
    int height_;
    bool closeRequested_;
};

#endif  // XGRAPHICSWINDOW_H_
