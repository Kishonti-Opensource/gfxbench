/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GRAPHICSWINDOW_H_
#define GRAPHICSWINDOW_H_

#include "graphics/graphicscontext.h"

class GraphicsWindow
{
public:
    virtual ~GraphicsWindow() {}
    virtual int width() = 0;
    virtual int height() = 0;
    virtual bool shouldClose() = 0;
    virtual void pollEvents() = 0;
    virtual void requestClose() = 0;
};


#endif  // GRAPHICSWINDOW_H_
