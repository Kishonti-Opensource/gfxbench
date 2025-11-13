/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef WGLGRAPHICSCONTEXT_H_
#define WGLGRAPHICSCONTEXT_H_

#include "graphics/graphicscontext.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <Windows.h>

class WGLGraphicsContext : public GraphicsContext {
public:
	WGLGraphicsContext();
	bool create(HWND wnd, int bpp);
	void destroy();

	// implementing GraphicsContext
	virtual bool isValid();
	virtual bool makeCurrent();
	virtual bool detachThread();
	virtual bool swapBuffers();
	virtual GraphicsType type();
	virtual int versionMajor();
	virtual int versionMinor();
	virtual bool hasFlag(int flag);

private:
	HDC hDC_;
	HGLRC hGLRC_;
};


#endif // WGLGRAPHICSCONTEXT_H_