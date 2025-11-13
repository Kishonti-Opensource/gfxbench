/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NGL_GL_ADAPTER_INTERFACE_H
#define NGL_GL_ADAPTER_INTERFACE_H

#include <stdint.h>
#include <vector>

struct NGL_gl_adapter_interface
{
	virtual ~NGL_gl_adapter_interface() {}

	virtual void GetDefaultFramebufferTextures(std::vector<uint32_t> &ids) = 0;
	virtual uint32_t GetDefaultFramebufferTextureIndex() = 0;
};

#endif
