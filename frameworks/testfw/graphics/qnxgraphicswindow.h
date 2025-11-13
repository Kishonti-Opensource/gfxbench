/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef QNXGRAPHICSWINDOW_H_
#define QNXGRAPHICSWINDOW_H_

#include <screen/screen.h>
#include <sys/keycodes.h>
#include "graphics/graphicswindow.h"
#include "glformat.h"
#include <string>


class QNXGraphicsWindow : public GraphicsWindow
{
public:
    QNXGraphicsWindow();
    virtual ~QNXGraphicsWindow();
    void create(int width, int height, const std::string &title, bool fullscreen, const std::string& pixelformat, const std::string &displayType);
    void setNumBuffers(int);
    void setInterval(int);
    virtual bool shouldClose();
    virtual void pollEvents();
    virtual void requestClose();
    screen_window_t handle();
    virtual int width();
    virtual int height();
    virtual int pixelFormat();
    virtual std::string pixelFormatStr();

    screen_context_t context();
    screen_window_t window();
private:
    void useProvidedDisplayId(int displayId, const std::vector<screen_display_t> &displays);
    void useProvidedDisplayTarget(const std::string &displayName, const std::vector<screen_display_t> &displays);

    screen_context_t context_;
    screen_window_t window_;
    screen_event_t screenEv_;
    bool shouldClose_;
    int width_;
    int height_;
    bool fullscreen_;
    int pixelformat_;
    std::string pixelformatstr_;
    int nbuffers_;
    int interval_;
};

#endif  // QNXGRAPHICSWINDOW_H_
