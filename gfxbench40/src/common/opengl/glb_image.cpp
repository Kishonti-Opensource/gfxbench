/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file image.cpp
	Implementation of image handling classes.
	Contains ImageBase and descendant classes Image2D and ImageCube.
*/
#include "opengl/glb_image.h"

#include "stdc.h"
#include "etc1.h"
#include "opengl/ext.h"
#include <kcl_os.h>
#include "misc2.h" //endian read


namespace GLB
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

using namespace GLB;

int Image2D::needDecoding = Image2D::DecodeDisabled;

Image2D::Image2D () :
KCL::Image2D()
{}


Image2D::~Image2D ()
{
	if(m_id)
	{
		glDeleteTextures (1, &m_id);
	}
	reset();
}

void Image2D::commit (bool repeatS, bool repeatT)
{
	KCL::uint8 *data = 0;
	KCL::uint32 size = 0;
	bool supported = isFormatSupported ();
	if (!supported)
	{
		if ((m_format==KCL::Image_ETC1) && (needDecoding==DecodeTo565))
		{
			decodeETC1toRGB565();
		} else if ((m_format==KCL::Image_ETC1) && (needDecoding==DecodeTo888))
		{
			decodeETC1toRGB888();
		} else
		{
		convertRGB ();
	}
	}

	getMipmapData (0, &data, &size);


	glGenTextures (1, &m_id);
	glBindTexture (GL_TEXTURE_2D, m_id);
	
	//beware of NPOT FBO!
	if( mm_enabled)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	
	if(repeatS)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	}

	if(repeatT)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);

	using namespace KCL;

	switch (m_format) 
	{
	case Image_ALPHA_A8:
		glTexImage2D (GL_TEXTURE_2D, 0, GL_ALPHA, m_width, m_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, m_data);
		generateMipmap ();
		break;
	case Image_LUMINANCE_L8:
		glTexImage2D (GL_TEXTURE_2D, 0, GL_LUMINANCE, m_width, m_height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_data);
		generateMipmap ();
		break;
	case Image_LUMINANCE_ALPHA_LA88:
		glTexImage2D (GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, m_width, m_height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, m_data);
		generateMipmap ();
		break;
	case Image_RGB888:
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_data);
		generateMipmap ();
		break;
	case Image_RGB565:
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, m_data);
		generateMipmap ();
		break;
	case Image_RGBA8888:
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data);
		generateMipmap ();
		break;
	case Image_RGBA4444:
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, m_data);
		generateMipmap ();
		break;
	case Image_RGBA5551:
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, m_data);
		generateMipmap ();
		break;
	case Image_DXT1:
		if( m_has_pvr_alpha)
		{
			glCompressedTexImage2D (GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, m_width, m_height, 0, size, m_data);
			uploadMipmap (GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
		}
		else
		{
			glCompressedTexImage2D (GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, m_width, m_height, 0, size, m_data);
			uploadMipmap (GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
		}
		break;
	case Image_DXT3:
		glCompressedTexImage2D (GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, m_width, m_height, 0, size, m_data);
		uploadMipmap (GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
		break;
	case Image_DXT5:
		glCompressedTexImage2D (GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, m_width, m_height, 0, size, m_data);
		uploadMipmap (GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
		break;
	case Image_PVRTC2:
		glCompressedTexImage2D (GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, m_width, m_height, 0, size, m_data);
		uploadMipmap (GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG);
		break;
	case Image_PVRTC4:
		glCompressedTexImage2D (GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, m_width, m_height, 0, size, m_data);
		uploadMipmap (GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG);
		break;
	case Image_RGBA_ASTC_8x8:
		if(mm_enabled==false)
		{
			glCompressedTexImage2D (GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_ASTC_8x8_KHR, m_width, m_height, 0, size, m_data);
		}
		else
		{
			commitASTC();
		}
		break;
	case Image_ETC1:
		commitETC( GL_ETC1_RGB8_OES);
		break;
	case Image_ETC2_RGB:
		commitETC( GL_COMPRESSED_RGB8_ETC2);
		break;
	case Image_ETC2_RGBA8888:
		commitETC( GL_COMPRESSED_RGBA8_ETC2_EAC);
		break;
	case Image_RGBA_32F:
#ifdef OPENGL_DESKTOP
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, m_width, m_height, 0, GL_RGBA, GL_FLOAT, m_data);
#else
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_FLOAT, m_data);
#endif
		break;
	case Image_DEPTH_16:
		glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, m_data);
		break;
            
    default:
        break;
	}

	glBindTexture(GL_TEXTURE_2D,0);
	delete [] m_data;
	m_data = NULL;
	m_mutable = false;
}

void Image2D::commit_debug_mipmap ( bool repeatS, bool repeatT)
{
	glGenTextures (1, &m_id);
	glBindTexture (GL_TEXTURE_2D, m_id);


	glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
	glGetError();
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
		glTexImage2D (GL_TEXTURE_2D, l, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, c);

		w /= 2;
		h /= 2;

		delete []c;
	}	

	//beware of NPOT FBO!
	if( mm_enabled)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	
	if(repeatS)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	}

	if(repeatT)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	glBindTexture(GL_TEXTURE_2D,0);
	delete [] m_data;
	m_data = NULL;
	m_mutable = false;
}


void Image2D::commitASTC()
{
	int offset = 0;
	int width = m_width;
	int height = m_height;
	for (KCL::uint32 i = 0; i <= m_mipmaps; i++) 
	{
		KCL::uint32 size = 1<<(m_mipmaps - i);
		KCL::uint32 wh = width;
		KCL::uint32 hh = height;
		int x = (wh +8-1) / 8;
		int y = (hh +8-1) / 8;
		size = x * y  * 16;
		glCompressedTexImage2D (GL_TEXTURE_2D, i, GL_COMPRESSED_RGBA_ASTC_8x8_KHR, wh, hh, 0, size, m_data + offset );
		width /= 2;
		height /= 2;
		width < 1 ? width = 1:width;
		height < 1 ? height = 1:height;
		offset += size;
	}
}


void Image2D::commitETC( int internal_format)
{
	int offset = 0;
	for (KCL::uint32 i = 0; i <= m_mipmaps; i++) 
	{
		KCL::uint32 size = 1<<(m_mipmaps - i);
		KCL::uint32 wh = size;
		size *= size;
		size /= 2;
		size = size<8?8:size;
		glCompressedTexImage2D (GL_TEXTURE_2D, i, internal_format, wh, wh, 0, size, m_data+offset);
		offset += size;
	}
}


void Image2D::generateMipmap ()
{
#ifdef DISABLE_MIPMAPS
	return;
#endif
	glBindTexture (GL_TEXTURE_2D, m_id);
	if (m_mipmaps && m_width == m_height) 
	{
		KCL::uint8 *data = 0;
		for (KCL::uint32 i = 1; i < m_mipmaps; ++i) 
		{
			int size = 1<<(m_mipmaps-i-1);
			data = getMipmapData (i);

			// TODO: internalformat, format
			glTexImage2D (GL_TEXTURE_2D, i, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
	} 
	else 
	{
		//beware of NPOT FBO!

#if defined WIN32 && defined _DEBUG
		if( mm_enabled)
		{
			if(0 == m_width || 0 == m_height)
			{
				printf("Error in %s Image2D::generateMipmap! 0 == m_width || 0 == m_height\n", m_name.c_str());
				exit(-1);
			}
		}
#endif

		if( mm_enabled && m_width != 0 && m_height != 0)
		{
			glGenerateMipmap (GL_TEXTURE_2D);
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
			if (data)
			{
				glCompressedTexImage2D (GL_TEXTURE_2D, i, format, w, h, 0, mipmapsize, data);
			}
		}
	}
}

// GLB::ImageCube

void ImageCube::commit (bool repeatS, bool repeatT)
{
	using namespace KCL;

	bool supported = isFormatSupported ();
	if (!supported) convertRGB ();

	glGenTextures (1, &m_id);
	glBindTexture (GL_TEXTURE_CUBE_MAP, m_id);
	switch (m_format)
	{
	case Image_RGB565:
		for (int i = 0; i < 6; ++i)
		{
			glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, m_data[i]);
		}
		break;
	case Image_RGBA8888:
		for (int i = 0; i < 6; ++i)
		{
			glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data[i]);
		}
		break;
	case Image_RGB888:
		for (int i = 0; i < 6; ++i)
		{
			glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_data[i]);
		}
		break;
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
		glCompressedTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, i, GL_ETC1_RGB8_OES, wh, wh, 0, size, m_data[face]+offset);
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

	glBindTexture (GL_TEXTURE_CUBE_MAP, m_id);
	glGenerateMipmap (GL_TEXTURE_CUBE_MAP);
}
