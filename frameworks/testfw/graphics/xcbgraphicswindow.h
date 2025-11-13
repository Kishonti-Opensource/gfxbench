/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef XCBGRAPHICSWINDOW_H_
#define XCBGRAPHICSWINDOW_H_

#include <string>

#include <xcb/xcb.h>
#include "graphics/graphicswindow.h"

class XCBGraphicsWindow : public GraphicsWindow
{
public:
    XCBGraphicsWindow();
    virtual ~XCBGraphicsWindow();
    bool create(int width, int height, const std::string &title);
    virtual void pollEvents();
    virtual bool shouldClose();
    virtual void requestClose();
    virtual int width();
    virtual int height();
    xcb_window_t window()
    {
        return window_;
    }
    xcb_connection_t* connection()
    {
        return connection_;
    }
    void schedule_xcb_repaint();
    xcb_screen_t* screen()
    {
        return screen_;
    }
    void destroy();
private:
    xcb_connection_t *connection_;
    xcb_screen_t* screen_;
    xcb_window_t window_;

    int width_;
    int height_;
    bool closeRequested_;
};

#endif  // XCBGRAPHICSWINDOW_H_

