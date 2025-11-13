/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __CONTEXT_RENDER_API_ADAPTER_METAL_H__
#define __CONTEXT_RENDER_API_ADAPTER_METAL_H__

#include "graphics/graphicscontext.h"
#include <string>

class NGL_metal_adapter;

NGL_metal_adapter* GetNGLMetalAdapter(GraphicsContext* context, const std::string &device_id, bool screenshot_mode, bool macos_use_subpass);

#endif // __CONTEXT_RENDER_API_ADAPTER_METAL_H__


