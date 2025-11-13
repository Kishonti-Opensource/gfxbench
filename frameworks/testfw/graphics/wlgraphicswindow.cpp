/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "graphics/wlgraphicswindow.h"
#include "ng/log.h"

#include <fcntl.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <poll.h>


/******************************/
/******XDG Window Manager******/
/******************************/

static void wm_ping(void* data, struct xdg_wm_base* xdg_wm_base, uint32_t serial)
{
    (void)data;
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener wm_base_listener = { wm_ping };

/******************************/
/*********XDG Surface**********/
/******************************/

static void surface_configure(void* data, struct xdg_surface* xdg_surface, uint32_t serial)
{
    (void)data;

    xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener surface_listener = { surface_configure };

/******************************/
/********XDG Toplevel**********/
/******************************/

static void toplevel_configure(void* data, struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, struct wl_array* states)
{
    WaylandData* waylandData = static_cast<WaylandData*>(data);
    (void)xdg_toplevel;
    (void)states;

    if (!width && !height)
        return;

    if (waylandData->width != width || waylandData->height != height)
    {
        waylandData->width = width;
        waylandData->height = height;

        wl_egl_window_resize(waylandData->eglWindow, width, height, 0, 0);
        wl_surface_commit(waylandData->surf);
    }
}

static void toplevel_close(void* data, struct xdg_toplevel* xdg_toplevel)
{
    (void)xdg_toplevel;

    WaylandData* waylandData = static_cast<WaylandData*>(data);

    waylandData->running = 0;
}

static const struct xdg_toplevel_listener toplevel_listener = { toplevel_configure, toplevel_close };


/// WLRegistyListener
static void registry_add_object(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
    WaylandData* waylandData = static_cast<WaylandData*>(data);
    if (!strcmp(interface, "wl_compositor"))
    {
        waylandData->compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        waylandData->xdg_wm_base = static_cast<xdg_wm_base*>(wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));


        xdg_wm_base_add_listener(waylandData->xdg_wm_base, &wm_base_listener, waylandData);
    }
}
static void registry_remove_object(void* data, wl_registry* registry, uint32_t name)
{
}
static wl_registry_listener s_registry_listener = { &registry_add_object, &registry_remove_object };


WLGraphicsWindow::WLGraphicsWindow(bool need_egl)
    : createEGL(need_egl)
    , closeRequested_(false)
{
}


WLGraphicsWindow::~WLGraphicsWindow()
{
    destroy();
}


bool WLGraphicsWindow::create(int width, int height, const std::string& title)
{
    NGLOG_INFO("WLGraphicsWindow::create");
    if (width <= 0 || height <= 0)
    {
        NGLOG_ERROR("WLGraphicsWindow: width and height must be greater than zero (%s, %s)", width, height);
        return false;
    }
    memset(&waylandData, 0, sizeof(waylandData));
    waylandData.fd = -1;

    waylandData.running = 1;

    waylandData.display = wl_display_connect(NULL);
    if (waylandData.display == nullptr)
    {
        fprintf(stderr, "Coudn't initalize wayland %d!\n", errno);
        return false;
    }
    waylandData.registry = wl_display_get_registry(waylandData.display);
    wl_registry_add_listener(waylandData.registry, &s_registry_listener, &waylandData);

    wl_display_roundtrip(waylandData.display);
    if (!waylandData.compositor || !waylandData.xdg_wm_base)
    {
        printf("invalid %p %p\n", waylandData.compositor, waylandData.xdg_wm_base);
        return false;
    }
    waylandData.fd = wl_display_get_fd(waylandData.display);
    if (waylandData.fd < 0)
    {
        printf("invalid wayland file descriptor %d\n", waylandData.fd);
        return false;
    }

    waylandData.surf = wl_compositor_create_surface(waylandData.compositor);

    waylandData.xdg_surface = xdg_wm_base_get_xdg_surface(waylandData.xdg_wm_base, waylandData.surf);
    xdg_surface_add_listener(waylandData.xdg_surface, &surface_listener, NULL);
    waylandData.xdg_toplevel = xdg_surface_get_toplevel(waylandData.xdg_surface);

    xdg_toplevel_set_title(waylandData.xdg_toplevel, "testfwapp");
    xdg_toplevel_add_listener(waylandData.xdg_toplevel, &toplevel_listener, &waylandData);
    wl_surface_commit(waylandData.surf);


    pollEvents();

    static wl_region* region;
    region = wl_compositor_create_region(waylandData.compositor);

    wl_region_add(region, 0, 0, width, height);
    wl_surface_set_opaque_region(waylandData.surf, region);

    if (createEGL)
    {
        waylandData.eglWindow = wl_egl_window_create(waylandData.surf, width, height);
    }
    waylandData.width = width;
    waylandData.height = height;
    return true;
}


WLEGLWindow* WLGraphicsWindow::handle()
{
    return waylandData.eglWindow;
}

WLDisplay* WLGraphicsWindow::display()
{
    return waylandData.display;
}

WLSurface* WLGraphicsWindow::surface()
{
    return waylandData.surf;
}

void WLGraphicsWindow::destroy()
{
    NGLOG_INFO("WLGraphicsWindow::destroy");
    if (waylandData.eglWindow)
    {
        wl_egl_window_destroy(waylandData.eglWindow);
        waylandData.eglWindow = NULL;
    }

    if (waylandData.xdg_wm_base)
    {
        xdg_wm_base_destroy(waylandData.xdg_wm_base);
        waylandData.xdg_wm_base = NULL;
    }
    if (waylandData.surf)
    {
        wl_surface_destroy(waylandData.surf);
        waylandData.surf = NULL;
    }
    if (waylandData.registry)
    {
        wl_registry_destroy(waylandData.registry);
        waylandData.registry = NULL;
    }
    if (waylandData.display)
    {
        wl_display_disconnect(waylandData.display);
        waylandData.display = NULL;
    }
}

void WLGraphicsWindow::pollEvents()
{
    int timeout = 0;
    struct pollfd fds = { 0 };
    wl_display_dispatch_pending(waylandData.display);
    wl_display_flush(waylandData.display);

    fds.fd = waylandData.fd;
    fds.events = POLLIN | POLLOUT | POLLERR | POLLHUP;


    if (poll(&fds, 1, timeout) > 0)
    {
        if (fds.revents & (POLLERR | POLLHUP))
        {
            close(waylandData.fd);
            waylandData.fd = -1;
        }

        if (fds.revents & POLLIN)
            wl_display_dispatch(waylandData.display);
        if (fds.revents & POLLOUT)
            wl_display_flush(waylandData.display);
    }
}

bool WLGraphicsWindow::shouldClose()
{
    return closeRequested_;
}

int WLGraphicsWindow::width()
{
    return waylandData.width;
}

int WLGraphicsWindow::height()
{
    return waylandData.height;
}

void WLGraphicsWindow::requestClose()
{
    NGLOG_INFO("WLGraphicsWindow::requestClose");
    closeRequested_ = true;
}
