/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "graphics/dxgraphicscontext.h"
#include "graphics/glformat.h"
#include <string>


class DxBareGraphicsContext : public GraphicsContext
{
public:
	DxBareGraphicsContext(const HWND& hWnd, int major, int minor, GraphicsType type_in = GraphicsContext::DIRECTX) : hwnd_(hWnd), major_(major), minor_(minor), type_(type_in){};
	~DxBareGraphicsContext() {};

	virtual bool makeCurrent() override {return true;}
	virtual bool detachThread() override{ return true; }
	virtual bool swapBuffers() override { return true; }
	virtual int versionMinor() { return minor_; }
	virtual int versionMajor() { return major_; }

	virtual GraphicsType type() { return type_;  }
	virtual bool hasFlag(int flag) { return true; };

	virtual bool isValid() { return true; };

	HWND hwnd()
	{
		return hwnd_;
	}

private:
	HWND hwnd_;
	int major_;
	int minor_;
	GraphicsType type_;
};

class DxWin32GraphicsContext : public DXGraphicsContext
{
public:

	DxWin32GraphicsContext(const HWND& hWnd);
	~DxWin32GraphicsContext();

	void init(std::string selected_device, bool vsync);
	virtual void initializeBuffers(ID3D11Texture2D* renderTarget);
	virtual bool makeCurrent() override;
	virtual bool detachThread() override;
	virtual bool swapBuffers() override;
	virtual int versionMinor() { return m_minor; }
	virtual int versionMajor() { return m_major; }
	virtual bool hasFlag(int Flag) { if (Flag == FLAG_DX_DEBUG_CONTEXT) return m_debug; return false; }

	virtual bool isValid() { return true; };
	virtual ID3D11Device1* getD3D11Device() const { return m_d3dDevice; };
	virtual ID3D11DeviceContext1* getD3D11DeviceContext() const { return m_d3dContext; };
	virtual ID3D11RenderTargetView* getD3D11RenderTargetView() const { return m_d3dRenderTargetView; };
	virtual ID3D11DepthStencilView* getD3D11DepthStencilView() const { return m_d3dDepthStencilView; };
	virtual D3D11_VIEWPORT* getDefaultViewport() { return &m_defaultViewport; };
	virtual void resize(int32_t newx, int32_t newy);

	virtual void setFormat(int32_t redBits, int32_t greenBits, int32_t blueBits, int32_t alphaBits, int32_t depthBits, int32_t stencilBits, int32_t fsaaSamples)
	{
		m_format = tfw::GLFormat(redBits, greenBits, blueBits, alphaBits, depthBits, stencilBits, fsaaSamples, false);
	}

private:
	HWND m_hwnd;

	bool vsync_;

	IDXGISwapChain1*        m_swapChain;
	ID3D11Device1*          m_d3dDevice;
	ID3D11DeviceContext1*   m_d3dContext;
	ID3D11Texture2D*		m_renderTargetTexture;
	ID3D11RenderTargetView* m_d3dRenderTargetView;
	ID3D11Texture2D*		m_depthStencilTexture;
	ID3D11DepthStencilView* m_d3dDepthStencilView;
	D3D11_VIEWPORT          m_screenViewport;
	D3D11_VIEWPORT          m_defaultViewport;

	D3D_DRIVER_TYPE m_driverType;
	bool					m_debug;
	int						m_major;
	int						m_minor;

	tfw::GLFormat m_format;
	ID3D11Texture2D* getSwapChainTexture(const HWND& hWnd);
	void createSwapChain(const HWND& hWnd);

private:
#ifdef _DEBUG
	ID3D11Debug *m_d3dDebug;
	ID3D11InfoQueue *m_d3dInfoQueue;
#endif
};
