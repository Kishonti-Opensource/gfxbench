/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "fbo3.h"
#include "kcl_os.h"
#include "kcl_io.h"
#include "misc2.h"

using namespace GLB;

GLB::FBO* FBO::m_currentGlobal = 0;

FBO::FBO() : m_d3d11_texture_msaa(nullptr), m_bUseMsaa(false)
{
}

bool FBO::initDepthOnly(KCL::uint32 width, KCL::uint32 height) //no MSAA
{
    m_d3d11_texture_msaa = nullptr;

	CD3D11_TEXTURE2D_DESC textureDesc(DXGI_FORMAT_R32_TYPELESS, width, height, 1, 1, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	
	CD3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D32_FLOAT);
	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R32_FLOAT, 0, 1);

    HRESULT result;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;

	{
		// Non-msaa only
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateTexture2D(&textureDesc, NULL, &tex)
		);

		DX_THROW_IF_FAILED(
			result = DX::getDevice()->CreateDepthStencilView(tex.Get(), &dsvDesc, &m_depthStencilView)
		);

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateShaderResourceView(tex.Get(), &srvDesc, &m_d3d11_tid)
		);

		m_bUseMsaa = false;
	}

	//static const char c_szName[] = "FBO";
	//m_d3d11_tid->SetPrivateData( WKPDID_D3DDebugObjectName, sizeof( c_szName ) - 1, c_szName );

	CD3D11_VIEWPORT viewPort(
        0.0f,
        0.0f,
		float(width),
		float(height)
        );

	m_viewport = viewPort;
		
	return true;
}

bool FBO::init (KCL::uint32 width, KCL::uint32 height, FBO_COLORMODE color_mode, FBO_DEPTHMODE depth_mode, int msaa)
{
	m_d3d11_texture_msaa = nullptr;

	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;

	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	
	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	
	if (msaa==1)
	{
		// Non-msaa only
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateTexture2D(&textureDesc, NULL, &tex)
		);

		DX_THROW_IF_FAILED(
			result = DX::getDevice()->CreateRenderTargetView(tex.Get(), &renderTargetViewDesc, &m_renderTargetView)
		);

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateShaderResourceView(tex.Get(), &shaderResourceViewDesc, &m_d3d11_tid)
		);

		m_bUseMsaa = false;
	}
	else
	{
		// non-msaa part
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateTexture2D(&textureDesc, NULL, &tex)
		);

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateShaderResourceView(tex.Get(), &shaderResourceViewDesc, &m_d3d11_tid)
		);
		
		// msaa only
		textureDesc.SampleDesc.Count = msaa;
		textureDesc.SampleDesc.Quality = 0;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;

		UINT qualityLevels;
		DX::getDevice()->CheckMultisampleQualityLevels(textureDesc.Format,msaa,&qualityLevels);

		if (qualityLevels==0)
		{
			return false;
		}

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateTexture2D(&textureDesc, NULL, &m_d3d11_texture_msaa)
		);

		DX_THROW_IF_FAILED(
			result = DX::getDevice()->CreateRenderTargetView(m_d3d11_texture_msaa.Get(), &renderTargetViewDesc, &m_renderTargetView)
		);

		m_bUseMsaa = true;
	}

	//static const char c_szName[] = "FBO";
	//m_d3d11_tid->SetPrivateData( WKPDID_D3DDebugObjectName, sizeof( c_szName ) - 1, c_szName );

	//TODO: depth mode kivulrol!
	if( depth_mode != DEPTH_None)
	{
		 // Create a descriptor for the depth/stencil buffer.
		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			DXGI_FORMAT_D24_UNORM_S8_UINT, 
			width,
			height,
			1,
			1,
			D3D11_BIND_DEPTH_STENCIL);
		// textureDesc.SampleDesc last set with msaa texture
		depthStencilDesc.SampleDesc.Count = textureDesc.SampleDesc.Count;
		depthStencilDesc.SampleDesc.Quality = textureDesc.SampleDesc.Quality;

		// Allocate a 2-D surface as the depth/stencil buffer.
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateTexture2D(
				&depthStencilDesc,
				nullptr,
				&depthStencil
				)
			);

		// Create a DepthStencil view on this surface to use on bind.
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateDepthStencilView(
				depthStencil.Get(),
				&CD3D11_DEPTH_STENCIL_VIEW_DESC(msaa == 1 ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS),
				&m_depthStencilView
				)
			);
	}

	CD3D11_VIEWPORT viewPort(
        0.0f,
        0.0f,
		float(width),
		float(height)
        );

	m_viewport = viewPort;
		
	return true;
}

KCL::Image* FBO::GetScreenshotImage()
{
	KCL::Image *img=new KCL::Image();
	if (GetScreenshotImage(*img)) return NULL;
	return img;
}

KCL::uint32 FBO::GetScreenshotImage(KCL::Image& img)
{
	ID3D11Resource *srcResPtr = NULL;
	if (GetGlobalFBO() != NULL)
	{
		GetGlobalFBO()->m_renderTargetView->GetResource(&srcResPtr);
	} else { 
		DX::getGraphicsContext()->getD3D11RenderTargetView()->GetResource(&srcResPtr);
	}

	assert(srcResPtr);

	D3D11_RESOURCE_DIMENSION dim = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	srcResPtr->GetType(&dim);
	if (dim != D3D11_RESOURCE_DIMENSION_TEXTURE2D) return 1;

	ID3D11Texture2D *srcTexPtr = NULL;
	srcResPtr->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&srcTexPtr));
	assert(srcTexPtr);

	D3D11_TEXTURE2D_DESC srcTexDesc;
	srcTexPtr->GetDesc(&srcTexDesc);
	if (DXGI_FORMAT_R8G8B8A8_UNORM != srcTexDesc.Format
		&& DXGI_FORMAT_B8G8R8A8_UNORM != srcTexDesc.Format)
		return 2;

	// Create a temporary staging texture with the same settings
	D3D11_TEXTURE2D_DESC dstTexDesc;
	memcpy(&dstTexDesc, &srcTexDesc, sizeof(D3D11_TEXTURE2D_DESC));
	dstTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	dstTexDesc.Usage = D3D11_USAGE_STAGING;
	dstTexDesc.BindFlags = 0;

	ID3D11Texture2D *dstTexPtr = NULL;
	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateTexture2D(&dstTexDesc, NULL, &dstTexPtr));
	assert(dstTexPtr);

	ID3D11Resource *dstResPtr = NULL;
	dstTexPtr->QueryInterface(__uuidof(ID3D11Resource), reinterpret_cast<void**>(&dstResPtr));
	assert(dstResPtr);

	DX::getContext()->CopyResource(dstResPtr, srcResPtr);

	size_t size = srcTexDesc.Width * srcTexDesc.Height * 4;
	D3D11_MAPPED_SUBRESOURCE txData;
	DX_THROW_IF_FAILED(
		DX::getContext()->Map(dstResPtr, 0, D3D11_MAP::D3D11_MAP_READ, 0, &txData));
	assert(txData.pData);

	img.Allocate2D(dstTexDesc.Width, dstTexDesc.Height, KCL::Image_RGBA8888);
	memcpy(img.getData(), txData.pData, size);

	if (srcTexDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM)
	{
		KCL::uint8* iptr = (KCL::uint8*)img.getData();
		for (int i = 0; i < size; i += 4)
		{
			KCL::uint8 tmp = *(iptr + 2);
			*(iptr + 2) = *iptr;
			*iptr = tmp;
			iptr += 4;
		}
	}

	DX::getContext()->Unmap(dstResPtr, 0);

	srcResPtr->Release();
	srcTexPtr->Release();
	dstResPtr->Release();
	dstTexPtr->Release();

	return 0;
}


void FBO::set_viewport( float x, float y, float w, float h, float mind, float maxd)
{
	CD3D11_VIEWPORT viewPort(
        x,
        y,
		w,
		h,
		mind,
		maxd
        );

	m_viewport = viewPort;
}

ID3D11ShaderResourceView* FBO::Get() 
{
	if (m_bUseMsaa)
	{
		ID3D11ShaderResourceView* dstresource = m_d3d11_tid.Get();
		ID3D11Resource* dst = NULL;
		ID3D11Resource* src = NULL;
		src = m_d3d11_texture_msaa.Get();
		dstresource->GetResource(&dst);

		DX::getContext()->ResolveSubresource(
			dst,
			0,
			src,
			0,
			DXGI_FORMAT_R8G8B8A8_UNORM
		);
		dst->Release();
	}

	return m_d3d11_tid.Get();
}


void FBO::bind( FBO *fbo)
{
	if( !fbo)
	{
		FBO* fbo_ = FBO::GetGlobalFBO();

		if( !fbo_)
		{
			DX::BindDefaultRenderTargetView();
			return;
		}
		fbo = fbo_;
	}

	DX::getContext()->OMSetRenderTargets(
		fbo->m_renderTargetView ? 1 : 0,
		fbo->m_renderTargetView ? fbo->m_renderTargetView.GetAddressOf() : NULL,
		fbo->m_depthStencilView.Get()
		);
   
	DX::getStateManager()->SetViewportNoDepth(fbo->m_viewport);
}


void FBO::clear( FBO *fbo, float r, float g, float b, float a)
{
	if( !fbo)
	{
		FBO* fbo_ = FBO::GetGlobalFBO();

		if( !fbo_)
		{
			DX::Clear( r, g, b, a);
			return;
		}
		fbo = fbo_;
	}

	const float clearColor[] = { r, g, b, a};

	DX::getStateManager()->SetBlendEnabled(false);

    if(fbo->m_renderTargetView)
    {
	    DX::getContext()->ClearRenderTargetView(
		    fbo->m_renderTargetView.Get(),
		    clearColor
		    );
    }

	if( fbo->m_depthStencilView.Get())
	{
		DX::getContext()->ClearDepthStencilView(
			fbo->m_depthStencilView.Get(),
			D3D11_CLEAR_DEPTH,
			1.0f,
			0
			);
	}
}

void FBO::discardDepth( FBO *fbo)
{
	if (!fbo)
	{
		FBO* fbo_ = FBO::GetGlobalFBO();
		if (!fbo_)
		{
			DX::DiscardDepth();
			return;
		}
		fbo = fbo_;
	}

    if (fbo->m_renderTargetView && fbo->m_depthStencilView.Get()) //do not discard depth if depth-only FBO
		DX::getContext()->DiscardView(fbo->m_depthStencilView.Get());
}

void FBO::discard( FBO *fbo )
{
	if( !fbo)
	{
		FBO* fbo_ = FBO::GetGlobalFBO();

		if( !fbo_)
		{
			DX::Discard();
			return;
		}
		fbo = fbo_;
	}

	DX::getContext()->DiscardView(fbo->m_renderTargetView.Get());

	if( fbo->m_depthStencilView.Get())
	{
		DX::getContext()->ClearDepthStencilView(
			fbo->m_depthStencilView.Get(),
			D3D11_CLEAR_DEPTH,
			1.0f,
			0
			);
	}
}

void FBO::SetGlobalFBO( FBO *fbo)
{
	m_currentGlobal = fbo;
}


FBO* FBO::GetGlobalFBO()
{
	return m_currentGlobal;
}
