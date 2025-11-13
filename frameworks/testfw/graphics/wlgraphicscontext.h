/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef WLGRAPHICSCONTEXT_H
#define WLGRAPHICSCONTEXT_H

#include <string>

#include <wayland-client.h>
#include <wayland-egl.h>
#include "graphicscontext.h"

class WLGraphicsContext : public GraphicsContext
{
public:
    WLGraphicsContext(GraphicsContext::GraphicsType type = GraphicsContext::VULKAN, int major = 1, int minor = 0, int flags = 0)
        : type_(type)
        , major_(major)
        , minor_(minor)
        , flags_(flags)
    {
		}

    wl_surface* surface()
    {
		return surface_;
	}
    wl_display* display()
    {
		return display_;
	}
    void init(wl_display *display, wl_surface *surface)
    {
		display_ = display;
		surface_ = surface;
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

  
private:
    wl_surface *surface_;
	wl_display *display_;

    GraphicsType type_;
    int major_;
    int minor_;
    int flags_;
};


#endif  
