/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file texture.cpp
	Implementation of texture support functions.
*/
#include "texture.h"

using namespace GLB;

#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703

Texture::Texture ()
{
	m_scale = KCL::Vector2D (1, 1);
	m_imageFilter = FILTER_NEAREST;
	m_levelFilter = FILTER_BASE_LEVEL;
	m_wrapS = GL_REPEAT;
	m_wrapT = GL_REPEAT;
	m_minFilter = GL_NEAREST;
	m_magFilter = GL_NEAREST;
	m_paramsChanged = true;
}


Texture::~Texture ()
{
}


void Texture::setMinMagFilter (KCL::enum_t minFilter, KCL::enum_t magFilter)
{
	if (m_minFilter != minFilter)
	{
		m_minFilter = minFilter;
		m_paramsChanged = true;
	}
	if (m_magFilter != magFilter)
	{
		m_magFilter = magFilter;
		m_paramsChanged = true;
	}
}


void Texture::setFiltering (KCL::uint8 levelFilter, KCL::uint8 imageFilter)
{

	if (m_levelFilter != levelFilter)
		m_paramsChanged = true;
	m_levelFilter = levelFilter;
	if (m_imageFilter != imageFilter)
		m_paramsChanged = true;
	m_imageFilter = imageFilter;
#ifdef DISABLE_MIPMAPS
	if (levelFilter == FILTER_BASE_LEVEL && imageFilter == FILTER_NEAREST) {
		setMinMagFilter(GL_NEAREST, GL_NEAREST);
	} else if (levelFilter == FILTER_BASE_LEVEL && imageFilter == FILTER_LINEAR) {
		setMinMagFilter(GL_LINEAR, GL_LINEAR);
	} else if (levelFilter == FILTER_NEAREST && imageFilter == FILTER_NEAREST) {
		setMinMagFilter(GL_LINEAR, GL_NEAREST);
	} else if (levelFilter == FILTER_NEAREST && imageFilter == FILTER_LINEAR) {
		setMinMagFilter(GL_LINEAR, GL_LINEAR);
	} else if (levelFilter == FILTER_LINEAR && imageFilter == FILTER_NEAREST) {
		setMinMagFilter(GL_LINEAR, GL_NEAREST);
	} else if (levelFilter == FILTER_LINEAR && imageFilter == FILTER_LINEAR) {
		setMinMagFilter(GL_LINEAR, GL_LINEAR);
	}
#else
	if (levelFilter == FILTER_BASE_LEVEL && imageFilter == FILTER_NEAREST) {
		setMinMagFilter(GL_NEAREST, GL_NEAREST);
	} else if (levelFilter == FILTER_BASE_LEVEL && imageFilter == FILTER_LINEAR) {
		setMinMagFilter(GL_LINEAR, GL_LINEAR);
	} else if (levelFilter == FILTER_NEAREST && imageFilter == FILTER_NEAREST) {
		setMinMagFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
	} else if (levelFilter == FILTER_NEAREST && imageFilter == FILTER_LINEAR) {
		setMinMagFilter(GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR);
	} else if (levelFilter == FILTER_LINEAR && imageFilter == FILTER_NEAREST) {
		setMinMagFilter(GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
	} else if (levelFilter == FILTER_LINEAR && imageFilter == FILTER_LINEAR) {
		setMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	}
#endif
}


void Texture::setWrapping (KCL::uint8 wrapS, KCL::uint8 wrapT)
{
	if (wrapS == WRAP_REPEAT) {
		if (m_wrapS != GL_REPEAT)
		{
			m_wrapS = GL_REPEAT;
			m_paramsChanged = true;
		}
	} else {
		if (m_wrapS != GL_CLAMP_TO_EDGE)
		{
			m_wrapS = GL_CLAMP_TO_EDGE;
			m_paramsChanged = true;
		}
	}
	if (wrapT == WRAP_REPEAT) {
		if (m_wrapT != GL_REPEAT)
		{
			m_wrapT = GL_REPEAT;
			m_paramsChanged = true;
		}
	} else {
		if (m_wrapT != GL_CLAMP_TO_EDGE)
		{
			m_wrapT = GL_CLAMP_TO_EDGE;
			m_paramsChanged = true;
		}
	}
}


Texture2D::Texture2D (DXB::Image2D *image)
{
	setImage (image, true);
	m_textureType = COLOR;
}


Texture2D::Texture2D ()
{
	m_image = NULL;
	m_referenced = false;
	m_textureType = COLOR;
}


void Texture2D::setImage (DXB::Image2D *image, bool addReference)
{
	if (m_referenced) delete m_image;
	m_referenced = addReference;
	m_image = image;
}


Texture2D::~Texture2D ()
{
	if (m_referenced) delete m_image;
	m_referenced = false;
	m_image = NULL;
}


void Texture2D::bind ()
{
	if (m_paramsChanged)
	{
		
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = FLT_MAX;
		
		if (m_minFilter == GL_LINEAR && m_magFilter == GL_LINEAR )
		{
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		}
		else if (m_minFilter == GL_LINEAR && m_magFilter == GL_NEAREST )
		{
			samplerDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		}
		else
		{
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		}
		
		samplerDesc.AddressU = ( m_wrapS == GL_REPEAT ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP );
		samplerDesc.AddressV = ( m_wrapT == GL_REPEAT ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP );
		
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateSamplerState(
				&samplerDesc,
				&m_sampler
				)
			);
		m_paramsChanged = false;
	}
}



TextureCube::TextureCube (DXB::ImageCube *image)
{
	m_image = image;
	m_referenced = true;
	m_textureType = ENVMAP;
}


TextureCube::TextureCube ()
{
	m_image = NULL;
	m_referenced = false;
	m_textureType = ENVMAP;
}


TextureCube::~TextureCube ()
{
	delete m_image;
	m_image = NULL;
}


void TextureCube::bind ()
{
	m_paramsChanged = false;
}

void TextureCube::setImage (DXB::ImageCube *image, bool addReference)
{
	if (m_referenced) delete m_image;
	m_referenced = addReference;
	m_image = image;
}
