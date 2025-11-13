/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef VULKAN_GRAPHICS_CONTEXT_H_
#define VULKAN_GRAPHICS_CONTEXT_H_

//#include "glformat.h"
#include "graphicscontext.h"
#include <stdint.h>
#include <vector>


class VulkanGraphicsContext : public GraphicsContext
{
public:
	VulkanGraphicsContext();
    virtual ~VulkanGraphicsContext();
    
	bool init();

    void destroy();
    bool isValid();

    // from GraphicsContext
    virtual bool makeCurrent();
    virtual bool detachThread();
    virtual bool swapBuffers();
    virtual GraphicsType type();
	virtual int versionMajor();
    virtual int versionMinor();
	virtual bool hasFlag(int flag);


protected:

};

#endif  // VULKAN_GRAPHICS_CONTEXT_H_
