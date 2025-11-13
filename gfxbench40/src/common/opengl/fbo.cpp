/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_os.h"

#include "stdc.h"
#include "fbo.h"
#include "texture.h"
#include "opengl/ext.h"
#include "kcl_image.h"

#include "glb_discard_functions.h"

#ifndef GL_FRAMEBUFFER_INCOMPLETE_FORMATS
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS	0x8CDA
#endif

#ifndef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
#endif

#ifndef GL_DEPTH_COMPONENT24_OES
#define GL_DEPTH_COMPONENT24_OES	GL_DEPTH_COMPONENT24
#endif

typedef void (GFXB_APIENTRY *_PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
typedef void (GFXB_APIENTRY *_PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

_PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC _glFramebufferTexture2DMultisampleEXT = 0;
_PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC _glRenderbufferStorageMultisampleEXT = 0;


using namespace GLB;

GLB::FBO* FBO::m_lastBound = 0;
GLB::FBO* FBO::m_originalGlobal = 0;
GLB::FBO* FBO::m_currentGlobal = 0;


FBO::FBO() :
	m_name (0),
	m_texture (0),
	m_depth_renderbuffer (0),
	m_depth_texture (0),
	m_width (0),
	m_height (0)
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&m_name);

	////GLint params[4]; 
	////glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, params);
	////glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER, params);

	////glBindTexture(GL_TEXTURE_2D, (GLuint)params[0]);
	////glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, params);
}


FBO::FBO(KCL::uint32 width, KCL::uint32 height, int samples, FBO_COLORMODE color_mode, FBO_DEPTHMODE depth_mode, const char *debug_label) :
	m_name (0),
	m_texture (0),
	m_depth_renderbuffer (0),
	m_depth_texture (0),
	m_width (width),
	m_height (height)
{
	m_debug_label = debug_label;
	using namespace KCL;

	if( samples && (!_glFramebufferTexture2DMultisampleEXT || !_glRenderbufferStorageMultisampleEXT) )
	{
		Destroy();
		throw "Multisampling not supported.";
	}

	KCL::uint32 clear_mask = 0;

	glGenFramebuffers (1, &m_name);
	FBO::bind( this);


	if( color_mode)
	{
		clear_mask |= GL_COLOR_BUFFER_BIT;
		Image2D *image = new GLB::Image2D;
		image->setName(m_debug_label.c_str());

		switch( color_mode)
		{
		case RGB565_Linear:
			{
				image->setDimensions (width, height, Image_RGB565);

				image->commit ();
				m_texture = new Texture2D (image);
				
				m_texture->setFiltering (Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
				break;
			}
		case RGB888_Linear:
			{
				image->setDimensions (width, height, Image_RGB888);

				image->commit ();
				m_texture = new Texture2D (image);

				m_texture->setFiltering (Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
				break;
			}
		case RGB888_MipMap:
			{
				image->setDimensions (width, height, Image_RGB888);
				image->EnableMipmap();

				image->commit ();
				m_texture = new Texture2D (image);

				m_texture->setFiltering (Texture::FILTER_LINEAR, Texture::FILTER_LINEAR);
				break;
			}
        case RGB888_Nearest:
            {
                image->setDimensions(width, height, Image_RGB888);

                image->commit();
                m_texture = new Texture2D(image);

                m_texture->setFiltering(Texture::FILTER_BASE_LEVEL, Texture::FILTER_NEAREST);
                break;
            }
		case RGB565_Nearest:
			{
				image->setDimensions (width, height, Image_RGB565);

				image->commit ();
				m_texture = new Texture2D (image);

				m_texture->setFiltering (Texture::FILTER_BASE_LEVEL, Texture::FILTER_NEAREST);
				break;
			}
		case RGBA8888_Linear:
			{
				image->setDimensions (width, height, Image_RGBA8888);

				image->commit ();
				m_texture = new Texture2D (image);

				m_texture->setFiltering (Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
				break;
			}
		case RGBA8888_Nearest:
			{
				image->setDimensions (width, height, Image_RGBA8888);

				image->commit ();
				m_texture = new Texture2D (image);

				m_texture->setFiltering (Texture::FILTER_BASE_LEVEL, Texture::FILTER_NEAREST);
				break;
			}
		case RGBA5551_Linear:
			{
				image->setDimensions (width, height, Image_RGBA5551);

				image->commit ();
				m_texture = new Texture2D (image);

				m_texture->setFiltering (Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
				break;
			}
		}

		m_texture->setWrapping (Texture::WRAP_CLAMP, Texture::WRAP_CLAMP);
		m_texture->bind ();

		if( samples)
		{
			_glFramebufferTexture2DMultisampleEXT (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image->getId(), 0, samples);
		}
		else
		{
			glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image->getId(), 0);
		}
	}

	if( depth_mode)
	{
		clear_mask |= GL_DEPTH_BUFFER_BIT;

		switch( depth_mode)
		{
		case DEPTH_16_RB:
		case DEPTH_24_RB:
			{
				uint32 gl_depth;
				
				if( depth_mode == DEPTH_16_RB)
				{
					gl_depth = GL_DEPTH_COMPONENT16;
				}
				else
				{
					gl_depth = GL_DEPTH_COMPONENT24_OES;
				}

				glGenRenderbuffers (1, &m_depth_renderbuffer);
				glBindRenderbuffer (GL_RENDERBUFFER, m_depth_renderbuffer);
				if( samples)
				{
					_glRenderbufferStorageMultisampleEXT (GL_RENDERBUFFER, samples, gl_depth, width, height);
				}
				else
				{
					glRenderbufferStorage (GL_RENDERBUFFER, gl_depth, width, height);
				}
				glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_renderbuffer);
				glBindRenderbuffer (GL_RENDERBUFFER, 0);
				break;
			}
		case DEPTH_16_T:
			{
				if( !g_extension->hasExtension( GLBEXT_depth_texture))
				{
					Destroy();
					throw "GLBEXT_depth_texture extension not available.";
				}

				glGenTextures (1, &m_depth_texture);
				glBindTexture (GL_TEXTURE_2D, m_depth_texture);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
				glFramebufferTexture2D (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, 0);
				glBindTexture (GL_TEXTURE_2D, 0);
				break;
			}
		case DEPTH_None:
			{
				break;
			}
		}
	}
	
	KCL::int32 status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
	glClear( clear_mask);

	FBO::bind(0);

	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		INFO("framebuffer(%d = %d x %d) color_mode = %d depth_mode = %d, status=%04X\n", m_name, m_width, m_height, color_mode, depth_mode, status);
	}

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		Destroy();
		throw status;
	}
}


FBO::~FBO ()
{
	Destroy();
}


void FBO::InvalidateLastBound()
{
	m_lastBound = 0;
}


void FBO::SetGlobalFBO( FBO *fbo)
{
	InvalidateLastBound();
	if( !fbo)
	{
		fbo = m_originalGlobal;
	}
	m_currentGlobal = fbo;
}


FBO* FBO::GetGlobalFBO()
{
	return m_currentGlobal;
}


void FBO::CreateGlobalFBO()
{
	m_originalGlobal = new GLB::FBO;
	m_currentGlobal = m_originalGlobal;
	INFO("CreateGLobal: %p", m_originalGlobal);
}


void FBO::DeleteGlobalFBO()
{
	INFO("DeleteGLobal: %p", m_originalGlobal);
	delete m_originalGlobal;
}


void FBO::bind( FBO *fbo)
{
	FBO *new_fbo = fbo;

	if( fbo == 0)
	{
		new_fbo = m_currentGlobal;
	}

	//if (m_lastBound != new_fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, new_fbo->m_name);
        m_lastBound = new_fbo;	
	}

#if 0
	GLint bound = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bound);
	if (bound != m_lastBound->getName())
	{
		INFO("BUG: m_lastBound is out-of sync; its value is %d, while the actual bound FBO is %d\n", m_lastBound, bound);
	}
#endif

}


void FBO::ResetInternalState()
{
	InvalidateLastBound();
	bind( 0);
}


void CheckFBOStatus (const char *name)
{
	GLenum error;
	char buf[1024];

	error = glCheckFramebufferStatus (GL_FRAMEBUFFER);
	if (error != GL_FRAMEBUFFER_COMPLETE) {
		switch (error) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			sprintf (buf, "%s: 0x%04x (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)", name, error);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			sprintf (buf, "%s: 0x%04x (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)", name, error);
			break;
#if !defined HAVE_GLES3
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
			sprintf (buf, "%s: 0x%04x (GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)", name, error);
			break;
#endif
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
			sprintf (buf, "%s: 0x%04x (GL_FRAMEBUFFER_INCOMPLETE_FORMATS)", name, error);
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			sprintf (buf, "%s: 0x%04x (GL_FRAMEBUFFER_UNSUPPORTED)", name, error);
			break;
		default:
			sprintf (buf, "%s: 0x%04x (unknown error)", name, error);
			break;
		}
	}
}


void checkError (const char *name)
{
	GLenum error = glGetError ();
	char buf[1024];
	if (error != GL_NO_ERROR)
	{
		switch (error)
	{
		case GL_INVALID_ENUM:
			sprintf (buf, "%s: 0x%04x (GL_INVALID_ENUM)", name, error);
			break;
		case GL_INVALID_VALUE:
			sprintf (buf, "%s: 0x%04x (GL_INVALID_VALUE)", name, error);
			break;
		case GL_INVALID_OPERATION:
			sprintf (buf, "%s: 0x%04x (GL_INVALID_OPERATION)", name, error);
			break;
		case GL_OUT_OF_MEMORY:
			sprintf (buf, "%s: 0x%04x (GL_OUT_OF_MEMORY)", name, error);
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			sprintf (buf, "%s: 0x%04x (GL_INVALID_FRAMEBUFFER_OPERATION)", name, error);
			break;
		default:
			sprintf (buf, "%s: 0x%04x (unknown error)", name, error);
			break;
		}
		INFO("GLError %s", buf);
	}
}


KCL::uint32 FBO::getTextureName () const
{
	assert(m_texture != 0 && m_texture->getImage() != 0);
	return m_texture->getImage()->getId();
}


/*!Do a screenshot about the current FBO
*/
KCL::uint32 FBO::GetScreenshotImage(KCL::Image& img)
{
	FBO* fbo = GetGlobalFBO();
	bind(fbo);

	GLint params[4] = { 0, 0, 0, 0 };
	params[2] = fbo->m_width;
	params[3] = fbo->m_height;
	if ((fbo->m_width == 0) || (fbo->m_height == 0))
	{
		glGetIntegerv(GL_VIEWPORT, params);
	}

	img.Allocate2D(params[2], params[3], KCL::Image_RGBA8888);

	while (glGetError()!=GL_NO_ERROR);

	glReadPixels(params[0], params[1], params[2], params[3], GL_RGBA, GL_UNSIGNED_BYTE, img.getData());
	if (glGetError()!=GL_NO_ERROR)
	{
		return 1;
	}

	img.flipY();
	return 0;
}


void FBO::Destroy()
{
	// don't delete framebuffer for global FBO. HACK: IOS
	if (this != m_originalGlobal)
	{
		if( m_name)
		{
			glDeleteFramebuffers( 1, &m_name);
			m_name = 0;
		}
		if( m_depth_renderbuffer)
		{
			glDeleteRenderbuffers( 1, &m_depth_renderbuffer);
			m_depth_renderbuffer = 0;
		}
		if( m_depth_texture)
		{
			glDeleteTextures( 1, &m_depth_texture);
			m_depth_texture = 0;
		}
	}
	if( m_lastBound == this)
    {
        InvalidateLastBound();
    }

	if (m_texture != NULL)
	{
		delete m_texture;
		m_texture = NULL;
	}
}


void FBO::InvalidateGlobalDepthAttachment()
{
#if defined HAVE_GLES3 || defined HAVE_GLEW

	if (m_currentGlobal == m_originalGlobal)
	{
		const GLenum e[] = { GL_DEPTH, GL_STENCIL };
		glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, e);
	}
	else
	{
		const GLenum e[] = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
		glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, e);
	}
	
#endif
}

