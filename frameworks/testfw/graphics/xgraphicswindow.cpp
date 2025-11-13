/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "graphics/xgraphicswindow.h"
#include "ng/log.h"

XGraphicsWindow::XGraphicsWindow()
    : display_(0)
    , window_(0)
    , width_(0)
    , height_(0)
    , closeRequested_(false)
{
}


XGraphicsWindow::~XGraphicsWindow()
{
    destroy();
}


bool XGraphicsWindow::create(int width, int height, const std::string &title)
{
    if (width <= 0 || height <= 0)
    {
        NGLOG_ERROR("XGraphicsWindow: width, and height must be greater than zero (%s, %s)", width, height);
        return false;
    }
    XSetWindowAttributes wa;
    Display *dpy = XOpenDisplay(NULL);

    wa.background_pixel = 0xFF95FF94;
    wa.border_pixel = 0;
    wa.event_mask = StructureNotifyMask | ExposureMask;

    Window window = XCreateWindow(dpy,
            RootWindow(dpy, DefaultScreen(dpy)),
            0,
            0,
            width,
            height,
            0,
            24,//red + green + blue,
            InputOutput,
            CopyFromParent,
            CWBackPixel | CWBorderPixel | CWEventMask,
            &wa);
    XStoreName(dpy, window, title.c_str());
    XMapWindow(dpy, window);
    XFlush(dpy);

    /* This is required to get window close notifications */
    Atom wmDeleteMessage = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, window, &wmDeleteMessage, 1);

    display_ = dpy;
    window_ = window;
    width_ = width;
    height_ = height;
    return true;
}


Window XGraphicsWindow::handle()
{
    return window_;
}


void XGraphicsWindow::destroy()
{
    if (window_ != 0) {
        XDestroyWindow(display_, window_);
        XCloseDisplay(display_);
    }
}

void XGraphicsWindow::pollEvents()
{
    XEvent event;
    while (XPending(display_)) {
        XNextEvent(display_, &event);
        NGLOG_INFO("in loop");
        switch (event.type)
        {
        case ClientMessage:
            closeRequested_ = true;
            break;
        }
    }
}

bool XGraphicsWindow::shouldClose()
{
    return closeRequested_;
}

int XGraphicsWindow::width()
{
    return width_;
}

int XGraphicsWindow::height()
{
    return height_;
}

void XGraphicsWindow::requestClose()
{
    closeRequested_ = true;
}
