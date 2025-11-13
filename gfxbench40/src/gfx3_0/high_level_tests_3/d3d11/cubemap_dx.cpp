/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "cubemap.h"
#include "platform.h"
#include "d3d11/dxb_image.h"
#include "misc2.h"//savebmp
#include "d3d11/fbo3.h"

#include <cassert>



CubeEnvMap::CubeEnvMap(int size)
{
}


CubeEnvMap* CubeEnvMap::Load( KCL::uint32 idx, const char* path )
{
	CubeEnvMap *cem = 0;
	DXB::Image2D* imgs[6]; 
	
	for(int i=0; i < 6; i++)
	{
		char name[1024] = {0};
		
		sprintf(name, "%senvmap%03d_%x.pvr", path, idx, 0x8515 + i); // GL_TEXTURE_CUBE_MAP_POSITIVE_X = 0x8515
		imgs[i] = new DXB::Image2D();
		bool isOk = imgs[i]->load( name);
		if(!isOk)
		{
			delete imgs[i];
			return 0;
		}
	}


	cem = new CubeEnvMap( 16);

	D3D11_SUBRESOURCE_DATA tbsdarray[6];
	uint32 m_width;
	uint32 m_height;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

	for(int i=0; i < 6; i++)
	{
		DXB::Image2D *img = imgs[i];

		bool supported = img->isFormatSupported ();
		if (!supported)
		{
			img->convertRGB ();
		}
		
		m_width = img->getWidth();
		m_height = img->getHeight();
		uint32 m_format = img->getFormat();
		uint32 m_width = img->getWidth();
		uint32 m_height = img->getHeight();
		void* m_data = img->data();

		if ( m_format==KCL::Image_DXT1)
		{
			format = DXGI_FORMAT_BC1_UNORM;
			uint8 *data = 0;
			uint32 mp;
			img->getMipmapData (0, &data, 0, 0, 0, &mp);
			
			tbsdarray[i].pSysMem = (void *)data;
			tbsdarray[i].SysMemPitch = 4*mp;
			tbsdarray[i].SysMemSlicePitch = m_width*m_height*4; // Not needed since this is a 2d texture
		}
		else
		{
			uint8 *data = 0;
			uint32 size = 0;

			img->decodeRGB888toRGBA8888();

			img->getMipmapData (0, &data, &size);
				
			tbsdarray[i].pSysMem = (void *)data;
			tbsdarray[i].SysMemPitch = 4*m_width;
			tbsdarray[i].SysMemSlicePitch = m_width*m_height*4; // Not needed since this is a 2d texture
		}
	}

	
	CD3D11_TEXTURE2D_DESC tdesc(
		format,
		m_width,
		m_height,
		6,
		1
		);
	tdesc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		
	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
	
	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateTexture2D(&tdesc,tbsdarray,&tex)
	);
	
	CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(
		tex.Get(),
		D3D11_SRV_DIMENSION_TEXTURECUBE
		);

	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateShaderResourceView(
			tex.Get(),
			&shaderResourceViewDesc,
			&cem->m_d3d11_id
			)
		);

	for(int i=0; i < 6; i++)
	{
		DXB::Image2D *img = imgs[i];
		delete img;
	}

	return cem;
}


void CubeEnvMap::Save( KCL::uint32 idx, KCL::uint32 target, KCL::uint32 size, const char* path )
{
	char filename[256];
	sprintf(filename, "%senvmap%03d_%x.png", path, idx, target);

	unsigned char* m = new unsigned char[size * size * 4];

	convertRGBAtoBGR(m, size*size);
	savePng( filename, size, size, m, 1);
	delete[] m;
}


CubeEnvMap::~CubeEnvMap()
{
}


FboEnvMap::FboEnvMap(int cubemapSize) : m_cubemapSize(cubemapSize)
{
}

FboEnvMap::~FboEnvMap()
{
}
	
void FboEnvMap::Bind()
{
}
	
void FboEnvMap::Unbind()
{
	//glBindFramebuffer(GL_FRAMEBUFFER, FBO::GetGlobalFBO()->getName() );
}
	
void FboEnvMap::AttachCubemap(CubeEnvMap *const cubemap, size_t face)
{
	assert(face < 6);
}

void FboEnvMap::DetachCubemap(size_t face)
{
	assert(face < 6);
}


bool GenerateNormalisationCubeMap( KCL::uint32 &texture_object, KCL::uint32 size)
{
	return true;
}
