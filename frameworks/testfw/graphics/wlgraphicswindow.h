/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once

#include <string>

#include <wayland-client.h>
#include <wayland-egl.h>
#include "graphics/graphicswindow.h"
#include "xdg-shell-client-protocol.h"

typedef struct wl_display WLDisplay;
typedef struct wl_surface WLSurface;
typedef struct wl_egl_window WLEGLWindow;

struct WaylandData
{
    wl_display* display;
    wl_surface* surf;
    wl_registry* registry;
    wl_compositor* compositor;
    wl_egl_window* eglWindow;

    struct xdg_wm_base* xdg_wm_base;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;

    uint8_t running;
    int fd;

    int width;
    int height;
};

class WLGraphicsWindow : public GraphicsWindow
{
public:
    struct WaylandData waylandData;

    WLGraphicsWindow(bool createEGL);
    virtual ~WLGraphicsWindow();
    bool create(int width, int height, const std::string& title);
    virtual void pollEvents();
    virtual bool shouldClose();
    virtual void requestClose();
    virtual int width();
    virtual int height();
    WLEGLWindow* handle();
    WLSurface* surface();
    WLDisplay* display();
    void destroy();

private:
    bool createEGL = false;

    bool closeRequested_;
};
