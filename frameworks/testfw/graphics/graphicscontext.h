/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GRAPHICS_CONTEXT_H_
#define GRAPHICS_CONTEXT_H_

class GraphicsContext
{
public:
	enum GraphicsType
	{
		NONE 		= 0x00,
		OPENGL 		= 0x01,
		GLES 		= 0x02,
		DIRECTX		= 0x04,
		METAL 		= 0x08,
		OVR_EGL 	= 0x10,
		VULKAN  	= 0x20,
		DIRECTX12	= 0x40
	};
	static const int FLAG_GL_CORE_PROFILE = 1<<0;
	static const int FLAG_GL_COMPATIBILITY_PROFILE = 1<<1;
	static const int FLAG_GL_DEBUG_CONTEXT = 1<<2;
	static const int FLAG_GL_FORWARD_COMPATIBLE = 1<<3;
	static const int FLAG_GL_ROBUSTNESS = 1<<4;
	static const int FLAG_DX_DEBUG_CONTEXT = 1<<5;

	virtual ~GraphicsContext() {};
	virtual bool isValid() { return false; }
	virtual bool makeCurrent() = 0;
	virtual bool detachThread() = 0;
	virtual bool swapBuffers() = 0;
	virtual GraphicsType type() = 0;
	virtual int versionMajor() = 0;
	virtual int versionMinor() = 0;
	virtual bool hasFlag(int flag) = 0;
};

#endif  // GRAPHICS_CONTEXT_H_
