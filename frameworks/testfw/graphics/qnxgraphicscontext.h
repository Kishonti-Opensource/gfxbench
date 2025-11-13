/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef QNXGRAPHICSCONTEXT_H
#define QNXGRAPHICSCONTEXT_H

#include <string>

#include <screen/screen.h>
#include "graphicscontext.h"

class QNXGraphicsContext : public GraphicsContext
{
public:
    QNXGraphicsContext(GraphicsContext::GraphicsType type = GraphicsContext::VULKAN, int major = 1, int minor = 0, int flags = 0)
        : type_(type)
        , major_(major)
        , minor_(minor)
        , flags_(flags)
    {
    }

    screen_context_t context()
    {
        return context_;
    }
    screen_window_t window()
    {
        return window_;
    }
    const char* format()
    {
        return format_;
    }
    int nbuffers()
    {
        return nbuffers_;
    }

    void init(screen_context_t context, screen_window_t window, const char* format, int nbuffers)
    {
        context_ = context;
        window_ = window;
        format_ = strdup(format);
        nbuffers_ = nbuffers;
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
    screen_context_t context_;
    screen_window_t window_;
    const char* format_;
    int nbuffers_;

    GraphicsType type_;
    int major_;
    int minor_;
    int flags_;
};

#endif /* QNXGRAPHICSCONTEXT_H */
