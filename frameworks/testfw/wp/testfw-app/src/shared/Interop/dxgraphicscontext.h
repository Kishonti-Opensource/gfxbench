/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DX_GRAPHICS_CONTEXT_H_
#define DX_GRAPHICS_CONTEXT_H_

#include "graphics/glformat.h"
#include "graphics/dxgraphicscontext.h"

using namespace Platform;

// Provides an interface for an application that owns DeviceResources to be notified of the device being lost or created.
interface IDeviceNotify
{
	virtual void OnDeviceLost() = 0;
	virtual void OnDeviceRestored() = 0;
};

/**
Represents a Direct3D graphics context.
Since the instantiation of the render target depends on actual platform (Windows desktop/RT/Phone), this class should be kept abstract.
*/
class DxGraphicsContext : public DXGraphicsContext
{
protected:
	D3D_FEATURE_LEVEL m_featureLevel;
	D3D_DRIVER_TYPE m_driverType;

public:
	DxGraphicsContext(void);

	void SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel);
	void SetLogicalSize(Windows::Foundation::Size logicalSize);
	bool SetResolution(Windows::Foundation::Size resolution);
	
	void ValidateDevice();
	void HandleDeviceLost();
	void RegisterDeviceNotify(IDeviceNotify* deviceNotify);
	void Trim();

	/* override */ virtual void resize(int32_t newx, int32_t newy) {}
	/* override */ virtual bool isValid();
	/* override */ virtual bool makeCurrent();
	/* override */ virtual bool detachThread();
	/* override */ virtual bool swapBuffers();
	/* override */ virtual void updateSize(UINT width, UINT height) {};

	//TODO
	int versionMajor() { return 0; }
	int versionMinor() { return 0; }
	bool hasFlag(int flag) { return true; }

	ID3D11Device1* getD3D11Device() const	{ return m_d3dDevice.Get(); }
	ID3D11DeviceContext1* getD3D11DeviceContext() const	{ return m_d3dContext.Get(); }
	ID3D11RenderTargetView* getD3D11RenderTargetView() const { return m_d3dRenderTargetView.Get(); }
	ID3D11DepthStencilView* getD3D11DepthStencilView() const { return m_d3dDepthStencilView.Get(); }
	D3D11_VIEWPORT* getDefaultViewport() { return &m_screenViewport; }

	void setFormat(int32_t redBits, int32_t greenBits, int32_t blueBits, int32_t depthBits, int32_t stencilBits, int32_t fsaaSamples);

	inline D3D_FEATURE_LEVEL getFeatureLevel() const	{ return m_featureLevel; }

	// Device Accessors.
	Windows::Foundation::Size GetOutputSize() const					{ return m_outputSize; }
	Windows::Foundation::Size GetLogicalSize() const				{ return m_logicalSize; }

private:
	void CreateDeviceIndependentResources();
	void CreateDeviceResources();
	void CreateWindowSizeDependentResources();
	//DXGI_MODE_ROTATION ComputeDisplayRotation();

	// Direct3D objects.
	Microsoft::WRL::ComPtr<ID3D11Device1>			m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1>	m_d3dContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain1>			m_swapChain;
	Microsoft::WRL::ComPtr<IDXGISwapChain2>			m_swapChain2;


	// Direct3D rendering objects. Required for 3D.
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	m_d3dRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_d3dDepthStencilView;
	D3D11_VIEWPORT									m_screenViewport;

	// Cached reference to the XAML panel.
	Windows::UI::Xaml::Controls::SwapChainPanel^    m_swapChainPanel;

	// Cached device properties.
	D3D_FEATURE_LEVEL								m_d3dFeatureLevel;
	Windows::Foundation::Size						m_d3dRenderTargetSize;
	Windows::Foundation::Size						m_outputSize;
	Windows::Foundation::Size						m_logicalSize;
	float											m_compositionScaleX;
	float											m_compositionScaleY;

	// Transforms used for display orientation.
	D2D1::Matrix3x2F	m_orientationTransform2D;
	DirectX::XMFLOAT4X4	m_orientationTransform3D;

	// The IDeviceNotify can be held directly as it owns the DeviceResources.
	IDeviceNotify* m_deviceNotify;

	DXGI_MODE_ROTATION m_displayRotation;

	tfw::GLFormat m_format;
};

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP)
namespace WindowsPhoneInterop
#else
namespace WindowsRTInterop
#endif

{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class ContextFactory sealed
	{
	public:
		void SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel);
		void SetLogicalSize(Windows::Foundation::Size logicalSize);
		bool SetResolution(Windows::Foundation::Size resolution);

		long long getContext();

		inline void setFormat(int32_t redBits, int32_t greenBits, int32_t blueBits, int32_t depthBits, int32_t stencilBits, int32_t fsaaSamples)
		{
			m_context.setFormat(redBits, greenBits, blueBits, depthBits, stencilBits, fsaaSamples);
		}

	private:
		DxGraphicsContext m_context;
	};
}

#endif