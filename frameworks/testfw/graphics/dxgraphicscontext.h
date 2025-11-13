/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DIRECTX_GRAPHICS_CONTEXT_H_
#define DIRECTX_GRAPHICS_CONTEXT_H_

#include "graphics/graphicscontext.h"
#include <d3d11_1.h>
#include <stdint.h>

class DXGraphicsContext : public GraphicsContext
{
public:
	virtual bool isValid() = 0;
	virtual bool makeCurrent() = 0;
	virtual bool detachThread() = 0;
	virtual bool swapBuffers() = 0;
	virtual GraphicsType type() { return DIRECTX; }
	virtual int versionMajor() = 0;
	virtual int versionMinor() = 0;
	virtual bool hasFlag(int flag) = 0;

	virtual ID3D11Device1* getD3D11Device() const = 0;
	virtual ID3D11DeviceContext1* getD3D11DeviceContext() const = 0;
	virtual ID3D11RenderTargetView* getD3D11RenderTargetView() const = 0;
	virtual ID3D11DepthStencilView* getD3D11DepthStencilView() const = 0;
	virtual D3D11_VIEWPORT* getDefaultViewport() = 0;
	virtual void resize(int32_t newx, int32_t newy) = 0;

	virtual void setFormat(int32_t redBits, int32_t greenBits, int32_t blueBits, int32_t alphaBits, int32_t depthBits, int32_t stencilBits, int32_t fsaaSamples) = 0;
};

#endif  // DIRECTX_GRAPHICS_CONTEXT_H_
