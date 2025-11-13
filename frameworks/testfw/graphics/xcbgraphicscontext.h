/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef XCB_GRAPHICS_CONTEXT_H_
#define XCB_GRAPHICS_CONTEXT_H_

#include "graphicscontext.h"
#include <xcb/xcb.h>
class XCBGraphicsContext : public GraphicsContext
{
public:
    XCBGraphicsContext(GraphicsContext::GraphicsType type = GraphicsContext::VULKAN, int major = 1, int minor = 0, int flags = 0)
        : type_(type)
        , major_(major)
        , minor_(minor)
        , flags_(flags)
    {}

    void init(xcb_connection_t *con, xcb_window_t win, xcb_screen_t* scr)
    {
        connection_ = con;
        window_ = win;
        screen_ = scr;
    }

    bool isValid()
    {
        return true;
    }
    bool makeCurrent()
    {
        return true;
    }
    bool detachThread()
    {
        return true;
    }
    bool swapBuffers()
    {
        return true;
    }
    GraphicsType type()
    {
        return type_;
    }
    int versionMajor()
    {
        return major_;
    }
    int versionMinor()
    {
        return minor_;
    }
    bool hasFlag(int flag)
    {
        return (flags_ & flag) != 0;
    }

    xcb_connection_t* getConnection()
    {
        return connection_;
    }
    xcb_window_t getWindow()
    {
        return window_;
    }
    xcb_screen_t* screen()
    {
        return screen_;
    }
private:
    GraphicsType type_;
    int major_;
    int minor_;
    int flags_;

    xcb_connection_t *connection_;
    xcb_window_t window_;
    xcb_screen_t* screen_;
};


#endif  // XCB_GRAPHICS_CONTEXT_H_
