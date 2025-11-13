/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NULL_GRAPHICS_CONTEXT_H_
#define NULL_GRAPHICS_CONTEXT_H_

#include "graphicscontext.h"

class NullGraphicsContext : public GraphicsContext
{
public:
	NullGraphicsContext(GraphicsContext::GraphicsType type = GraphicsContext::NONE, int major = 1, int minor = 0, int flags = 0)
	: type_(type)
	, major_(major)
	, minor_(minor)
	, flags_(flags)
	{}
	bool isValid() { return true; }
	bool makeCurrent() { return true; }
	bool detachThread() { return true; }
	bool swapBuffers() { return true; }
	GraphicsType type() { return type_; }
	int versionMajor() { return major_; }
	int versionMinor() { return minor_; }
	bool hasFlag(int flag) { return (flags_ & flag) != 0; }

private:
	GraphicsType type_;
	int major_;
	int minor_;
	int flags_;
};


#endif  // NULL_GRAPHICS_CONTEXT_H_
