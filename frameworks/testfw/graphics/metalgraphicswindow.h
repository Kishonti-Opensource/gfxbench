/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef METAL_GRAPHICS_WINDOW_H_
#define METAL_GRAPHICS_WINDOW_H_

#include "graphics/graphicswindow.h"
#include "graphics/graphicscontext.h"
#include "messagequeue.h"
#include "schemas/apidefinition.h"

class MTLGraphicsWindow : public GraphicsWindow
{
public:
    MTLGraphicsWindow(int width, int height, const std::vector<tfw::ApiDefinition>& api) : m_width(width), m_height(height), m_api(api) { }
    virtual ~MTLGraphicsWindow() { }
    
    virtual bool create() = 0;
    virtual GraphicsContext* getGraphicsContext() = 0;
    
    virtual int width() = 0;
    virtual int height() = 0;
    virtual bool shouldClose() = 0 ;
    virtual void pollEvents() = 0;
    virtual void requestClose() = 0;
	
	virtual tfw::MessageQueue* getMessageQueue() = 0;

protected:
    
    int m_width, m_height;
    std::vector<tfw::ApiDefinition> m_api;
};


MTLGraphicsWindow* CreateMTLGraphicsWindow(int width, int height, const std::vector<tfw::ApiDefinition>& api) ;



#endif // METAL_GRAPHICS_WINDOW_H_

