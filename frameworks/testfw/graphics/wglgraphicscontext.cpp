/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "graphics/wglgraphicscontext.h"
#include "ng/log.h"
#include "ng/require.h"
#ifdef HAVE_EPOXY
#include "epoxy/wgl.h"
#endif

namespace {

// Create a string with last error message
std::string GetLastErrorStdStr()
{
	DWORD error = GetLastError();
	if (error)
	{
		LPVOID lpMsgBuf;
		DWORD bufLen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);
		if (bufLen)
		{
			LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
			std::string result(lpMsgStr, lpMsgStr + bufLen);

			LocalFree(lpMsgBuf);

			return result;
		}
	}
	return std::string();
}

}

WGLGraphicsContext::WGLGraphicsContext()
	: hDC_(0)
	, hGLRC_(0)
{
}

bool WGLGraphicsContext::create(HWND wnd, int bpp)
{
	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		(BYTE)bpp,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	int pixelFormat;

	hDC_ = GetDC(wnd);
	require(hDC_ != NULL);

	if (!(pixelFormat = ChoosePixelFormat(hDC_, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		NGLOG_ERROR(GetLastErrorStdStr());
		return false;
	}

	BOOL b = SetPixelFormat(hDC_, pixelFormat, &pfd);
	if (!b)
	{
		NGLOG_ERROR(GetLastErrorStdStr());
		return false;
	}
	hGLRC_ = wglCreateContext(hDC_);
	require(makeCurrent());
	return hGLRC_ != 0;
}


void WGLGraphicsContext::destroy()
{
	if (isValid())
	{
		wglDeleteContext(hGLRC_);
		hGLRC_ = 0;
		hDC_ = 0;
	}
}

bool WGLGraphicsContext::isValid()
{
	return hDC_ != 0 && hGLRC_ != 0;
}
	
bool WGLGraphicsContext::makeCurrent()
{
	return TRUE == wglMakeCurrent(hDC_, hGLRC_);
}

bool WGLGraphicsContext::detachThread()
{
	return TRUE == wglMakeCurrent(NULL, NULL);
}

bool WGLGraphicsContext::swapBuffers()
{
	BOOL b = SwapBuffers(hDC_);
	if (!b)
		NGLOG_ERROR(GetLastErrorStdStr());
	return b == TRUE;
}
GraphicsContext::GraphicsType WGLGraphicsContext::type()
{
	return GraphicsContext::OPENGL;
}
int WGLGraphicsContext::versionMajor()
{
	int version = 11;
#ifdef HAVE_EPOXY
	version = epoxy_gl_version();
#endif
	return version / 10;
}
int WGLGraphicsContext::versionMinor()
{
	int version = 11;
#ifdef HAVE_EPOXY
	version = epoxy_gl_version();
#endif
	return version % 10;
}
bool WGLGraphicsContext::hasFlag(int flag)
{
	return false;
}
