/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "pch.h"
#include "dxgraphicscontext.h"
#include "DirectXHelper.h"
#include <windows.ui.xaml.media.dxinterop.h>

using namespace D2D1;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Platform;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP)
using namespace WindowsPhoneInterop;
#else
using namespace WindowsRTInterop;
#endif

// Constructor for DeviceResources.
DxGraphicsContext::DxGraphicsContext() :
m_screenViewport(),
m_d3dFeatureLevel(D3D_FEATURE_LEVEL_9_1),
m_d3dRenderTargetSize(),
m_outputSize(),
m_logicalSize(),
m_compositionScaleX(1.0f),
m_compositionScaleY(1.0f),
m_deviceNotify(nullptr)
{
	CreateDeviceIndependentResources();
	CreateDeviceResources();

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP)
	m_displayRotation = DXGI_MODE_ROTATION_ROTATE270;
#else
	m_displayRotation = DXGI_MODE_ROTATION_IDENTITY;
#endif
}

// Configures resources that don't depend on the Direct3D device.
void DxGraphicsContext::CreateDeviceIndependentResources()
{
	// Initialize Direct2D resources.
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
	// If the project is in a debug build, enable Direct2D debugging via SDK Layers.
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
}

// Configures the Direct3D device, and stores handles to it and the device context.
void DxGraphicsContext::CreateDeviceResources()
{
	// This flag adds support for surfaces with a different color channel ordering
	// than the API default. It is required for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	if (DX::SdkLayersAvailable())
	{
		// If the project is in a debug build, enable debugging via SDK Layers with this flag.
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
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

	// Create the Direct3D 11 API device object and a corresponding context.
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;

	HRESULT hr = D3D11CreateDevice(
		nullptr,					// Specify nullptr to use the default adapter.
		D3D_DRIVER_TYPE_HARDWARE,	// Create a device using the hardware graphics driver.
		0,							// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
		creationFlags,				// Set debug and Direct2D compatibility flags.
		featureLevels,				// List of feature levels this app can support.
		ARRAYSIZE(featureLevels),	// Size of the list above.
		D3D11_SDK_VERSION,			// Always set this to D3D11_SDK_VERSION for Windows Store apps.
		&device,					// Returns the Direct3D device created.
		&m_d3dFeatureLevel,			// Returns feature level of device created.
		&context					// Returns the device immediate context.
		);

	if (FAILED(hr))
	{
		// If the initialization fails, fall back to the WARP device.
		// For more information on WARP, see: 
		// http://go.microsoft.com/fwlink/?LinkId=286690
		DX::ThrowIfFailed(
			D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
			0,
			creationFlags,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&device,
			&m_d3dFeatureLevel,
			&context
			)
			);
	}

	// Store pointers to the Direct3D 11.1 API device and immediate context.
	DX::ThrowIfFailed(
		device.As(&m_d3dDevice)
		);

	DX::ThrowIfFailed(
		context.As(&m_d3dContext)
		);

	// Create the Direct2D device object and a corresponding context.
	ComPtr<IDXGIDevice3> dxgiDevice;
	DX::ThrowIfFailed(
		m_d3dDevice.As(&dxgiDevice)
		);
}

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

// These resources need to be recreated every time the window size is changed.
void DxGraphicsContext::CreateWindowSizeDependentResources()
{
	DXGI_FORMAT depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
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

	for (size_t i = 0; depth_formats[i].format != DXGI_FORMAT_UNKNOWN; i++)
	{
		int depth = m_format.depth;
		int stencil = m_format.stencil;

		if (depth_formats[i].depth == depth &&
			depth_formats[i].stencil == stencil)
		{
			depthFormat = depth_formats[i].format;
			break;;
		}
	}

	// Clear the previous window size specific context.
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
	m_d3dRenderTargetView = nullptr;
	m_d3dDepthStencilView = nullptr;
	m_d3dContext->Flush();

	// Calculate the necessary swap chain and render target size in pixels.
	m_outputSize.Width = m_logicalSize.Width*m_compositionScaleX;
	m_outputSize.Height = m_logicalSize.Height*m_compositionScaleY;

	// Prevent zero size DirectX content from being created.
	m_outputSize.Width = max(m_outputSize.Width, 1);
	m_outputSize.Height = max(m_outputSize.Height, 1);

	bool swapDimensions = m_displayRotation == DXGI_MODE_ROTATION_ROTATE90 || m_displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_d3dRenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
	m_d3dRenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;


	if (m_swapChain != nullptr)
	{
		// If the swap chain already exists, resize it.
		HRESULT hr = m_swapChain->ResizeBuffers(
			2, // Double-buffered swap chain.
			static_cast<UINT>(m_d3dRenderTargetSize.Width),
			static_cast<UINT>(m_d3dRenderTargetSize.Height),
			colorFormat,
			0
			);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			HandleDeviceLost();

			// Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
			// and correctly set up the new device.
			return;
		}
		else
		{
			DX::ThrowIfFailed(hr);
		}
	}
	else
	{
		// Otherwise, create a new one using the same adapter as the existing Direct3D device.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

		swapChainDesc.Width = static_cast<UINT>(m_d3dRenderTargetSize.Width); // Match the size of the window.
		swapChainDesc.Height = static_cast<UINT>(m_d3dRenderTargetSize.Height);
		swapChainDesc.Format = colorFormat; // This is the most common swap chain format.
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2; // Use double-buffering to minimize latency.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		// This sequence obtains the DXGI factory that was used to create the Direct3D device above.
		ComPtr<IDXGIDevice3> dxgiDevice;
		DX::ThrowIfFailed(
			m_d3dDevice.As(&dxgiDevice)
			);

		ComPtr<IDXGIAdapter> dxgiAdapter;
		DX::ThrowIfFailed(
			dxgiDevice->GetAdapter(&dxgiAdapter)
			);

		ComPtr<IDXGIFactory2> dxgiFactory;
		DX::ThrowIfFailed(
			dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
			);

		// When using XAML interop, the swap chain must be created for composition.
		DX::ThrowIfFailed(
			dxgiFactory->CreateSwapChainForComposition(
			m_d3dDevice.Get(),
			&swapChainDesc,
			nullptr,
			&m_swapChain
			)
			);

		// Associate swap chain with SwapChainPanel
		// UI changes will need to be dispatched back to the UI thread
		m_swapChainPanel->Dispatcher->RunAsync(CoreDispatcherPriority::High, ref new DispatchedHandler([=]()
		{
			// Get backing native interface for SwapChainPanel
			ComPtr<ISwapChainPanelNative> panelNative;
			DX::ThrowIfFailed(
				reinterpret_cast<IUnknown*>(m_swapChainPanel)->QueryInterface(IID_PPV_ARGS(&panelNative))
				);

			DX::ThrowIfFailed(
				panelNative->SetSwapChain(m_swapChain.Get())
				);
		}, CallbackContext::Any));

		// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
		// ensures that the application will only render after each VSync, minimizing power consumption.
		DX::ThrowIfFailed(
			dxgiDevice->SetMaximumFrameLatency(1)
			);
	}


	DX::ThrowIfFailed(
		m_swapChain->SetRotation(m_displayRotation)
		);

	// Setup inverse scale on the swap chain
	DXGI_MATRIX_3X2_F inverseScale = { 0 };
	inverseScale._11 = 1.0f / m_compositionScaleX;
	inverseScale._22 = 1.0f / m_compositionScaleY;
	DX::ThrowIfFailed(
		m_swapChain.As<IDXGISwapChain2>(&m_swapChain2)
		);

	DX::ThrowIfFailed(
		m_swapChain2->SetMatrixTransform(&inverseScale)
		);


	// Create a render target view of the swap chain back buffer.
	ComPtr<ID3D11Texture2D> backBuffer;
	DX::ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
		);

	DX::ThrowIfFailed(
		m_d3dDevice->CreateRenderTargetView(
		backBuffer.Get(),
		nullptr,
		&m_d3dRenderTargetView
		)
		);


	// Create a depth stencil view for use with 3D rendering if needed.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		depthFormat,
		static_cast<UINT>(m_d3dRenderTargetSize.Width),
		static_cast<UINT>(m_d3dRenderTargetSize.Height),
		1, // This depth stencil view has only one texture.
		1, // Use a single mipmap level.
		D3D11_BIND_DEPTH_STENCIL
		);

	ComPtr<ID3D11Texture2D> depthStencil;
	DX::ThrowIfFailed(
		m_d3dDevice->CreateTexture2D(
		&depthStencilDesc,
		nullptr,
		&depthStencil
		)
		);

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	DX::ThrowIfFailed(
		m_d3dDevice->CreateDepthStencilView(
		depthStencil.Get(),
		&depthStencilViewDesc,
		&m_d3dDepthStencilView
		)
		);

	ComPtr<IDXGISurface2> dxgiBackBuffer;
	DX::ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer))
		);

	//--- adapters---

	//IDXGIFactory1 *dxgiFactory;
	//if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)&dxgiFactory)))
	//{
	//}

	//UINT i = 0;
	//IDXGIAdapter * pAdapter;
	//std::vector <IDXGIAdapter*> vAdapters;
	//while (dxgiFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	//{
	//	vAdapters.push_back(pAdapter);
	//	DXGI_ADAPTER_DESC adapterDescription;
	//	pAdapter->GetDesc(&adapterDescription);

	//	std::wstring  wdesc = adapterDescription.Description;
	//	std::string desc;
	//	desc.assign(wdesc.begin(), wdesc.end());  // convert wstring to string
	//	++i;
	//}

	//ValidateDevice();
	//----------------
}

// This method is called when the XAML control is created (or re-created).
void DxGraphicsContext::SetSwapChainPanel(SwapChainPanel^ panel)
{
	m_swapChainPanel = panel;
	m_compositionScaleX = panel->CompositionScaleX;
	m_compositionScaleY = panel->CompositionScaleY;
}

// This method is called in the event handler for the SizeChanged event.
void DxGraphicsContext::SetLogicalSize(Windows::Foundation::Size logicalSize)
{
	if (m_logicalSize != logicalSize)
	{
		m_logicalSize = logicalSize;
		CreateWindowSizeDependentResources();
	}
}

bool DxGraphicsContext::SetResolution(Windows::Foundation::Size resolution)
{
	UINT renderWidth = static_cast<UINT>(resolution.Width - 1);
	UINT renderHeight = static_cast<UINT>(resolution.Height - 1);

	//bool swapDimensions = m_displayRotation == DXGI_MODE_ROTATION_ROTATE90 || m_displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	//UINT tmp = renderWidth;
	//if (swapDimensions)
	//{
	//	renderWidth = renderHeight;
	//	renderHeight = tmp;
	//}

	// Change the region of the swap chain that will be presented to the screen.
	HRESULT hr = m_swapChain2->SetSourceSize(renderWidth, renderHeight);
	if (FAILED(hr))
	{
		return false;
	}

	// In Direct3D, change the Viewport to match the region of the swap
	// chain that will now be presented from.
	m_screenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		static_cast<float>(renderWidth + 1),
		static_cast<float>(renderHeight + 1)
		);

	m_d3dContext->RSSetViewports(1, &m_screenViewport);

	return true;
}

// This method is called in the event handler for the DisplayContentsInvalidated event.
void DxGraphicsContext::ValidateDevice()
{
	// The D3D Device is no longer valid if the default adapter changed since the device
	// was created or if the device has been removed.

	// First, get the information for the default adapter from when the device was created.

	ComPtr<IDXGIDevice3> dxgiDevice;
	DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

	ComPtr<IDXGIAdapter> deviceAdapter;
	DX::ThrowIfFailed(dxgiDevice->GetAdapter(&deviceAdapter));

	ComPtr<IDXGIFactory2> deviceFactory;
	DX::ThrowIfFailed(deviceAdapter->GetParent(IID_PPV_ARGS(&deviceFactory)));

	ComPtr<IDXGIAdapter1> previousDefaultAdapter;
	DX::ThrowIfFailed(deviceFactory->EnumAdapters1(0, &previousDefaultAdapter));

	DXGI_ADAPTER_DESC previousDesc;
	DX::ThrowIfFailed(previousDefaultAdapter->GetDesc(&previousDesc));

	// Next, get the information for the current default adapter.

	ComPtr<IDXGIFactory2> currentFactory;
	DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory)));

	ComPtr<IDXGIAdapter1> currentDefaultAdapter;
	DX::ThrowIfFailed(currentFactory->EnumAdapters1(0, &currentDefaultAdapter));

	DXGI_ADAPTER_DESC currentDesc;
	DX::ThrowIfFailed(currentDefaultAdapter->GetDesc(&currentDesc));

	// If the adapter LUIDs don't match, or if the device reports that it has been removed,
	// a new D3D device must be created.

	if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
		previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
		FAILED(m_d3dDevice->GetDeviceRemovedReason()))
	{
		// Release references to resources related to the old device.
		dxgiDevice = nullptr;
		deviceAdapter = nullptr;
		deviceFactory = nullptr;
		previousDefaultAdapter = nullptr;

		// Create a new device and swap chain.
		HandleDeviceLost();
	}
}

// Recreate all device resources and set them back to the current state.
void DxGraphicsContext::HandleDeviceLost()
{
	m_swapChain = nullptr;

	if (m_deviceNotify != nullptr)
	{
		m_deviceNotify->OnDeviceLost();
	}

	CreateDeviceResources();
	CreateWindowSizeDependentResources();

	if (m_deviceNotify != nullptr)
	{
		m_deviceNotify->OnDeviceRestored();
	}
}

// Register our DeviceNotify to be informed on device lost and creation.
void DxGraphicsContext::RegisterDeviceNotify(IDeviceNotify* deviceNotify)
{
	m_deviceNotify = deviceNotify;
}

// Call this method when the app suspends. It provides a hint to the driver that the app 
// is entering an idle state and that temporary buffers can be reclaimed for use by other apps.
void DxGraphicsContext::Trim()
{
	ComPtr<IDXGIDevice3> dxgiDevice;
	m_d3dDevice.As(&dxgiDevice);

	dxgiDevice->Trim();
}

// Present the contents of the swap chain to the screen.
bool DxGraphicsContext::swapBuffers()
{
	// The first argument instructs DXGI to block until VSync, putting the application
	// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	// frames that will never be displayed to the screen.
	HRESULT hr = m_swapChain->Present(1, 0);

	// Discard the contents of the render target.
	// This is a valid operation only when the existing contents will be entirely
	// overwritten. If dirty or scroll rects are used, this call should be removed.
	m_d3dContext->DiscardView(m_d3dRenderTargetView.Get());

	// Discard the contents of the depth stencil.
	m_d3dContext->DiscardView(m_d3dDepthStencilView.Get());

	// If the device was removed either by a disconnection or a driver upgrade, we 
	// must recreate all device resources.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		HandleDeviceLost();
		return false;
	}
	else
	{
		DX::ThrowIfFailed(hr);
	}

	m_d3dContext->OMSetRenderTargets(1, m_d3dRenderTargetView.GetAddressOf(), m_d3dDepthStencilView.Get());
	m_d3dContext->RSSetViewports(1, &m_screenViewport);
	return true;
}

bool DxGraphicsContext::makeCurrent()
{
	return true;
}

bool DxGraphicsContext::detachThread()
{
	return true;
}

bool DxGraphicsContext::isValid()
{
	return (NULL != m_d3dContext);
}

void DxGraphicsContext::setFormat(int32_t redBits, int32_t greenBits, int32_t blueBits, int32_t depthBits, int32_t stencilBits, int32_t fsaaSamples)
{
	m_format = tfw::GLFormat(redBits, greenBits, blueBits, depthBits, stencilBits, fsaaSamples);
	CreateWindowSizeDependentResources();
}

//--------- Interop
void ContextFactory::SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel)
{
	m_context.SetSwapChainPanel(panel);
}

void ContextFactory::SetLogicalSize(Windows::Foundation::Size logicalSize)
{
	m_context.SetLogicalSize(logicalSize);
}

bool ContextFactory::SetResolution(Windows::Foundation::Size resolution)
{
	return m_context.SetResolution(resolution);
}

long long ContextFactory::getContext()
{
	return (long long)&m_context;
}
