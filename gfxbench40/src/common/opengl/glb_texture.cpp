/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_texture.h"
#include "opengl/glb_opengl_state_manager.h"
#include "opengl/ext.h"
#include <cassert>
#include <sstream>

// Possible error codes for commit()
#define GLB_TEXERROR_NO_ERROR				 0
#define GLB_TEXERROR_ALREADY_COMMITTED		-1
#define GLB_TEXERROR_IMAGE_NOT_SET			-2
#define GLB_TEXERROR_INVALID_TYPE			-3
#define GLB_TEXERROR_NOT_IMPLEMENTED		-4
#define GLB_TEXERROR_INVALID_SLICE_COUNT	-5
#define GLB_TEXERROR_INVALID_FORMAT			-6

#define ANIOSOTROPIC_FILTER_VALUE			4.0f

#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT		0x84FE
#endif

#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT		0x84FF
#endif

GLB::sampler::sampler(KCL::uint32 samplerID) : m_samplerID(samplerID), m_refCount(1)
{
}

GLB::samplingDesc::samplingDesc(GLenum minFilter, GLenum magFilter, GLenum wrapS, GLenum wrapT, GLenum wrapR, GLfloat aniosotropy) :
    m_minFilter(minFilter),
    m_magFilter(magFilter),
    m_wrapS(wrapS),
    m_wrapT(wrapT),
    m_wrapR(wrapR),
	m_aniosotropy(aniosotropy)
{
	std::stringstream ss;

	ss << m_minFilter << " " << m_magFilter << " " << m_wrapS << " " << m_wrapT << " " << m_wrapR << " " << m_aniosotropy;

	id = ss.str();
}

bool GLB::samplingDesc::operator<(const GLB::samplingDesc &other) const 
{
	return id < other.id;
}

GLB::GLBTexture::GLBTexture() : 
	KCL::Texture(),
	m_textureId(0)
{	
}

GLB::GLBTexture::GLBTexture(const KCL::Image* image, bool releaseUponCommit) : 
    KCL::Texture(image, releaseUponCommit),
	m_textureId(0)
{
}

GLB::GLBTexture::GLBTexture(const KCL::Image* image, KCL::TextureType type, bool releaseUponCommit) : 
    KCL::Texture(image, type, releaseUponCommit),
	m_textureId(0)
{
}

GLB::GLBTexture::~GLBTexture(void)
{
}

GLenum GLB::GLBTexture::getGlTextureType(KCL::TextureType type)
{
	switch (type)
	{
#ifdef GL_TEXTURE_1D
	case KCL::Texture_1D:
		return GL_TEXTURE_1D;
#endif

#ifdef GL_TEXTURE_2D
	case KCL::Texture_2D:
		return GL_TEXTURE_2D;
#endif

#ifdef GL_TEXTURE_3D
	case KCL::Texture_3D:
		return GL_TEXTURE_3D;
#endif

#ifdef GL_TEXTURE_2D_ARRAY
	case KCL::Texture_Array:
		return GL_TEXTURE_2D_ARRAY;
#endif

#ifdef GL_TEXTURE_CUBE_MAP
	case KCL::Texture_Cube:
		return GL_TEXTURE_CUBE_MAP;
#endif

	default:
		return 0;
	}
}

GLint GLB::GLBTexture::getGlTextureFilter(KCL::TextureFilter filter, KCL::TextureFilter mipFilter)
{
	switch (mipFilter)
	{
	case KCL::TextureFilter_Linear:
		switch (filter)
		{
		case KCL::TextureFilter_Linear:
			return GL_LINEAR_MIPMAP_LINEAR;

		case KCL::TextureFilter_Nearest:
			return GL_NEAREST_MIPMAP_LINEAR;

		default:
			return 0;
		}

	case KCL::TextureFilter_Nearest:
		switch (filter)
		{
		case KCL::TextureFilter_Linear:
			return GL_LINEAR_MIPMAP_NEAREST;

		case KCL::TextureFilter_Nearest:
			return GL_NEAREST_MIPMAP_NEAREST;

		default:
			return 0;
		}

	case KCL::TextureFilter_NotApplicable:
		switch (filter)
		{
		case KCL::TextureFilter_Linear:
			return GL_LINEAR;

		case KCL::TextureFilter_Nearest:
			return GL_NEAREST;

		default:
			return 0;
		}

	default:
		return 0;
	}
}

GLint GLB::GLBTexture::getGlTextureWrap(KCL::TextureWrap wrap)
{
	switch (wrap)
	{
	case KCL::TextureWrap_Clamp:
		return GL_CLAMP_TO_EDGE;

	case KCL::TextureWrap_Mirror:
		return GL_MIRRORED_REPEAT;

	case KCL::TextureWrap_Repeat:
		return GL_REPEAT;

	default:
		return 0;
	}
}

bool mipmapRequired(KCL::TextureFilter filter)
{
	switch (filter)
	{
	case KCL::TextureFilter_Linear:
	case KCL::TextureFilter_Nearest:
		return true;

	default:
		return false;
	}
}

void GLB::GLBTexture::getGlTextureFormatES2(KCL::ImageFormat imgFormat, GLint& internalFormat, GLint& format, GLenum& type)
{ 
	switch (imgFormat)
	{
	case KCL::Image_ALPHA_A8:
		internalFormat = format = GL_ALPHA;
		type = GL_UNSIGNED_BYTE;
		break;

	case KCL::Image_LUMINANCE_L8:
		internalFormat = format = GL_LUMINANCE;
		type = GL_UNSIGNED_BYTE;
		break;

	case KCL::Image_LUMINANCE_ALPHA_LA88:
		internalFormat = format = GL_LUMINANCE_ALPHA;
		type = GL_UNSIGNED_BYTE;
		break;

	case KCL::Image_RGB888:
		internalFormat = format = GL_RGB;
		type = GL_UNSIGNED_BYTE;
		break;

	case KCL::Image_RGB565:
		internalFormat = format = GL_RGB;
		type = GL_UNSIGNED_SHORT_5_6_5;
		break;

	case KCL::Image_RGBA8888:
		internalFormat = format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
		break;

	case KCL::Image_RGBA4444:
		internalFormat = format = GL_RGBA;
		type = GL_UNSIGNED_SHORT_4_4_4_4;
		break;

	case KCL::Image_RGBA5551:
		internalFormat = format = GL_RGBA;
		type = GL_UNSIGNED_SHORT_5_5_5_1;
		break;

	case KCL::Image_RGB_32F:
		internalFormat = GL_RGB32F;
		format = GL_RGB;
		type = GL_FLOAT;
		break;

	case KCL::Image_RGBA_32F:
		internalFormat = GL_RGBA32F;
		format = GL_RGBA;
		type = GL_FLOAT;
		break;

	case KCL::Image_DEPTH_16:
		internalFormat = format = GL_DEPTH_COMPONENT;
		type = GL_UNSIGNED_SHORT;
		break;

	case KCL::Image_DXT1:
		internalFormat = format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		type = 0;
		break;

	case KCL::Image_DXT1_RGBA:
		internalFormat = format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		type = 0;
		break;

	case KCL::Image_DXT3:
		internalFormat = format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		type = 0;
		break;

	case KCL::Image_DXT5:
		internalFormat = format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		type = 0;
		break;

	case KCL::Image_PVRTC2:
		internalFormat = format = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
		type = 0;
		break;

	case KCL::Image_PVRTC4:
		internalFormat = format = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		type = 0;
		break;

	case KCL::Image_ETC1:
		internalFormat = format = GL_ETC1_RGB8_OES;
		type = 0;
		break;
	case KCL::Image_ETC2_RGB:
		internalFormat = format = GL_COMPRESSED_RGB8_ETC2;
		type = 0;
		break;
	case KCL::Image_ETC2_RGBA8888:
		internalFormat = format = GL_COMPRESSED_RGBA8_ETC2_EAC;
		type = 0;
		break;

	default:
		internalFormat = format = 0;
		type = 0;
		break;
	}
}

void GLB::GLBTexture::getGlTextureFormatES3(KCL::ImageFormat imgFormat, bool srgb, GLint& internalFormat, GLint& format, GLenum& type)
{ 
#if defined HAVE_GLES3 || defined __glew_h__
	switch (imgFormat)
	{
	case KCL::Image_ALPHA_A8:
		internalFormat = GL_ALPHA;
		format = GL_ALPHA;
		type = GL_UNSIGNED_BYTE;
		break;

	case KCL::Image_LUMINANCE_L8:
		internalFormat = GL_LUMINANCE;
		format = GL_LUMINANCE;
		type = GL_UNSIGNED_BYTE;
		break;

	case KCL::Image_LUMINANCE_ALPHA_LA88:
		internalFormat = GL_LUMINANCE_ALPHA;
		format = GL_LUMINANCE_ALPHA;
		type = GL_UNSIGNED_BYTE;
		break;

	case KCL::Image_RGB888:
		internalFormat = (srgb)? GL_SRGB8 : GL_RGB8;
		format = GL_RGB;
		type = GL_UNSIGNED_BYTE;
		break;

	case KCL::Image_RGB565:
		internalFormat = GL_RGB565;
		format = GL_RGB;
		type = GL_UNSIGNED_SHORT_5_6_5;
		break;

	case KCL::Image_RGBA8888:
		internalFormat = (srgb)? GL_SRGB8_ALPHA8 : GL_RGBA8;
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
		break;

	case KCL::Image_RGBA4444:
		internalFormat = GL_RGBA4;
		format = GL_RGBA;
		type = GL_UNSIGNED_SHORT_4_4_4_4;
		break;

	case KCL::Image_RGBA5551:
		internalFormat = GL_RGB5_A1;
		format = GL_RGBA;
		type = GL_UNSIGNED_SHORT_5_5_5_1;
		break;

	case KCL::Image_RGB_32F:
		internalFormat = GL_RGB32F;
		format = GL_RGB;
		type = GL_FLOAT;
		break;

	case KCL::Image_RGBA_32F:
		internalFormat = GL_RGBA32F;
		format = GL_RGBA;
		type = GL_FLOAT;
		break;

	case KCL::Image_DEPTH_16:
		internalFormat = GL_DEPTH_COMPONENT16;
		format = GL_DEPTH_COMPONENT;
		type = GL_UNSIGNED_SHORT;
		break;

	case KCL::Image_DXT1:
		format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		internalFormat = (srgb)? GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:GL_COMPRESSED_RGB_S3TC_DXT1_EXT ;
		type = 0;
		break;

	case KCL::Image_DXT1_RGBA:
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		internalFormat = (srgb)? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ;
		type = 0;
		break;

	case KCL::Image_DXT3:
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		internalFormat = (srgb)? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ;
		type = 0;
		break;

	case KCL::Image_DXT5:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		internalFormat = (srgb)? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ;
		type = 0;
		break;

	case KCL::Image_PVRTC2:
		format = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
		internalFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
		type = 0;
		break;

	case KCL::Image_PVRTC4:
		format = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		type = 0;
		break;

	case KCL::Image_ETC1:
		internalFormat = format = GL_ETC1_RGB8_OES;
		type = 0;
		break;
	case KCL::Image_ETC2_RGB:
		internalFormat = format = (srgb)?GL_COMPRESSED_SRGB8_ETC2:GL_COMPRESSED_RGB8_ETC2;
		type = 0;
		break;
	case KCL::Image_ETC2_RGBA8888:
		internalFormat = format = (srgb)?GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:GL_COMPRESSED_RGBA8_ETC2_EAC;
		type = 0;
		break;

	case KCL::Image_RGBA_ASTC_4x4:
		internalFormat = format = (srgb)?GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
		type = 0;
		break;
	case KCL::Image_RGBA_ASTC_5x5:
		internalFormat = format = (srgb)?GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
		type = 0;
		break;
	case KCL::Image_RGBA_ASTC_8x8:
		internalFormat = format = (srgb)?GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
		type = 0;
		break;

	default:
		internalFormat = format = 0;
		type = 0;
		break;
	}
#endif
}


bool GLB::GLBTexture::isFormatSupported(KCL::ImageFormat imgFormat)
{
	switch (imgFormat) 
	{
	case KCL::Image_LUMINANCE_L8:
	case KCL::Image_ALPHA_A8:
	case KCL::Image_LUMINANCE_ALPHA_LA88:
	case KCL::Image_RGB565:
	case KCL::Image_RGBA4444:
	case KCL::Image_RGBA5551:
	case KCL::Image_RGB888:
	case KCL::Image_RGBA8888:
		return true;
	case KCL::Image_DXT1:
    case KCL::Image_DXT1_RGBA:
	case KCL::Image_DXT3:
	case KCL::Image_DXT5:
		if(!GLB::g_extension)
		{
			return false;
		}
		return GLB::g_extension->hasExtension(GLB::GLBEXT_texture_compression_s3tc);
	case KCL::Image_PVRTC2:
	case KCL::Image_PVRTC4:
		if(!GLB::g_extension)
		{
			return false;
		}
		return GLB::g_extension->hasExtension(GLB::GLBEXT_texture_compression_pvrtc);
	case KCL::Image_RGBA_ASTC_4x4:
	case KCL::Image_RGBA_ASTC_5x5:
    case KCL::Image_RGBA_ASTC_8x8:
	    if(!g_extension)
	    {
		    return false;
	    }
	    return g_extension->hasExtension(GLBEXT_texture_compression_astc_ldr);
	case KCL::Image_ETC1:
		if(!GLB::g_extension)
		{
			return false;
		}
		return GLB::g_extension->hasExtension(GLB::GLBEXT_compressed_ETC1_RGB8_texture);
	case KCL::Image_ETC2_RGBA8888:
	case KCL::Image_ETC2_RGB:
		{
			//TODO: isFormatSupported, is_context_es3 
			bool is_context_es3 = true;

			if( is_context_es3)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

	default:
		return false;
	}
}

GLfloat GLB::GLBTexture::getMaxAnisotropy()
{
	if (!(g_extension && g_extension->hasExtension(GLBEXT_texture_filter_anisotropic)))
	{
		return 0;
	}
	float value = 0;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &value);
	return value;
}


GLB::GLBTextureES2::GLBTextureES2() : GLB::GLBTexture()
{
}

GLB::GLBTextureES2::GLBTextureES2(const KCL::Image* image, bool releaseUponCommit) : GLB::GLBTexture(image, releaseUponCommit)
{
}

GLB::GLBTextureES2::GLBTextureES2(const KCL::Image* image, KCL::TextureType type, bool releaseUponCommit) : GLB::GLBTexture(image, type, releaseUponCommit)
{
}

GLB::GLBTextureES2::~GLBTextureES2(void)
{
	release();
}

void GLB::GLBTextureES2::bindSampling(KCL::uint32 slotId) {}

long GLB::GLBTextureES2::bind(KCL::uint32 slotId)
{
    assert(m_isCommitted);

	if (slotId > 15)
	{
		return -1;
	}

    OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + slotId);

    switch (m_type)
	{
#ifdef GL_TEXTURE_1D
	case KCL::Texture_1D:
		glBindTexture (GL_TEXTURE_1D, m_textureId);
	    bindSampling(slotId);
		return GLB_TEXERROR_NO_ERROR;
#endif

#ifdef GL_TEXTURE_2D
	case KCL::Texture_2D:
		glBindTexture (GL_TEXTURE_2D, m_textureId);
	    bindSampling(slotId);
		return GLB_TEXERROR_NO_ERROR;
#endif

#ifdef GL_TEXTURE_3D
	case KCL::Texture_3D:
		glBindTexture (GL_TEXTURE_3D, m_textureId);
	    bindSampling(slotId);
		return GLB_TEXERROR_NO_ERROR;
#endif

#ifdef GL_TEXTURE_2D_ARRAY
	case KCL::Texture_Array:
		glBindTexture (GL_TEXTURE_2D_ARRAY, m_textureId);
	    bindSampling(slotId);
		return GLB_TEXERROR_NO_ERROR;
#endif

#ifdef GL_TEXTURE_CUBE_MAP
	case KCL::Texture_Cube:
		glBindTexture (GL_TEXTURE_CUBE_MAP, m_textureId);
	    bindSampling(slotId);
		return GLB_TEXERROR_NO_ERROR;
#endif

#ifdef GL_TEXTURE_2D
	case KCL::Texture_Video:
		glBindTexture (GL_TEXTURE_2D, m_textureId);
	    bindSampling(slotId);
		return GLB_TEXERROR_NO_ERROR;
#endif

	default:
		// Not supported texture type.
		return GLB_TEXERROR_INVALID_TYPE;
	}
}

long GLB::GLBTextureES2::setVideoTime(float time)
{
	// To be implemented.
	return GLB_TEXERROR_NOT_IMPLEMENTED;
}

long GLB::GLBTextureES2::commit()
{
    if (m_isCommitted)
	{
		// Already committed
		return GLB_TEXERROR_ALREADY_COMMITTED;
	}

    long result;
	if (m_video)
	{
		result = commitVideo();

        if(result == GLB_TEXERROR_NO_ERROR)
        {
            m_isCommitted = true;
        }

        if (m_releaseUponCommit)
        {
            delete m_video;
            m_video = NULL;
        }

        return result;
	}
	else if (m_image)
	{
	    //bool supported = isFormatSupported(fmt);
	    //if (!supported)
        if((KCL::Texture::decodeTarget != KCL::ImageTypeAny) && (m_image->getFormat() == KCL::Texture::decodeSource))
	    {
            const KCL::Image* img = m_image->cloneTo(KCL::Texture::decodeTarget);  
            if(m_releaseUponCommit)
            {
                delete m_image;
            }
            m_image = img;
	    }

        m_sRGB_values = m_image->isDataInSRGB();

		switch (m_type)
		{
		case KCL::Texture_1D:
			result = commit1D();
            break;
		case KCL::Texture_2D:
			result = commit2D();
            break;
		case KCL::Texture_3D:
			result = commit3D();
            break;
		case KCL::Texture_Array:
			result = commitArray();
            break;
		case KCL::Texture_Cube:
			result = commitCube();
            break;
		case KCL::Texture_CubeArray:
			result = commitCubeArray();
			break;
		default:
			// Not supported texture type.
			result = GLB_TEXERROR_INVALID_TYPE;
            break;
		}

        if(result == GLB_TEXERROR_NO_ERROR)
        {
            m_isCommitted = true;
        }

        if (m_releaseUponCommit)
        {
            delete m_image;
            m_image = NULL;
        }

        return result;
	}
	else
	{
		return GLB_TEXERROR_IMAGE_NOT_SET;
	}
}

void GLB::GLBTextureES2::generateMipmap ()
{
#ifdef DISABLE_MIPMAPS
	return;
#endif
	glBindTexture (GL_TEXTURE_2D, m_textureId);
    if (m_image->getMipCount() && (m_image->getWidth() == m_image->getHeight()) )
	{
		KCL::uint8 *data = 0;
		for (KCL::uint32 i = 1; i < m_image->getMipCount(); ++i) 
		{
			int size = 1<<(m_image->getMipCount()-i-1);
			data = m_image->getMipmapData(i);

			// TODO: internalformat, format
			glTexImage2D (GL_TEXTURE_2D, i, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
	} 
	else 
	{
		//beware of NPOT FBO!
        bool mm_enabled = mipmapRequired(m_mipFilter);


#if defined WIN32 && defined _DEBUG
		if( mm_enabled )
		{
			if( (0 == m_image->getWidth()) || (0 == m_image->getHeight()) )
			{
				printf("Error in %s GLB::GLBTextureES2::generateMipmap! 0 == m_width || 0 == m_height\n", m_name.c_str());
				exit(-1);
			}
		}
#endif

		if( mm_enabled && (m_image->getWidth() != 0) && (m_image->getHeight() != 0) )
		{
			glGenerateMipmap (GL_TEXTURE_2D);
		}
	}
}


void GLB::GLBTextureES2::uploadMipmap (int format)
{
    if (m_image->getMipCount()) 
	{
		KCL::uint8 *data = 0;
		for (KCL::uint32 i = 1; i < m_image->getMipCount()+1; ++i) 
		{
			KCL::uint32 mipmapsize = 0;
			KCL::uint32 w, h;
			m_image->getMipmapData (i, &data, &mipmapsize, &w, &h);
			if (data)
			{
				glCompressedTexImage2D (GL_TEXTURE_2D, i, format, w, h, 0, mipmapsize, data);
			}
		}
	}
}

void GLB::GLBTextureES2::setupSampling()
{
    switch(m_type)
    {
    #ifdef GL_TEXTURE_1D
    case KCL::Texture_1D:
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, getGlTextureFilter(m_minFilter, m_mipFilter));
	    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, getGlTextureFilter(m_magFilter, KCL::TextureFilter_NotApplicable));
	    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, getGlTextureWrap(m_wrapS));
		if (m_anisotropic_filter_enabled && getMaxAnisotropy() >= ANIOSOTROPIC_FILTER_VALUE)
		{
			glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAX_ANISOTROPY_EXT, ANIOSOTROPIC_FILTER_VALUE);
		}
        break;
    #endif
    #ifdef GL_TEXTURE_2D
    case KCL::Texture_2D:
    case KCL::Texture_Video:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, getGlTextureFilter(m_minFilter, m_mipFilter));
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, getGlTextureFilter(m_magFilter, KCL::TextureFilter_NotApplicable));
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, getGlTextureWrap(m_wrapS));
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, getGlTextureWrap(m_wrapT));
		if (m_anisotropic_filter_enabled && getMaxAnisotropy() >= ANIOSOTROPIC_FILTER_VALUE)
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, ANIOSOTROPIC_FILTER_VALUE);
		}
        break;
    #endif
    #ifdef GL_TEXTURE_3D
    case KCL::Texture_3D:
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, getGlTextureFilter(m_minFilter, m_mipFilter));
	    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, getGlTextureFilter(m_magFilter, KCL::TextureFilter_NotApplicable));
	    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, getGlTextureWrap(m_wrapS));
	    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, getGlTextureWrap(m_wrapT));
	    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, getGlTextureWrap(m_wrapU));
		if (m_anisotropic_filter_enabled && getMaxAnisotropy() >= ANIOSOTROPIC_FILTER_VALUE)
		{
			glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_ANISOTROPY_EXT, ANIOSOTROPIC_FILTER_VALUE);
		}
        break;
    #endif
    #ifdef GL_TEXTURE_2D_ARRAY
    case KCL::Texture_Array:
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, getGlTextureFilter(m_minFilter, m_mipFilter));
	    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, getGlTextureFilter(m_magFilter, KCL::TextureFilter_NotApplicable));
	    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, getGlTextureWrap(m_wrapS));
	    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, getGlTextureWrap(m_wrapT));
		if (m_anisotropic_filter_enabled && getMaxAnisotropy() >= ANIOSOTROPIC_FILTER_VALUE)
		{
			glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, ANIOSOTROPIC_FILTER_VALUE);
		}
        break;
    #endif
    #ifdef GL_TEXTURE_CUBE_MAP
    case KCL::Texture_Cube:
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, getGlTextureFilter(m_minFilter, m_mipFilter));
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, getGlTextureFilter(m_magFilter, KCL::TextureFilter_NotApplicable));
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, getGlTextureWrap(m_wrapS));
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, getGlTextureWrap(m_wrapT));
		if (m_anisotropic_filter_enabled && getMaxAnisotropy() >= ANIOSOTROPIC_FILTER_VALUE)
		{
			glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, ANIOSOTROPIC_FILTER_VALUE);
		}
        break;
    #endif
    default:
        break;
    }
}

void GLB::GLBTextureES2::commitASTC()
{
	int offset = 0;
    int width = m_image->getWidth();
	int height = m_image->getHeight();
    for (KCL::uint32 i = 0; i <= m_image->getMipCount(); i++) 
	{
		KCL::uint32 size = 1<<(m_image->getMipCount() - i);
		KCL::uint32 wh = width;
		KCL::uint32 hh = height;
		int x = (wh +8-1) / 8;
		int y = (hh +8-1) / 8;
		size = x * y  * 16;
		glCompressedTexImage2D (GL_TEXTURE_2D, i, GL_COMPRESSED_RGBA_ASTC_8x8_KHR, wh, hh, 0, size, (KCL::uint8*)(m_image->getData()) + offset );
		width /= 2;
		height /= 2;
		width < 1 ? width = 1:width;
		height < 1 ? height = 1:height;
		offset += size;
	}
}

void GLB::GLBTextureES2::commitETC ( int internalformat)
{
	int offset = 0;
	for (KCL::uint32 i = 0; i <= m_image->getMipCount(); i++) 
	{
		KCL::uint32 size = 1<<(m_image->getMipCount() - i);
		KCL::uint32 wh = size;
		size *= size;
		size /= 2;
		size = size<8?8:size;
        glCompressedTexImage2D (GL_TEXTURE_2D, i, internalformat, wh, wh, 0, size, (KCL::uint8*)(m_image->getData()) +offset);
		offset += size;
	}
}

long GLB::GLBTextureES2::commit1D()
{
#ifdef GL_TEXTURE_1D
	bool mipRequired = mipmapRequired(m_mipFilter);

    KCL::uint8 *data = 0;
    KCL::uint32 size = 0;
    m_image->getMipmapData (0, &data, &size);

	glGenTextures (1, &m_textureId);
	glBindTexture (GL_TEXTURE_1D, m_textureId);

    setupSampling();

	GLint internalFormat; 
	GLint format; 
	GLenum type;
	getGlTextureFormatES2(m_image->getFormat(), internalFormat, format, type);
	
    if (type)
	{
		glTexImage1D(GL_TEXTURE_1D, 0, internalFormat, m_image->getWidth(), 0, format, type, m_image->getData());
        generateMipmap();
	}
	else
	{
        if(m_image->getFormat() != KCL::Image_ETC1)
		{
		    GLsizei size = m_image->getWidth() * m_image->getBpp() / 8;
			glCompressedTexImage1D(GL_TEXTURE_1D, 0, internalFormat, (GLsizei)m_image->getWidth(), 0, (GLsizei)m_image->getDataSize(), m_image->getData());
            uploadMipmap(internalFormat);
        }
        else
        {
            commitETC( GL_ETC1_RGB8_OES);
        }
	}

	return GLB_TEXERROR_NO_ERROR;
#else
	return GLB_TEXERROR_NOT_IMPLEMENTED;
#endif
}

long GLB::GLBTextureES2::commit2D()
{
#ifdef GL_TEXTURE_2D
	bool mipRequired = mipmapRequired(m_mipFilter);
    
    KCL::uint8 *data = 0;
    KCL::uint32 size = 0;
    m_image->getMipmapData (0, &data, &size);

	glGenTextures (1, &m_textureId);
	glBindTexture (GL_TEXTURE_2D, m_textureId);

    setupSampling();

	GLint internalFormat; 
	GLint format; 
	GLenum type;
	getGlTextureFormatES2(m_image->getFormat(), internalFormat, format, type);

	if (type)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_image->getWidth(), m_image->getHeight(), 0, format, type, m_image->getData());
        generateMipmap();
	}
	else
	{
        if(mipRequired && (m_image->getFormat() == KCL::Image_RGBA_ASTC_8x8) )
        {
            commitASTC();
        }
        else if(m_image->getFormat() == KCL::Image_ETC1)
        {
            commitETC( GL_ETC1_RGB8_OES);
        }
		else if(m_image->getFormat() == KCL::Image_ETC2_RGB)
        {
            commitETC( GL_COMPRESSED_RGB8_ETC2);
        }
		else if(m_image->getFormat() == KCL::Image_ETC2_RGBA8888)
        {
			commitETC( GL_COMPRESSED_RGBA8_ETC2_EAC);
        }
        else
		{
            GLsizei size = m_image->getWidth() * m_image->getHeight() * m_image->getBpp() / 8;
		    glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_image->getWidth(), m_image->getHeight(), 0, size, m_image->getData());
            uploadMipmap(internalFormat);
        }
	}

	return GLB_TEXERROR_NO_ERROR;
#else
	return GLB_TEXERROR_NOT_IMPLEMENTED;
#endif
}

long GLB::GLBTextureES2::commit3D()
{
	return GLB_TEXERROR_NOT_IMPLEMENTED;
}

long GLB::GLBTextureES2::commitArray()
{
	return GLB_TEXERROR_NOT_IMPLEMENTED;
}

long GLB::GLBTextureES2::commitCube()
{
#ifdef GL_TEXTURE_CUBE_MAP
	if (m_image->getDepth() != 6)
	{
		return GLB_TEXERROR_INVALID_SLICE_COUNT;
	}

	glGenTextures (1, &m_textureId);
	glBindTexture (GL_TEXTURE_CUBE_MAP, m_textureId);

    setupSampling();

	GLint internalFormat; 
	GLint format; 
	GLenum type;
	getGlTextureFormatES2(m_image->getFormat(), internalFormat, format, type);
	if (type)
	{
		for (int i = 0; i < 6; ++i)
		{
			glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, internalFormat, m_image->getWidth(), m_image->getHeight(), 0, format, type, m_image->getData(i));
		}
	}
	else
	{
		KCL::uint32 size = m_image->getWidth() * m_image->getHeight() * m_image->getBpp() / 8;

		for (int i = 0; i < 6; ++i)
		{
			glCompressedTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, m_image->getWidth(), m_image->getHeight(), 0, size, m_image->getData(i));
		}
	}

	return GLB_TEXERROR_NO_ERROR;
#else
	return GLB_TEXERROR_NOT_IMPLEMENTED;
#endif
}

long GLB::GLBTextureES2::commitCubeArray()
{
	return GLB_TEXERROR_NOT_IMPLEMENTED;
}

long GLB::GLBTextureES2::commitVideo()
{
	return GLB_TEXERROR_NOT_IMPLEMENTED;
}

long GLB::GLBTextureES2::release()
{
	if (m_textureId)
	{
        m_isCommitted = false;
		glDeleteTextures(1, &m_textureId);
		m_textureId = 0;
	}

	return GLB_TEXERROR_NO_ERROR;
}


std::map<GLB::samplingDesc, GLB::sampler> samplerMap;

void GLB::GLBTextureES3::resetSamplerMap()
{
	samplerMap.clear();
}


#if defined HAVE_GLES3 || defined __glew_h__

GLB::GLBTextureES3::GLBTextureES3() : GLB::GLBTextureES2()
{
}

GLB::GLBTextureES3::GLBTextureES3(const KCL::Image* image, bool releaseUponCommit) : GLB::GLBTextureES2(image, releaseUponCommit)
{
}

GLB::GLBTextureES3::GLBTextureES3(const KCL::Image* image, KCL::TextureType type, bool releaseUponCommit) : GLB::GLBTextureES2(image, type, releaseUponCommit)
{
}


void GLB::GLBTextureES3::bindSampling(KCL::uint32 slotId)
{
	glBindSampler(slotId, m_samplerIt->second.m_samplerID);
}


void GLB::GLBTextureES3::setupSampling()
{
	float aniosotropy = m_anisotropic_filter_enabled && getMaxAnisotropy() >= ANIOSOTROPIC_FILTER_VALUE ? ANIOSOTROPIC_FILTER_VALUE : 0.0f;
    samplingDesc desc(getGlTextureFilter(m_minFilter, m_mipFilter),
                      getGlTextureFilter(m_magFilter, KCL::TextureFilter_NotApplicable),
                      getGlTextureWrap(m_wrapS),
                      getGlTextureWrap(m_wrapT),
                      getGlTextureWrap(m_wrapU),
					  aniosotropy);

    std::map<samplingDesc, sampler>::iterator itm = samplerMap.find(desc);
    if(itm == samplerMap.end())
    {
        KCL::uint32 id;
        glGenSamplers(1, &id);

        glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, getGlTextureFilter(m_minFilter, m_mipFilter));
        glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, getGlTextureFilter(m_magFilter, KCL::TextureFilter_NotApplicable));
        glSamplerParameteri(id, GL_TEXTURE_WRAP_S, getGlTextureWrap(m_wrapS));
        glSamplerParameteri(id, GL_TEXTURE_WRAP_T, getGlTextureWrap(m_wrapT));
        glSamplerParameteri(id, GL_TEXTURE_WRAP_R, getGlTextureWrap(m_wrapU));	
		if (aniosotropy >= 1.0f)
		{
			glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniosotropy);
		}

        sampler s(id);
        std::pair<std::map<samplingDesc, sampler>::iterator,bool> ret = samplerMap.insert(std::pair<samplingDesc, sampler>(desc, s));
        m_samplerIt = ret.first;
    }
    else
    {
        m_samplerIt = itm;
        itm->second.m_refCount++;
    }
}

long GLB::GLBTextureES3::release()
{
    if(m_isCommitted)
    {
        m_samplerIt->second.m_refCount--;
        if(m_samplerIt->second.m_refCount <= 0)
        {
            glDeleteSamplers(1, &m_samplerIt->second.m_samplerID);
            samplerMap.erase(m_samplerIt);
        }
    }

    return GLBTextureES2::release();
}

GLB::GLBTextureES3::~GLBTextureES3()
{
    release();
}


long GLB::GLBTextureES3::commit2D()
{
#ifdef GL_TEXTURE_2D
	GLint internalFormat; 
	GLint format; 
	GLenum type; //type == 0 if compressed	
	getGlTextureFormatES3(m_image->getFormat(), m_isSRGB, internalFormat, format, type);

	bool mipRequired = mipmapRequired(m_mipFilter);
	int width = m_image->getWidth();
	int height = m_image->getHeight();
	   
	int lods = mipRequired ? KCL::texture_levels(width, height) : 1;

	glGenTextures (1, &m_textureId);
	glBindTexture (GL_TEXTURE_2D, m_textureId);

	setupSampling();

	// ?
    assert( (mipRequired == false && lods == 1) || (mipRequired == true && lods > 1));

	glTexStorage2D( GL_TEXTURE_2D, lods, internalFormat, width, height);


   	KCL::uint32 pbo;
	glGenBuffers( 1, &pbo);
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, pbo);
	glBufferData( GL_PIXEL_UNPACK_BUFFER, m_image->getDataSize(), m_image->getData(), GL_STATIC_DRAW);
	
	if( type) //uncompressed
	{
		glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, 0);

		if( mipRequired)
		{
			glGenerateMipmap( GL_TEXTURE_2D);
		}

	}
	else //compressed
	{		
		if (commitAsCompressedBlocks(m_image->getFormat()))
		{			
			if (mipRequired)
			{
				assert(lods == (m_image->getMipCount() + 1));
			}

			// Dimensions of a block in texels
			KCL::uint32 block_dim_x, block_dim_y, block_dim_z;
			m_image->getBlockDimensions(block_dim_x, block_dim_y, block_dim_z);

			// Size of a block in bits
			KCL::uint32 block_size = m_image->getBlockSize();
			
			KCL::uint32 offset = 0;
			for (KCL::uint32 i = 0; i < lods; i++) 
			{
				// Calculate the dimensions of the mipmap
				KCL::uint32 mipmap_width = width / (1 << i);
				KCL::uint32 mipmap_height = height / (1 << i);
				mipmap_width = mipmap_width ? mipmap_width : 1;
				mipmap_height = mipmap_height ? mipmap_height : 1;

				// Calculate the number of blocks for the mipmap
				KCL::uint32 block_count_x = (mipmap_width + block_dim_x - 1) / block_dim_x;
				KCL::uint32 block_count_y = (mipmap_height + block_dim_y - 1) / block_dim_y;

				// Calculate the compressed data size
				KCL::uint32 data_size = block_count_x * block_count_y * block_size; // in bits
				data_size /= 8; // in bytes

				// Upload the mipmap level
				glCompressedTexSubImage2D (GL_TEXTURE_2D, i, 0, 0, mipmap_width, mipmap_height, internalFormat, data_size, (KCL::uint8*)offset);
				offset += data_size;
			}
		}
		else
		{
			KCL::uint32 num_mipmaps = 1;

			if (mipRequired)
			{
				num_mipmaps = m_image->getMipCount() + 1;
			}


			KCL::uint32 bpp = m_image->getBpp();
		
			// DXT1, DXT5, ETC1, ETC2
			KCL::uint32 min_mipmap_width = 4; 
			KCL::uint32 min_mipmap_height = 4;

			int offset = 0;
			for (KCL::uint32 i = 0; i < num_mipmaps; i++)
			{
				// NOTE: Currently we only support square compressed textures
				KCL::uint32 size = 1 << (m_image->getMipCount() - i);
				KCL::uint32 w = size;
				KCL::uint32 h = size;

				//while data is padded to block size (usually 4x4), the w/h for the call needs to match the actual mip dimensions (can be 2x2, or 1x1 as well)
				KCL::uint32 data_w = w < min_mipmap_width ? min_mipmap_width : w;
				KCL::uint32 data_h = h < min_mipmap_height ? min_mipmap_height : h;
			
				KCL::uint32 data_size = data_w * data_h * bpp; // size in bits			
				data_size /= 8; // size in bytes
	
				glCompressedTexSubImage2D (GL_TEXTURE_2D, i, 0, 0, w, h, internalFormat, data_size, (KCL::uint8*)offset);

				offset += data_size;
			}
		}
	}

	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0);
    glDeleteBuffers( 1, &pbo);

	glBindTexture (GL_TEXTURE_2D, 0);

	return GLB_TEXERROR_NO_ERROR;
#else
	return GLB_TEXERROR_NOT_IMPLEMENTED;
#endif
}


// TODO: upload compressed textures with PBOs 
long GLB::GLBTextureES3::commitArray()
{
#ifdef GL_TEXTURE_2D_ARRAY
	GLint internalFormat; 
	GLint format; 
	GLenum type;  //type == 0 if compressed

	getGlTextureFormatES3(m_image->getFormat(), m_isSRGB, internalFormat, format, type);

	bool mipRequired = mipmapRequired(m_mipFilter);
	int width = m_image->getWidth();
	int height = m_image->getHeight();
	int depth = m_image->getDepth();
	
	int lods = mipRequired ? KCL::texture_levels(width, height) : 1;

    assert( (mipRequired == false && lods == 1) || (mipRequired == true && lods > 1));

	glGenTextures( 1, &m_textureId);
	glBindTexture( GL_TEXTURE_2D_ARRAY, m_textureId);

	setupSampling();

	glTexStorage3D( GL_TEXTURE_2D_ARRAY, lods, internalFormat, width, height, depth);

	if( type) //uncompressed
	{
    	KCL::uint32 pbo;
    	glGenBuffers( 1, &pbo);
    	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, pbo);
	    glBufferData( GL_PIXEL_UNPACK_BUFFER, m_image->getDataSize(), m_image->getData(), GL_STATIC_DRAW);

		for (int i = 0; i < static_cast<int>(m_image->getDepth()); i++)
		{
			size_t offset = m_image->getSlicePitch() * i;
			glTexSubImage3D( GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, m_image->getWidth(), m_image->getHeight(), 1, format, type, (KCL::uint8*)offset);
		}

		if( mipRequired)
		{
			glGenerateMipmap( GL_TEXTURE_2D_ARRAY);
		}

    	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0);
    	glDeleteBuffers( 1, &pbo);
	}
	else //compressed
	{
        // TODO: PBO-based upload
		if (commitAsCompressedBlocks(m_image->getFormat()))
		{			
			// Check if the compressed texture contains enough lod levels
			if (mipRequired)
			{
				assert(lods == (m_image->getMipCount() + 1));
			}

			// Dimensions of a block in texels
			KCL::uint32 block_dim_x, block_dim_y, block_dim_z;
			m_image->getBlockDimensions(block_dim_x, block_dim_y, block_dim_z);

			// Size of a block in bits
			KCL::uint32 block_size = m_image->getBlockSize();

			// Upload the mipmaps			
			KCL::uint32 mipmap_level_offset = 0;
			for (KCL::uint32 i = 0; i < lods; i++) 
			{
				// Calculate the dimensions of the mipmap
				KCL::uint32 mipmap_width = width / (1 << i);
				KCL::uint32 mipmap_height = height / (1 << i);
				mipmap_width = mipmap_width ? mipmap_width : 1;
				mipmap_height = mipmap_height ? mipmap_height : 1;

				// Calculate the number of blocks for the mipmap
				KCL::uint32 block_count_x = (mipmap_width + block_dim_x - 1) / block_dim_x;
				KCL::uint32 block_count_y = (mipmap_height + block_dim_y - 1) / block_dim_y;

				// Calculate the compressed data size
				KCL::uint32 data_size = block_count_x * block_count_y * block_size; // in bits
				data_size /= 8; // in bytes

				// Upload all surfaces for this lod level
				for (KCL::uint32 depth = 0; depth < m_image->getDepth(); depth++)
				{
					glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, i, 0, 0, depth, mipmap_width, mipmap_height, 1, internalFormat, data_size, (KCL::uint8*)m_image->getData(depth) + mipmap_level_offset);
				}
				mipmap_level_offset += data_size;		
			}
		}
		else
		{
			assert(lods == (m_image->getMipCount() + 1));

			int offset = 0;

			for( KCL::uint32 j=0; j<=m_image->getMipCount(); j++)
			{
				KCL::uint32 w = 1 << (m_image->getMipCount() - j);
				KCL::uint32 size = w;

				if( size < 4)
				{
					size = 4;
				}

				size = size * size * 4;
				size /= 8;
			
				for (KCL::uint32 i = 0; i < m_image->getDepth(); i++)
				{			
					glCompressedTexSubImage3D( GL_TEXTURE_2D_ARRAY, j, 0, 0, i, w, w, 1, internalFormat, size, (char*)m_image->getData(i) + offset);
				}

				offset += size ;
			}
		}
	}

	glBindTexture( GL_TEXTURE_2D_ARRAY, 0);

	return GLB_TEXERROR_NO_ERROR;
#else
	return GLB_TEXERROR_NOT_IMPLEMENTED;
#endif
}

long GLB::GLBTextureES3::commitCube()
{
#ifdef GL_TEXTURE_CUBE_MAP
	if (m_image->getDepth() != 6)
	{
		return GLB_TEXERROR_INVALID_SLICE_COUNT;
	}

	GLint internalFormat; 
	GLint format; 
	GLenum type; // type == 0 if compressed	
	getGlTextureFormatES3(m_image->getFormat(), m_isSRGB, internalFormat, format, type);

	bool mipRequired = mipmapRequired(m_mipFilter);
	int width = m_image->getWidth();
	int height = m_image->getHeight();

    int lods = mipRequired ? KCL::texture_levels(width, height) : 1;

	glGenTextures (1, &m_textureId);
	glBindTexture (GL_TEXTURE_CUBE_MAP, m_textureId);

	setupSampling();
	
    glTexStorage2D( GL_TEXTURE_CUBE_MAP, lods, internalFormat, width, height);
	if( type)
	{
		for (int i = 0; i < 6; ++i)
		{
			glTexSubImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, 0, 0, m_image->getWidth(), m_image->getHeight(), format, type, m_image->getData(i));
		}

        if( mipRequired)
		{
			glGenerateMipmap( GL_TEXTURE_CUBE_MAP);
		}
	}
	else
	{			
        // Old code used pre 3.1 does not support compressed cubes with mipmaps so we choose the new path
        if (commitAsCompressedBlocks(m_image->getFormat()) || mipRequired)
		{	
			// Check if the compressed texture contains enough lod levels
			if (mipRequired)
			{
				assert(lods == (m_image->getMipCount() + 1));
			}

			// Dimensions of a block in texels
			KCL::uint32 block_dim_x, block_dim_y, block_dim_z;
			m_image->getBlockDimensions(block_dim_x, block_dim_y, block_dim_z);

			// Size of a block in bits
			KCL::uint32 block_size = m_image->getBlockSize();

			// Upload the mipmaps			
			KCL::uint32 mipmap_level_offset = 0;
			for (KCL::uint32 i = 0; i < lods; i++) 
			{
				// Calculate the dimensions of the mipmap
				KCL::uint32 mipmap_width = width / (1 << i);
				KCL::uint32 mipmap_height = height / (1 << i);
				mipmap_width = mipmap_width ? mipmap_width : 1;
				mipmap_height = mipmap_height ? mipmap_height : 1;

				// Calculate the number of blocks for the mipmap
				KCL::uint32 block_count_x = (mipmap_width + block_dim_x - 1) / block_dim_x;
				KCL::uint32 block_count_y = (mipmap_height + block_dim_y - 1) / block_dim_y;

				// Calculate the compressed data size
				KCL::uint32 data_size = block_count_x * block_count_y * block_size; // in bits
				data_size /= 8; // in bytes					

				// Upload the faces	for this lod level
				for (KCL::uint32 face = 0; face < 6; face++)
				{
					glCompressedTexSubImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, i, 0, 0, mipmap_width, mipmap_height, internalFormat, data_size, (KCL::uint8*)m_image->getData(face) + mipmap_level_offset);							
				}
				mipmap_level_offset += data_size;		
			}
		}
		else
		{
			//mips shall be already stored in the file

			//NOTE: GFXB 3 does not use mips for the cubemaps, even if the files contain it		
			KCL::uint32 size = m_image->getWidth() * m_image->getHeight() * m_image->getBpp();

			size /= 8;

			for (int i = 0; i < 6; ++i)
			{
				glCompressedTexSubImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, 0, 0, m_image->getWidth(), m_image->getHeight(), internalFormat, size, m_image->getData(i));
			}
		}
	}
	glBindTexture (GL_TEXTURE_CUBE_MAP, 0);

	return GLB_TEXERROR_NO_ERROR;
#else
	return GLB_TEXERROR_NOT_IMPLEMENTED;
#endif
}


long GLB::GLBTextureES3::commitCubeArray()
{
#ifdef GL_TEXTURE_CUBE_MAP_ARRAY
	GLint internalFormat; 
	GLint format; 
	GLenum type; // type == 0 if compressed	
	getGlTextureFormatES3(m_image->getFormat(), m_isSRGB, internalFormat, format, type);

	bool mipRequired = mipmapRequired(m_mipFilter);
	int width = m_image->getWidth();
	int height = m_image->getHeight();
	int faces = m_image->getDepth();

    int lods = mipRequired ? KCL::texture_levels(width, height) : 1;

	glGenTextures (1, &m_textureId);
	glBindTexture (GL_TEXTURE_CUBE_MAP_ARRAY, m_textureId);

	setupSampling();

	glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, lods, internalFormat, width, height, faces);
	
	if( type) //uncompressed
	{
		KCL::uint32 pbo = 0;
   		glGenBuffers( 1, &pbo);
   		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, pbo);
	    glBufferData( GL_PIXEL_UNPACK_BUFFER, m_image->getDataSize(), m_image->getData(), GL_STATIC_DRAW);
    	
		for (int i = 0; i < faces; i++)
		{
			size_t offset = m_image->getSlicePitch() * i;
			glTexSubImage3D( GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, i, width, height, 1, format, type, (KCL::uint8*)offset);
		}

		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0);
		glDeleteBuffers( 1, &pbo);

		if( mipRequired)
		{
			glGenerateMipmap( GL_TEXTURE_CUBE_MAP_ARRAY);
		}
	}
	else //compressed
	{
		// Check if the compressed texture contains enough lod levels
		if (mipRequired)
		{
			assert(lods == (m_image->getMipCount() + 1));
		}

		// Dimensions of a block in texels
		KCL::uint32 block_dim_x, block_dim_y, block_dim_z;
		m_image->getBlockDimensions(block_dim_x, block_dim_y, block_dim_z);

		// Size of a block in bits
		KCL::uint32 block_size = m_image->getBlockSize();

		// Upload the mipmaps			
		KCL::uint32 mipmap_level_offset = 0;
		for (KCL::uint32 lod = 0; lod < lods; lod++) 
		{
			// Calculate the dimensions of the mipmap
			KCL::uint32 mipmap_width = width / (1 << lod);
			KCL::uint32 mipmap_height = height / (1 << lod);
			mipmap_width = mipmap_width ? mipmap_width : 1;
			mipmap_height = mipmap_height ? mipmap_height : 1;

			// Calculate the number of blocks for the mipmap
			KCL::uint32 block_count_x = (mipmap_width + block_dim_x - 1) / block_dim_x;
			KCL::uint32 block_count_y = (mipmap_height + block_dim_y - 1) / block_dim_y;

			// Calculate the compressed data size
			KCL::uint32 mipmap_size = block_count_x * block_count_y * block_size; // in bits
			mipmap_size /= 8; // in bytes

			for (KCL::uint32 face = 0; face < faces; face++)
			{		
				glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, lod, 0, 0, face, mipmap_width, mipmap_height, 1, internalFormat, mipmap_size, (KCL::uint8*)m_image->getData(face) + mipmap_level_offset);
			}
			mipmap_level_offset += mipmap_size;				
		}
	}
	return GLB_TEXERROR_NO_ERROR;
#else
	return GLB_TEXERROR_NOT_IMPLEMENTED;
#endif	
}

long GLB::GLBTextureES3::commit3D()
{
#ifdef GL_TEXTURE_3D
	GLint internalFormat; 
	GLint format; 
	GLenum type;

	getGlTextureFormatES3(m_image->getFormat(), m_isSRGB, internalFormat, format, type);

	bool mipRequired = mipmapRequired(m_mipFilter);
	int width = m_image->getWidth();
	int height = m_image->getHeight();

    //type == 0 if compressed
	int lods = mipRequired ? KCL::texture_levels(width, height) : 1;

	glGenTextures (1, &m_textureId);
	glBindTexture (GL_TEXTURE_3D, m_textureId);

	setupSampling();

	glTexStorage3D( GL_TEXTURE_3D, lods, internalFormat, m_image->getWidth(), m_image->getHeight(), m_image->getDepth());

    KCL::uint32 pbo;
	glGenBuffers( 1, &pbo);
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData( GL_PIXEL_UNPACK_BUFFER, m_image->getDataSize(), m_image->getData(), GL_STATIC_DRAW);

	if (type) //uncompressed
	{
		for (KCL::uint32 i = 0; i < m_image->getDepth(); i++)
		{
            size_t offset = m_image->getSlicePitch() * i;
			glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i, m_image->getWidth(), m_image->getHeight(), 1, format, type, (KCL::uint8*)offset);
		}

		if (mipRequired)
		{
			glGenerateMipmap( GL_TEXTURE_3D);
		}
	}
	else //compressed
	{
        //TODO: upload with PBOs

		glBindTexture (GL_TEXTURE_3D, 0);

#if 0
        assert(lods == (m_image->getMipCount() + 1));

		if( internalFormat == GL_COMPRESSED_RGB8_ETC2)
		{
			int offset = 0;
			uint32 d = m_image->getDepth();

			for( uint32 j=0; j<=m_image->getMipCount(); j++)
			{
				KCL::uint32 w = 1 << (m_image->getMipCount() - j);
				KCL::uint32 size = w;

				if( size < 4)
				{
					size = 4;
				}

				size = size * size * 4;
				size /= 8;

				glCompressedTexImage3D( GL_TEXTURE_3D, j, internalFormat, w, w, d, 0, size * d, NULL);

				for (KCL::uint32 i = 0; i < d; i++)
				{
					glCompressedTexSubImage3D( GL_TEXTURE_3D, j, 0, 0, i, w, w, 1, internalFormat, size, (char*)m_image->getData(i) + offset);
				}

				offset += size;

				if( d >= 2)
				{
					d /= 2;
				}
			}
		}
		else
#endif
		{
			return GLB_TEXERROR_NOT_IMPLEMENTED;
		}
	}

    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0);
    glDeleteBuffers( 1, &pbo);

	glBindTexture (GL_TEXTURE_3D, 0);

	return GLB_TEXERROR_NO_ERROR;
#else
	return GLB_TEXERROR_NOT_IMPLEMENTED;
#endif
}


long GLB::GLBTextureES3::commitVideo()
{
	return GLB_TEXERROR_NOT_IMPLEMENTED;

#ifdef GL_TEXTURE_2D
	GLint internalFormat = (m_isSRGB)?GL_SRGB8_ALPHA8:GL_RGBA;


	glGenTextures (1, &m_textureId);
	glBindTexture (GL_TEXTURE_2D, m_textureId);

    setupSampling();

	glTexStorage2D( GL_TEXTURE_2D, 1, internalFormat, m_video->width, m_video->height);

	return GLB_TEXERROR_NO_ERROR;
#else
	return GLB_TEXERROR_NOT_IMPLEMENTED;
#endif
}

bool GLB::GLBTextureES3::commitAsCompressedBlocks(KCL::ImageFormat format)
{
	switch(format)
	{
	case KCL::Image_DXT1:
	case KCL::Image_DXT1_RGBA:
		return false;

	case KCL::Image_DXT5:
		return true;

	case KCL::Image_ETC2_RGB:
	case KCL::Image_ETC2_RGBA8888:		
		return true;

	case KCL::Image_RGBA_ASTC_4x4:
	case KCL::Image_RGBA_ASTC_5x5:
	case KCL::Image_RGBA_ASTC_8x8:
		return true;

	default:
		return false;
	}
}

#endif // end of #if defined HAVE_GLES3 || defined __glew_h__


GLB::GLBTexture* GLB::GLBTextureFactory::CreateTexture(const KCL::Image* img, bool releaseUponCommit)
{
	bool use_samplers = false;

    if(GLB::g_extension && GLB::g_extension->hasFeature(GLB::GLBFEATURE_sampler_object))
    {
        use_samplers  = true;
    }

#if defined HAVE_GLES3 || defined __glew_h__
	if( use_samplers)
	{
        return new GLB::GLBTextureES3(img, releaseUponCommit);
	}
	else
#endif
	{
        return new GLB::GLBTextureES2(img, releaseUponCommit);
	}
}