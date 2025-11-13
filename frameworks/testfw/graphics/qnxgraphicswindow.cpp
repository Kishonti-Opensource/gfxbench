/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <sys/keycodes.h>
#include "qnxgraphicswindow.h"
#include "ng/log.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "ng/format.h"
#include "ng/require.h"
#include "kcl_os.h"

#define CHECK(X) {\
    int rc = (X);\
    requireex(rc == 0, FORMATSTR("%s: %s", #X, strerror(errno)));\
}


QNXGraphicsWindow::QNXGraphicsWindow()
    : context_(0)
    , window_(0)
    , screenEv_(0)
    , shouldClose_(false)
    , width_(0)
    , height_(0)
    , fullscreen_(true)
    , pixelformat_(0)
    , pixelformatstr_("auto")
    , nbuffers_(4)
    , interval_(0)
{
    CHECK(screen_create_context(&context_, SCREEN_APPLICATION_CONTEXT));
    CHECK(screen_create_window(&window_, context_));
    CHECK(screen_get_window_property_iv(window_, SCREEN_PROPERTY_FORMAT, &pixelformat_));

    switch(pixelformat_) {
        case SCREEN_FORMAT_RGB565:
             pixelformatstr_ = "rgb565";
             break;
        case SCREEN_FORMAT_RGBA8888:
        case SCREEN_FORMAT_RGBX8888:
             pixelformatstr_ = "rgba8888";
             break;
#if defined(SCREEN_FORMAT_BGRA8888) && defined(SCREEN_FORMAT_BGRX8888)
        case SCREEN_FORMAT_BGRA8888:
        case SCREEN_FORMAT_BGRX8888:
             pixelformatstr_ = "bgra8888";
             break;
#endif /* SCREEN_FORMAT_BGRA8888 && SCREEN_FORMAT_BGRA8888 */
#if defined(SCREEN_FORMAT_RGBA1010102) && defined(SCREEN_FORMAT_RGBX1010102)
        case SCREEN_FORMAT_RGBA1010102:
        case SCREEN_FORMAT_RGBX1010102:
             pixelformatstr_ = "rgba1010102";
             break;
#endif /* SCREEN_FORMAT_RGBA1010102 && SCREEN_FORMAT_RGBX1010102 */
#if defined(SCREEN_FORMAT_BGRA1010102) && defined(SCREEN_FORMAT_BGRX1010102)
        case SCREEN_FORMAT_BGRA1010102:
        case SCREEN_FORMAT_BGRX1010102:
             pixelformatstr_ = "bgra1010102";
             break;
#endif /* SCREEN_FORMAT_BGRA1010102 && SCREEN_FORMAT_BGRX1010102 */
        default:
             pixelformat_ = SCREEN_FORMAT_RGBA8888;
             pixelformatstr_ = "rgba8888";
             break;
    }
}

QNXGraphicsWindow::~QNXGraphicsWindow()
{
    if (screenEv_) {
        screen_destroy_event(screenEv_);
    }
    if (window_) {
        screen_destroy_window(window_);
    }
    if (context_) {
        screen_destroy_context(context_);
    }
}

void QNXGraphicsWindow::setNumBuffers(int nbuffers) {
	nbuffers_ = nbuffers;
}

void QNXGraphicsWindow::setInterval(int interval) {
	interval_ = interval;
}

void QNXGraphicsWindow::create(int width, int height, const std::string &title, bool fullscreen, const std::string& pixelformat, const std::string &desiredDisplay)
{
    int usage = SCREEN_USAGE_OPENGL_ES2 | SCREEN_USAGE_OPENGL_ES3;
    int transp = SCREEN_TRANSPARENCY_NONE;
    int size[2];

    if (title.compare("QNX Vulkan") == 0) {
#if defined(SCREEN_USAGE_VULKAN)
        usage |= SCREEN_USAGE_VULKAN;
#endif /* SCREEN_USAGE_VULKAN */
        nbuffers_ = 0;
    }

    int numDisplays = 0;
    CHECK(screen_get_context_property_iv(context_, SCREEN_PROPERTY_DISPLAY_COUNT, &numDisplays));
    std::vector<screen_display_t> dpys(numDisplays);
    CHECK(screen_get_context_property_pv(context_, SCREEN_PROPERTY_DISPLAYS, (void**)dpys.data()));

    if(!desiredDisplay.empty()) {
        if(desiredDisplay.find_first_not_of("0123456789") == std::string::npos) {
            useProvidedDisplayId(atoi(desiredDisplay.c_str()), dpys);
        } else {
            useProvidedDisplayTarget(desiredDisplay, dpys);
        }
    }

    /* If pixel format is unknown, leave the default one, override only known values */
    if (pixelformat.compare("rgb565") == 0) {
        pixelformat_ = SCREEN_FORMAT_RGB565;
        pixelformatstr_ = pixelformat;
}
    if (pixelformat.compare("rgba8888") == 0) {
        pixelformat_ = SCREEN_FORMAT_RGBA8888;
        pixelformatstr_ = pixelformat;
    }
#if defined(SCREEN_FORMAT_BGRA8888)
    if (pixelformat.compare("bgra8888") == 0) {
        pixelformat_ = SCREEN_FORMAT_BGRA8888;
        pixelformatstr_ = pixelformat;
    }
#endif /* SCREEN_FORMAT_BGRA8888 */
#if defined(SCREEN_FORMAT_RGBA1010102)
    if (pixelformat.compare("rgba1010102") == 0) {
        pixelformat_ = SCREEN_FORMAT_RGBA1010102;
        pixelformatstr_ = pixelformat;
    }
#endif /* SCREEN_FORMAT_RGBA1010102 */
#if defined(SCREEN_FORMAT_BGRA1010102)
    if (pixelformat.compare("bgra1010102") == 0) {
        pixelformat_ = SCREEN_FORMAT_BGRA1010102;
        pixelformatstr_ = pixelformat;
    }
#endif /* SCREEN_FORMAT_BGRA1010102 */

    if ((width == 0) || (height == 0)) {
        CHECK(screen_get_window_property_iv(window_, SCREEN_PROPERTY_SIZE, size));
        width = size[0];
        height = size[1];
    } else {
        size[0] = width;
        size[1] = height;
    }

    CHECK(screen_set_window_property_iv(window_, SCREEN_PROPERTY_FORMAT, &pixelformat_));
    CHECK(screen_set_window_property_iv(window_, SCREEN_PROPERTY_USAGE, &usage));
    CHECK(screen_set_window_property_iv(window_, SCREEN_PROPERTY_SWAP_INTERVAL, &interval_));
    CHECK(screen_set_window_property_iv(window_, SCREEN_PROPERTY_TRANSPARENCY, &transp));
    NGLOG_INFO("using swap interval %s", std::to_string(interval_));
    CHECK(screen_set_window_property_iv(window_, SCREEN_PROPERTY_SIZE, size));
    if (nbuffers_ != 0) {
        CHECK(screen_create_window_buffers(window_, nbuffers_));
        NGLOG_INFO("allocating %s buffers", std::to_string(nbuffers_));
    }
    CHECK(screen_create_event(&screenEv_));

    width_ = width;
    height_ = height;
    fullscreen_ = fullscreen;
}

bool QNXGraphicsWindow::shouldClose()
{
    return shouldClose_;
}

void QNXGraphicsWindow::pollEvents()
{
    int rc = 0;
    int vis = 0;
    int val = -1;
    int pos[2] = { -1, -1 };
    while (!screen_get_event(context_, screenEv_, 0)) {
        rc = screen_get_event_property_iv(screenEv_, SCREEN_PROPERTY_TYPE, &val);

        if (rc || val == SCREEN_EVENT_NONE) {
            break;
        }
        switch (val) {
            case SCREEN_EVENT_CLOSE:
                shouldClose_ = true;
                break;
            case SCREEN_EVENT_PROPERTY:
                screen_get_event_property_iv(screenEv_, SCREEN_PROPERTY_NAME, &val);

                switch (val) {
                    case SCREEN_PROPERTY_VISIBLE:
                        screen_get_window_property_iv(window_, SCREEN_PROPERTY_VISIBLE, &vis);
                        shouldClose_ = !vis;
                        break;
                }
                break;
            case SCREEN_EVENT_POINTER:
                screen_get_event_property_iv(screenEv_, SCREEN_PROPERTY_BUTTONS, &val);
                if (val) {
                    screen_get_event_property_iv(screenEv_, SCREEN_PROPERTY_POSITION, pos);
                }
                break;
            case SCREEN_EVENT_KEYBOARD:
                screen_get_event_property_iv(screenEv_, SCREEN_PROPERTY_FLAGS, &val);
                if (val & KEY_DOWN) {
                    screen_get_event_property_iv(screenEv_, SCREEN_PROPERTY_SYM, &val);
                    switch (val) {
                        case KEYCODE_ESCAPE:
                            shouldClose_ = true;
                            break;
                        default:
                            break;
                    }
                }
                break;
        }
    }
}


void QNXGraphicsWindow::requestClose()
{
    shouldClose_ = true;
}

screen_window_t QNXGraphicsWindow::handle()
{
    return window_;
}

screen_context_t QNXGraphicsWindow::context()
{
    return context_;
}

screen_window_t QNXGraphicsWindow::window()
{
    return window_;
}

int QNXGraphicsWindow::width()
{
    return width_;
}

int QNXGraphicsWindow::height()
{
    return height_;
}

int QNXGraphicsWindow::pixelFormat()
{
    return pixelformat_;
}

std::string QNXGraphicsWindow::pixelFormatStr()
{
    return pixelformatstr_;
}

void QNXGraphicsWindow::useProvidedDisplayId(int displayId, const std::vector<screen_display_t> &displays)
{
    bool foundDisplay = false;
    int actual_id = 0; // Invalid

    for(unsigned int i = 0; i < displays.size(); i++) {
        CHECK(screen_get_display_property_iv(displays[i], SCREEN_PROPERTY_ID, &actual_id));
        if (displayId == actual_id) {
            CHECK(screen_set_window_property_pv(window_, SCREEN_PROPERTY_DISPLAY, (void **)&displays[i]));
            foundDisplay = true;
            break;
        }
    }

    if(!foundDisplay) {
        NGLOG_DEBUG("Found invalid display id: %s. Defaulting to first active display.", std::to_string(displayId));
    }
}

void QNXGraphicsWindow::useProvidedDisplayTarget(const std::string &displayName, const std::vector<screen_display_t> &displays)
{
    bool foundDisplay = false;

    for(int i = 0; i < displays.size(); i++) {
        char id_string[128];
        CHECK(screen_get_display_property_cv(displays[i], SCREEN_PROPERTY_ID_STRING, sizeof(id_string), id_string));
        if (id_string == displayName) {
            CHECK(screen_set_window_property_pv(window_, SCREEN_PROPERTY_DISPLAY, (void **)&displays[i]));
            foundDisplay = true;
            break;
        }
    }

        if(!foundDisplay) {
        NGLOG_DEBUG("Could not find display with id_string: %s. Defaulting to first active display.", displayName.c_str());
    }
}
