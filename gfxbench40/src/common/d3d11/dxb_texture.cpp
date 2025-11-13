/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dxb_texture.h"
#include "kcl_os.h"
#include "kcl_io.h"

#define SAFE_RELEASE(x)	if ((x)) { (x)->Release(); (x) = NULL; }
#define EXIT_IF_FAILED(x) if (FAILED(x)) { return -1; }

D3D11_TEXTURE_ADDRESS_MODE getWrap(KCL::TextureWrap wrap)
{
	switch (wrap)
	{
	case KCL::TextureWrap::TextureWrap_Repeat:
		return D3D11_TEXTURE_ADDRESS_WRAP;

	case KCL::TextureWrap::TextureWrap_Mirror:
		return D3D11_TEXTURE_ADDRESS_MIRROR;

	case KCL::TextureWrap::TextureWrap_Clamp:
	default:
		return D3D11_TEXTURE_ADDRESS_CLAMP;
	}
}

D3D11_FILTER getFilter(KCL::TextureFilter minFilter, KCL::TextureFilter magFilter, KCL::TextureFilter mipFilter)
{
	KCL::uint32 filterCode = (minFilter << 16) | (magFilter << 8) | mipFilter;
	switch (filterCode)
	{
	case 0x010101:
		return D3D11_FILTER_MIN_MAG_MIP_POINT;

	case 0x010102:
		return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;

	case 0x010201:
		return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;

	case 0x010202:
		return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;

	case 0x020101:
		return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;

	case 0x020102:
		return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;

	case 0x020201:
		return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;

	case 0x020202:
		return D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	default:
		// Fall back to nearest filtering
		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	}
}

DXB::DXBTexture::DXBTexture() : 
	KCL::Texture(), 
	m_resource(NULL),
	m_texture(NULL),
	m_sampler(NULL),
	m_device(DX::getDevice())
{
}

DXB::DXBTexture::DXBTexture(_ogg_decoder* video) :
	KCL::Texture(video),
	m_resource(NULL),
	m_texture(NULL),
	m_sampler(NULL),
	m_device(DX::getDevice())
{
}

DXB::DXBTexture::DXBTexture(const KCL::Image* image, bool releaseUponCommit) : 
	KCL::Texture(image,releaseUponCommit), 
	m_resource(NULL),
	m_texture(NULL),
	m_sampler(NULL),
	m_device(DX::getDevice())
{
}

DXB::DXBTexture::DXBTexture(const KCL::Image* image, KCL::TextureType type, bool releaseUponCommit) : 
	KCL::Texture(image, type,releaseUponCommit), 
	m_resource(NULL),
	m_texture(NULL),
	m_sampler(NULL),
	m_device(DX::getDevice())
{
}

DXB::DXBTexture::~DXBTexture(void)
{
	release();
}

long DXB::DXBTexture::bind(KCL::uint32 slotId)
{
	DX::getContext()->PSSetSamplers(0, 1, &m_sampler);
	DX::getContext()->PSSetShaderResources(slotId, 1, &m_resource);
	return 0;
}

long DXB::DXBTexture::commit()
{
	if (!m_image && !m_video)
	{
		return -1;
	}

	if (m_image)
	{
		switch (m_image->getFormat())
		{
		case KCL::Image_ETC1:
		case KCL::Image_RGBA_ASTC_8x8:
		case KCL::Image_RGB888:
			{
				KCL::Image* convertedImage = m_image->cloneTo(KCL::Image_RGBA8888);
				if (m_releaseUponCommit)
				{
					delete m_image;
				}
				m_image = convertedImage;
				m_releaseUponCommit = true;

				break;
			}

		default:
			break;
		}
	}

	DXGI_FORMAT format = getTextureFormat();
	if (format == DXGI_FORMAT_UNKNOWN)
	{
		return -2;
	}

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.AddressU = getWrap(getWrapS());
	samplerDesc.AddressV = getWrap(getWrapT());
	samplerDesc.AddressW = getWrap(getWrapU());
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.Filter = getFilter(getMinFilter(), getMagFilter(), getMipFilter());

	DX_THROW_IF_FAILED(m_device->CreateSamplerState(&samplerDesc, &m_sampler));

	long result;
	switch (m_type)
	{
	case KCL::TextureType::Texture_1D:
		
		result = commit1D(format);
		break;

	case KCL::TextureType::Texture_2D:
		result = commit2D(format);
		break;

	case KCL::TextureType::Texture_Video:
		result = commitVideo(format);
		break;

	case KCL::TextureType::Texture_3D:
		result = commit3D(format);
		break;

	case KCL::TextureType::Texture_Array:
	case KCL::TextureType::Texture_Cube:
		result = commitArray(format);
		break;

	default:
		// Not supported texture format.
		result = -4;
		break;
	}

	if (m_releaseUponCommit)
	{
		delete m_image;
		m_image = NULL;
	}

	return result;
}

long DXB::DXBTexture::release()
{
	SAFE_RELEASE(m_resource);
	SAFE_RELEASE(m_texture);
	SAFE_RELEASE(m_sampler);
	return 0;
}

long DXB::DXBTexture::commit1D(DXGI_FORMAT format)
{
	D3D11_SUBRESOURCE_DATA subresource;
	subresource.pSysMem = m_image->getData();
	subresource.SysMemPitch = m_image->getLinePitch();
	subresource.SysMemSlicePitch = m_image->getLinePitch();

	CD3D11_TEXTURE1D_DESC descriptor(
		format,
		m_image->getWidth(),
		1, 
		m_mipLevels > 0 ? 1 : 0);	// set 0 to generate mip levels automatically.

	DX_THROW_IF_FAILED(m_device->CreateTexture1D(
		&descriptor,
		&subresource, 
		(ID3D11Texture1D**)&m_texture));

	CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(
		(ID3D11Texture1D*)m_texture,
		D3D11_SRV_DIMENSION_TEXTURE1D);

	DX_THROW_IF_FAILED(m_device->CreateShaderResourceView(
		(ID3D11Texture1D*)m_texture,
		&shaderResourceViewDesc,
		&m_resource));

	return 0;
}

long DXB::DXBTexture::commit2D(DXGI_FORMAT format)
{
	if (m_image->isUncompressed())
	{
		CD3D11_TEXTURE2D_DESC descriptor(
			format,
			m_image->getWidth(),
			m_image->getHeight(),
			1, 
			m_mipLevels > 0 ? 1 : 0, // set 0 to generate mip levels automatically.
            m_mipLevels > 0 ? D3D11_BIND_SHADER_RESOURCE : D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);	

		if (descriptor.MipLevels != 1)
		{
			descriptor.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		DX_THROW_IF_FAILED(m_device->CreateTexture2D(
			&descriptor,
			NULL, //no initial data
			(ID3D11Texture2D**)&m_texture));

        D3D11_BOX box;
        box.left = 0;
        box.right = m_image->getWidth();
        box.top = 0;
        box.bottom = m_image->getHeight();
        box.front = 0;
        box.back = 1;

        DX::getContext()->UpdateSubresource(
            m_texture,  // the texture to update
            0,	   // first mip-level
            &box,	 // position of the pixels to update in the texture
            m_image->getData(),   // image data
            m_image->getLinePitch(),	 // row pitch
            0	   // not used for 2D textures
        );

		CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(
			(ID3D11Texture2D*)m_texture,
			D3D11_SRV_DIMENSION_TEXTURE2D);

		DX_THROW_IF_FAILED(m_device->CreateShaderResourceView(
			(ID3D11Texture2D*)m_texture,
			&shaderResourceViewDesc,
			&m_resource));

		if (descriptor.MipLevels != 1)
		{
			DX::getContext()->GenerateMips(m_resource);
		}
	}
	else
	{
		D3D11_SUBRESOURCE_DATA subresources[16];	// This should be enough for 2^16=64K textures
		for (unsigned int i = 0; i < m_image->getMipCount(); i++)
		{
			getMipmapSubresource(i, &subresources[i]);
		}

		m_mipLevels = m_image->getMipCount();
		CD3D11_TEXTURE2D_DESC descriptor(
			format,
			m_image->getWidth(),
			m_image->getHeight(),
			1, 
			m_mipLevels,
            D3D11_BIND_SHADER_RESOURCE,
            D3D11_USAGE_IMMUTABLE);

		DX_THROW_IF_FAILED(m_device->CreateTexture2D(
			&descriptor,
			subresources, 
			(ID3D11Texture2D**)&m_texture));

		CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(
			(ID3D11Texture2D*)m_texture,
			D3D11_SRV_DIMENSION_TEXTURE2D);

		DX_THROW_IF_FAILED(m_device->CreateShaderResourceView(
			(ID3D11Texture2D*)m_texture,
			&shaderResourceViewDesc,
			&m_resource));
	}

	return 0;
}

long DXB::DXBTexture::commit3D(DXGI_FORMAT format)
{
    assert(m_image->isUncompressed());

	D3D11_SUBRESOURCE_DATA subresource;
	subresource.pSysMem = m_image->getData();
	subresource.SysMemPitch = m_image->getLinePitch();
	subresource.SysMemSlicePitch = m_image->getSlicePitch();

	CD3D11_TEXTURE3D_DESC descriptor(
		format,
		m_image->getWidth(),
		m_image->getHeight(),
		m_image->getDepth(), 
        m_mipLevels > 0 ? 0 : 1);	// set 0 to generate mip levels automatically.
		//m_mipLevels > 0 ? 1 : 0);	// set 0 to generate mip levels automatically.

	DX_THROW_IF_FAILED(m_device->CreateTexture3D(
		&descriptor,
		&subresource, 
		(ID3D11Texture3D**)&m_texture));

	CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(
		(ID3D11Texture3D*)m_texture,
		format);

	DX_THROW_IF_FAILED(m_device->CreateShaderResourceView(
		(ID3D11Texture3D*)m_texture,
		&shaderResourceViewDesc,
		&m_resource));

	return 0;
}

long DXB::DXBTexture::commitArray(DXGI_FORMAT format)
{
    assert(m_image->isUncompressed());

	UINT arraySize = m_image->getDepth();

	CD3D11_TEXTURE2D_DESC descriptor(
		format,
		m_image->getWidth(),
		m_image->getHeight(),
		arraySize, 
		m_mipLevels > 0 ? 1 : 0, 	// set 0 to generate mip levels automatically
        m_mipLevels > 0 ? D3D11_BIND_SHADER_RESOURCE : D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);

    if (descriptor.MipLevels != 1)
    {
        descriptor.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

	DX_THROW_IF_FAILED(m_device->CreateTexture2D(
		&descriptor,
		NULL, 
		(ID3D11Texture2D**)&m_texture));

    D3D11_BOX box;
    box.left = 0;
    box.right = m_image->getWidth();
    box.top = 0;
    box.bottom = m_image->getHeight();
    box.front = 0;
    box.back = 1;

    UINT memPitch = m_image->getLinePitch();
	UINT memSlicePitch = m_image->getSlicePitch();

    for(UINT i=0; i<arraySize; ++i)
    {
        DX::getContext()->UpdateSubresource(
            m_texture,  // the texture to update
            D3D11CalcSubresource(0,i,KCL::texture_levels(box.right, box.bottom)),
            &box,
            m_image->getData(i),
            memPitch,
            memSlicePitch
        );
    }

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	shaderResourceViewDesc.Format = format;
	shaderResourceViewDesc.Texture2DArray.ArraySize = m_image->getDepth();
	shaderResourceViewDesc.Texture2DArray.MipLevels = -1;

	DX_THROW_IF_FAILED(m_device->CreateShaderResourceView(
		(ID3D11Texture2D*)m_texture,
		&shaderResourceViewDesc,
		&m_resource));

    if (descriptor.MipLevels != 1)
    {
        DX::getContext()->GenerateMips(m_resource);
    }

	return 0;
}

long DXB::DXBTexture::commitVideo(DXGI_FORMAT format)
{
	CD3D11_TEXTURE2D_DESC descriptor(
		format,
		m_video->width,
		m_video->height,
		1, 
		1);

	descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	descriptor.Usage = D3D11_USAGE_DYNAMIC;

	DX_THROW_IF_FAILED(m_device->CreateTexture2D(
		&descriptor,
		NULL, 
		(ID3D11Texture2D**)&m_texture));

	CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(
		(ID3D11Texture2D*)m_texture,
		D3D11_SRV_DIMENSION_TEXTURE2D);

	DX_THROW_IF_FAILED(m_device->CreateShaderResourceView(
		(ID3D11Texture2D*)m_texture,
		&shaderResourceViewDesc,
		&m_resource));

	return 0;
}

long DXB::DXBTexture::setVideoTime(float time)
{
	long result = KCL::Texture::setVideoTime(time);
	if (result <= 0)
	{
		return result;
	}

	if (NULL != m_video)
	{
		D3D11_MAPPED_SUBRESOURCE sr;
		DX_THROW_IF_FAILED(
			DX::getContext()->Map(m_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &sr));
		m_video->Decode(sr.pData);
		DX::getContext()->Unmap(m_texture, 0);
		return 0;
	}
	else
	{
		return -1;
	}
}

void DXB::DXBTexture::getMipmapSubresource(UINT mipLevel, D3D11_SUBRESOURCE_DATA* subresourceData) const
{
	m_image->getMipmapData(
		mipLevel, 
		(KCL::uint8**)&subresourceData->pSysMem, 
		NULL,
		NULL,
		NULL,
		&subresourceData->SysMemPitch);

	subresourceData->SysMemPitch *= 4;
	subresourceData->SysMemSlicePitch = subresourceData->SysMemPitch * m_image->getHeight();
}