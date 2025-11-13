/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "graphics/dxwin32graphicscontext.h"
#include <cassert>
#include "dxgi1_2.h"

#pragma comment(lib, "dxgi.lib")

#if _DEBUG
#define NGL_D3D11_ENABLE_DEBUG_LAYER
#endif

#define DX_THROW_IF_FAILED(expression)	{ \
	HRESULT __hr = (expression);	\
	if (FAILED(__hr)) {	\
	switch (__hr) {	\
	case DXGI_ERROR_DEVICE_REMOVED: {	\
			assert(0); \
		} \
	default: \
		assert(0); \
}}}\


struct DXGIFormatInfo
{
	int red;
	int green;
	int blue;
	int alpha;
	DXGI_FORMAT format;
};

//DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM
DXGIFormatInfo formats[] = {
		{ 8, 8, 8, 8, DXGI_FORMAT_R8G8B8A8_UNORM },
//		{ 5, 6, 5, 0, DXGI_FORMAT_B5G6R5_UNORM },
		{ 0, 0, 0, 0, DXGI_FORMAT_UNKNOWN }
};


struct DXGIDepthStencilInfo
{
	int depth;
	int stencil;
	DXGI_FORMAT format;
};


DXGIDepthStencilInfo depth_formats[] = {
		{ 16, 0, DXGI_FORMAT_D16_UNORM },
		{ 24, 0, DXGI_FORMAT_D24_UNORM_S8_UINT },
		{ 0, 0, DXGI_FORMAT_UNKNOWN }
};


void DxWin32GraphicsContext::initializeBuffers(ID3D11Texture2D* renderTarget)
{
	DXGI_FORMAT depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	for (size_t i = 0; depth_formats[i].format != DXGI_FORMAT_UNKNOWN; i++)
	{
		int depth = m_format.depth;
		int stencil = m_format.stencil;

		if (depth_formats[i].depth == depth && depth_formats[i].stencil == stencil)
		{
			depthFormat = depth_formats[i].format;
			break;;
		}
	}

	// Invalidate previous viewport.
	ZeroMemory(&m_defaultViewport, sizeof(D3D11_VIEWPORT));

	D3D11_TEXTURE2D_DESC t2dd;
	renderTarget->GetDesc(&t2dd);

	m_renderTargetTexture = renderTarget;

	// Create a descriptor for the RenderTargetView.
	CD3D11_RENDER_TARGET_VIEW_DESC rtvd(D3D11_RTV_DIMENSION_TEXTURE2D);

	// Create a view interface on the rendertarget to use on bind.
	DX_THROW_IF_FAILED(m_d3dDevice->CreateRenderTargetView(
		m_renderTargetTexture,
		&rtvd,
		&m_d3dRenderTargetView));

	m_renderTargetTexture->Release();


	if (m_format.depth > 0)
	{
		// Allocate a 2-D surface as the depth/stencil buffer.
		CD3D11_TEXTURE2D_DESC dsd(depthFormat, t2dd.Width, t2dd.Height, 1, 1, D3D11_BIND_DEPTH_STENCIL);
		DX_THROW_IF_FAILED(m_d3dDevice->CreateTexture2D(&dsd, NULL, &m_depthStencilTexture));

		// Create a DepthStencil view on this surface to use on bind.
		CD3D11_DEPTH_STENCIL_VIEW_DESC dsvd(D3D11_DSV_DIMENSION_TEXTURE2D);
		DX_THROW_IF_FAILED(m_d3dDevice->CreateDepthStencilView(
			m_depthStencilTexture,
			&dsvd,
			&m_d3dDepthStencilView));

		m_depthStencilTexture->Release();
	}

	// Fill viewport information only after everything succeeded.
	m_defaultViewport.TopLeftX = 0;
	m_defaultViewport.TopLeftY = 0;
	m_defaultViewport.Width = (float)t2dd.Width;
	m_defaultViewport.Height = (float)t2dd.Height;
	m_defaultViewport.MinDepth = 0.0f;
	m_defaultViewport.MaxDepth = 1.0f;
	m_screenViewport = m_defaultViewport;
}


DxWin32GraphicsContext::DxWin32GraphicsContext(const HWND& hWnd): m_hwnd(hWnd),
	m_debug(false)
{
	m_depthStencilTexture = nullptr;
	m_d3dDepthStencilView = nullptr;
}


IDXGIAdapter1 *DX11GetAdapter(IDXGIFactory2 *dxgi_factory, const std::string &selected_device, DXGI_ADAPTER_DESC1 &adapter_desc)
{
	size_t delimiter_pos = selected_device.find_last_of(";");
	if (delimiter_pos == std::string::npos || selected_device.size() == 0 || delimiter_pos >= selected_device.size() - 1)
	{
		return nullptr;
	}

	std::string selected_luid = selected_device.substr(delimiter_pos + 1);
	uint64_t luid = 0;

	IDXGIAdapter1 *adapter = nullptr;
	UINT i = 0;
	while (true)
	{
		if (dxgi_factory->EnumAdapters1(i, &adapter) == S_OK)
		{
			DX_THROW_IF_FAILED(adapter->GetDesc1(&adapter_desc));

			assert(sizeof(luid) == sizeof(adapter_desc.AdapterLuid));
			memcpy(&luid, &adapter_desc.AdapterLuid, sizeof(luid));
			std::string adapter_luid = std::to_string(luid);

			if (adapter_luid == selected_luid)
			{
				return adapter;
			}
		}
		else
		{
			return nullptr;
		}

		adapter->Release();
		i++;
	}

	assert(false);
	return nullptr;
}


void DxWin32GraphicsContext::init(std::string selected_device, bool vsync)
{
	vsync_ = vsync;

	assert(m_hwnd);
	m_driverType = D3D_DRIVER_TYPE_UNKNOWN;

	ZeroMemory(&m_defaultViewport, sizeof(D3D11_VIEWPORT));

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef NGL_D3D11_ENABLE_DEBUG_LAYER
	m_debug = true;
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// This array defines the set of DirectX hardware feature levels this app will support.
	// Note the ordering should be preserved.
	// Don't forget to declare your application's minimum required feature level in its
	// description.  All applications are assumed to support 9.1 unless otherwise stated.
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	D3D_FEATURE_LEVEL featureLevel;

	IDXGIFactory2 *dxgi_factory;
	IDXGIAdapter1 *adapter;
	DXGI_ADAPTER_DESC1 adapter_desc;
	DX_THROW_IF_FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory)));

	adapter = DX11GetAdapter(dxgi_factory, selected_device, adapter_desc);
	if (adapter == nullptr)
	{
		m_driverType = D3D_DRIVER_TYPE_HARDWARE;
	}

	// Create the DX11 API device object, and get a corresponding context.
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	HRESULT create_device_result = D3D11CreateDevice(
		adapter,                    // specify null to use the default adapter
		m_driverType,
		NULL,                    // leave as nullptr unless software device
		creationFlags,              // optionally set debug and Direct2D compatibility flags
		featureLevels,              // list of feature levels this app can support
		ARRAYSIZE(featureLevels),   // number of entries in above list
		D3D11_SDK_VERSION,          // always set this to D3D11_SDK_VERSION
		&device,                    // returns the Direct3D device created
		&featureLevel,            // returns feature level of device created
		&context                    // returns the device immediate context
		);

	// if fails, check if graphics debugging tools is installed
	// or comment out NGL_D3D11_ENABLE_DEBUG_LAYER at the top
	DX_THROW_IF_FAILED(create_device_result);

	switch (featureLevel)
	{
	case D3D_FEATURE_LEVEL_11_1:
		m_major = 11; m_minor = 1; break;
	case D3D_FEATURE_LEVEL_11_0:
		m_major = 11; m_minor = 0; break;
	case D3D_FEATURE_LEVEL_10_1:
		m_major = 10; m_minor = 1; break;
	case D3D_FEATURE_LEVEL_10_0:
		m_major = 10; m_minor = 0; break;
	case D3D_FEATURE_LEVEL_9_3:
		m_major = 9; m_minor = 3; break;
	case D3D_FEATURE_LEVEL_9_2:
		m_major = 9; m_minor = 2; break;
	case D3D_FEATURE_LEVEL_9_1:
		m_major = 9; m_minor = 1; break;
	default:
		m_major = 0; m_minor = 0; break;
	}

	device->QueryInterface(IID_ID3D11Device1, (void**)&m_d3dDevice);
	context->QueryInterface(IID_ID3D11DeviceContext1, (void**)&m_d3dContext);
	device->Release();
	context->Release();


#ifdef NGL_D3D11_ENABLE_DEBUG_LAYER
	m_d3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&m_d3dDebug));
	m_d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&m_d3dInfoQueue);
	m_d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	m_d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);


	/*!
	The 'internal' reference counts might still be > 0 since Direct3D objects actually use 'lazy destruction'
	and a few objects are alive as long as the device is alive for internal defaults.
	*/
	D3D11_MESSAGE_ID hide[] =
	{
		D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
		//D3D11_MESSAGE_ID_LIVE_INPUTLAYOUT,
		//D3D11_MESSAGE_ID_LIVE_DEPTHSTENCILSTATE,
	};
	D3D11_INFO_QUEUE_FILTER filter;
	memset(&filter, 0, sizeof(filter));
	filter.DenyList.NumIDs = _countof(hide);
	filter.DenyList.pIDList = hide;
	m_d3dInfoQueue->AddStorageFilterEntries(&filter);

#endif

	ID3D11Texture2D* renderTargetTexture = getSwapChainTexture(m_hwnd);
	initializeBuffers(renderTargetTexture);
}

DxWin32GraphicsContext::~DxWin32GraphicsContext()
{
	// Release all outstanding references to the swap chain's buffers.
	m_d3dContext->OMSetRenderTargets(0, 0, 0);
	m_d3dRenderTargetView->Release();
	if (m_d3dDepthStencilView)
	{
		m_d3dDepthStencilView->Release();
	}
	m_swapChain->Release();
#ifdef NGL_D3D11_ENABLE_DEBUG_LAYER
	m_d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
	m_d3dInfoQueue->Release();
	m_d3dDebug->Release();
#endif
	//m_d3dDevice->Release();
}


bool DxWin32GraphicsContext::makeCurrent()
{
	return true;
}


bool DxWin32GraphicsContext::detachThread()
{
	return true;
}


bool DxWin32GraphicsContext::swapBuffers()
{
	if (m_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		return true;
	}

    DXGI_PRESENT_PARAMETERS parameters = {0};
    parameters.DirtyRectsCount = 0;
    parameters.pDirtyRects = nullptr;
    parameters.pScrollRect = nullptr;
    parameters.pScrollOffset = nullptr;

	DX_THROW_IF_FAILED(m_swapChain->Present1(vsync_, 0, &parameters));

	m_d3dContext->OMSetRenderTargets(1, &m_d3dRenderTargetView, m_d3dDepthStencilView);
	m_d3dContext->RSSetViewports(1, &m_screenViewport);
	return true;
}


ID3D11Texture2D* DxWin32GraphicsContext::getSwapChainTexture(const HWND& hWnd)
{
	ID3D11Texture2D* texture = NULL;

	if( m_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		D3D11_TEXTURE2D_DESC backBufferDesc;
		backBufferDesc.Width = 1920;
		backBufferDesc.Height = 1080;
		backBufferDesc.MipLevels = 1;
		backBufferDesc.ArraySize = 1;
		backBufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		backBufferDesc.SampleDesc.Count = 1;
		backBufferDesc.SampleDesc.Quality = 0;
		backBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		backBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
		backBufferDesc.CPUAccessFlags = 0;
		backBufferDesc.MiscFlags = 0;

		DX_THROW_IF_FAILED(m_d3dDevice->CreateTexture2D(&backBufferDesc, NULL, &texture));
		return texture;
	}
	else
	{
		createSwapChain(hWnd);
		DX_THROW_IF_FAILED(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&texture));
		return texture;
	}
}


void DxWin32GraphicsContext::createSwapChain(const HWND& hWnd)
{
	DXGI_FORMAT colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	for (size_t i = 0; formats[i].format != DXGI_FORMAT_UNKNOWN; i++)
	{
		int red = m_format.red;
		int green = m_format.green;
		int blue = m_format.blue;

		if (formats[i].red == red &&
			formats[i].green == green &&
			formats[i].blue == blue)
		{
			colorFormat = formats[i].format;
			break;
		}
	}

	IDXGIDevice1* dxgiDevice;
#pragma comment ( lib, "dxguid.lib")
#pragma comment( lib, "d3d11.lib")
	DX_THROW_IF_FAILED(m_d3dDevice->QueryInterface(IID_IDXGIDevice1,(void**)&dxgiDevice));

    IDXGIAdapter* dxgiAdapter;
    DX_THROW_IF_FAILED(
        dxgiDevice->GetAdapter(&dxgiAdapter));
	////UINT size = sizeof(m_driverType);
	////if (dxgiAdapter->GetPrivateData(GUID_DeviceType, &size, &m_driverType) != S_OK)
	////{
	////	m_driverType = D3D_DRIVER_TYPE_HARDWARE;
	////}

	// Create a descriptor for the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
	swapChainDesc.Format = colorFormat;           // this is the most common swapchain format
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = vsync_ ? DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL : DXGI_SWAP_EFFECT_SEQUENTIAL; //DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	// Once the desired swap chain description is configured, it must be created on the same adapter as our D3D Device
	// First, retrieve the underlying DXGI Device from the D3D Device (done)
	// Identify the physical adapter (GPU or card) this device is running on. (done)
	// And obtain the factory object that created it.
	IDXGIFactory2* dxgiFactory;
	DX_THROW_IF_FAILED(
		dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory));

    swapChainDesc.Width = 0;                                     // use automatic sizing
	swapChainDesc.Height = 0;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;


    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {0};
	fullscreenDesc.RefreshRate.Numerator = 0; //use 60 for V-Sync
	fullscreenDesc.RefreshRate.Denominator = 1;
	fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    fullscreenDesc.Windowed = FALSE;

	m_swapChain = nullptr;
	// Create a swap chain for this window from the DXGI factory.
	DX_THROW_IF_FAILED(dxgiFactory->CreateSwapChainForHwnd(
	            m_d3dDevice,
				hWnd,
				&swapChainDesc,
                nullptr,//&fullscreenDesc,
                nullptr,            // allow on all displays
				&m_swapChain
				));

	// Ensure that DXGI does not queue more than one frame at a time. This both reduces
	// latency and ensures that the application will only render after each VSync, minimizing
	// power consumption.
	DX_THROW_IF_FAILED(dxgiDevice->SetMaximumFrameLatency(1));
	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();
}

/*!
After resize the m_d3dRenderTargetView and m_d3dDepthStencilView will be invalidated
*/
void DxWin32GraphicsContext::resize(int32_t newx, int32_t newy)
{
	if (newx < 1 || newy < 1)
	{
		return;
	}

	m_d3dContext->OMSetRenderTargets(0, 0, 0);
	m_d3dRenderTargetView->Release();
	m_d3dDepthStencilView->Release();

	HRESULT hr;
	/*!
	Set this value to DXGI_FORMAT_UNKNOWN to preserve the existing format of the back buffer.
	*/
	hr = m_swapChain->ResizeBuffers(2, newx, newy, DXGI_FORMAT_UNKNOWN, 0);
	DX_THROW_IF_FAILED(hr);

	ID3D11Texture2D* pBuffer;
	D3D11_TEXTURE2D_DESC backBufferDesc;
	backBufferDesc.Width = newx;
	backBufferDesc.Height = newy;
	backBufferDesc.MipLevels = 1;
	backBufferDesc.ArraySize = 1;
	backBufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	backBufferDesc.SampleDesc.Count = 1;
	backBufferDesc.SampleDesc.Quality = 0;
	backBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	backBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	backBufferDesc.CPUAccessFlags = 0;
	backBufferDesc.MiscFlags = 0;

	m_d3dDevice->CreateTexture2D(&backBufferDesc, NULL, &pBuffer);

	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);
	hr = getD3D11Device()->CreateRenderTargetView(pBuffer, NULL, &m_d3dRenderTargetView);
	pBuffer->Release();

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = newx;
	depthStencilDesc.Height = newy;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	ID3D11Texture2D* depthStencilBuffer;
	m_d3dDevice->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	m_d3dDevice->CreateDepthStencilView(depthStencilBuffer, NULL, &m_d3dDepthStencilView);


	getD3D11DeviceContext()->OMSetRenderTargets(1, &m_d3dRenderTargetView, m_d3dDepthStencilView);
	depthStencilBuffer->Release();

	D3D11_VIEWPORT vp;
	vp.Width = (float) newx;
	vp.Height = (float) newy;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	getD3D11DeviceContext()->RSSetViewports(1, &vp);

	m_screenViewport = m_defaultViewport = vp;
}
