/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file image.cpp
	Implementation of image handling classes.
	Contains ImageBase and descendant classes Image2D and ImageCube.
*/
#include "dxb_image.h"

#include "stdc.h"
#include "etc1.h"
#include <kcl_os.h>
#include "misc2.h" //endian read

namespace KCL
{
#ifndef max
	template<typename T>
	inline T max(const T &a, const T &b)
	{
		return (a > b) ? a : b;
	}
#endif

#ifndef min
	template<typename T>
	inline T min(const T &a, const T &b)
	{
		return (a < b) ? a : b;
	}
#endif
}

using namespace DXB;


Image2D::Image2D () :
KCL::Image2D()
{}


Image2D::~Image2D ()
{
	reset();
}

void Image2D::commit (bool repeatS, bool repeatT)
{
	if( !m_data)
	{
		return;
	}

	bool supported = isFormatSupported ();
	if (!supported)
	{
		convertRGB ();
	}

	if (m_data && m_width && m_height)
	{
		KCL::uint8 *datas[16];
		D3D11_SUBRESOURCE_DATA tbsd[16];
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

		if ( m_format==KCL::Image_DXT1)
		{
			format = DXGI_FORMAT_BC1_UNORM;
			KCL::uint32 mp;
			for( KCL::uint32 i=0; i<=m_mipmaps; i++)
			{
				getMipmapData (i, &datas[i], 0, 0, 0, &mp);
			
				tbsd[i].pSysMem = (void *)datas[i];
				tbsd[i].SysMemPitch = mp * 4;
				tbsd[i].SysMemSlicePitch = m_width*m_height*4; // Not needed since this is a 2d texture
			}
		}
		else
		{
			decodeRGB888toRGBA8888();
			
			getMipmapData (0, datas, 0);
	
			tbsd[0].pSysMem = (void *)datas[0];
			tbsd[0].SysMemPitch = 4 * m_width;
			tbsd[0].SysMemSlicePitch = m_width*m_height*4; // Not needed since this is a 2d texture
		}

		CD3D11_TEXTURE2D_DESC tdesc(
			format,
			m_width,
			m_height,
			1,
			m_mipmaps + 1
			);
		
		Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
	
	    DX_THROW_IF_FAILED(
			DX::getDevice()->CreateTexture2D(&tdesc, tbsd,&tex)
		);

		CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(
			tex.Get(),
			D3D11_SRV_DIMENSION_TEXTURE2D
			);

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateShaderResourceView(
				tex.Get(),
				&shaderResourceViewDesc,
				&m_d3d11_id
				)
			);

		//m_d3d11_id->SetPrivateData( WKPDID_D3DDebugObjectName, m_name.size(), m_name.c_str() );

		delete [] m_data;
		m_data = NULL;
		m_mutable = false;
	}
	else
	{

	}

}

void Image2D::commit_debug_mipmap ( bool repeatS, bool repeatT)
{
	struct _color
	{
		KCL::uint8 r,g,b,a;
	};

	KCL::uint32 w = m_width;
	KCL::uint32 h = m_height;

	KCL::uint32 m_uiMipMapCount = 1;
	KCL::uint32 kk = max(w, h);
	while( kk > 1)
	{
		m_uiMipMapCount++;
		kk /= 2;
	}


	static _color dbgs[]=
	{
		{255, 0, 0, 0},
		{0, 255, 0, 0},
		{0, 0, 255, 0},
		{0, 255, 255, 0},
		{255, 0, 255, 0},
		{255, 255, 0, 0},
		{0, 0, 0, 0},
		{128, 0, 0, 0},
		{0, 128, 0, 0},
		{0, 0, 128, 0},
		{255, 255, 255, 0}
	};

	for( KCL::uint32 l=0; l<m_uiMipMapCount; l++)
	{
		_color *c = new _color[w*h];

		for(KCL::uint32 j=0; j<h; j++)
		{
			for(KCL::uint32 i=0; i<w; i++)
			{
				c[(i + j *w )] = dbgs[l];
			}
		}

		w /= 2;
		h /= 2;

		delete []c;
	}	

	//beware of NPOT FBO!
	
	delete [] m_data;
	m_data = NULL;
	m_mutable = false;
}


void Image2D::commitETC1 ()
{
	int offset = 0;
	for (KCL::uint32 i = 0; i <= m_mipmaps; i++) 
	{
		KCL::uint32 size = 1<<(m_mipmaps - i);
		KCL::uint32 wh = size;
		size *= size;
		size /= 2;
		size = size<8?8:size;
		offset += size;
	}
}


void Image2D::generateMipmap ()
{
#ifdef DISABLE_MIPMAPS
	return;
#endif
	if (m_mipmaps && m_width == m_height) 
	{
		KCL::uint8 *data = 0;
		for (KCL::uint32 i = 1; i < m_mipmaps; ++i) 
		{
			int size = 1<<(m_mipmaps-i-1);
			data = getMipmapData (i);
		}
	} 
}


void Image2D::uploadMipmap (int format)
{
	if (m_mipmaps) 
	{
		KCL::uint8 *data = 0;
		for (KCL::uint32 i = 1; i < m_mipmaps+1; ++i) 
		{
			KCL::uint32 mipmapsize = 0;
			KCL::uint32 w, h;
			getMipmapData (i, &data, &mipmapsize, &w, &h);
		}
	}
}

void Image2D::convertRGB ()
{
	if (m_format == KCL::Image_ETC1)
	{
		decodeETC1toRGB888 ();
	}
	else
	{
		assert("Not supported format! (Image2D::convertRGB)");
	}
}

// GLB::ImageCube

void ImageCube::commit (bool repeatS, bool repeatT)
{
	using namespace KCL;
	
	bool supported = isFormatSupported ();
	if (!supported) convertRGB ();

	switch (m_format)
	{
	case Image_ETC1:
		for (int i = 0; i < 6; ++i)
		{
			commitETC1(i);
		}
		break;
	default:
		{
			//INFO("we couldn't call the glTexImage!\n");
			break;
		}
	}
	for (int i = 0; i < 6; ++i)
	{
		delete [] m_data[i];
		m_data[i] = NULL;
	}

	//generateMipmap ();
}

void ImageCube::commitETC1 (int face)
{
	int offset = 0;
	for (KCL::uint32 i = 0; i <= m_mipmaps; ++i) 
	{
		KCL::uint32 size = 1<<(m_mipmaps - i);
		KCL::uint32 wh = size;
		size *= size;
		size /= 2;
		size = size<8?8:size;
		offset += size;
	}
}

void ImageCube::generateMipmap ()
{
#ifdef DISABLE_MIPMAPS
	return;
#endif

	if (m_format == KCL::Image_ETC1)
		return;
}
