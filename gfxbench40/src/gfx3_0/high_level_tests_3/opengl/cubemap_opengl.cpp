/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "cubemap.h"

#include "opengl/glb_image.h"
#include "platform.h"
#include "misc2.h"//savebmp
#include "opengl/fbo.h"
#include "opengl/glb_texture.h"
#include "ng/log.h"

#include <cassert>

using namespace GLB;


CubeEnvMap::CubeEnvMap( int size) : m_id( 0)
{
}


CubeEnvMap* CubeEnvMap::Load( KCL::uint32 idx, const char* path, bool mipmap )
{
	char name[1024] = {0};
	sprintf(name, "%senvmap%03d", path, idx);

    KCL::uint32 flags = KCL::TC_Clamp | KCL::TC_Commit;
    if(!mipmap)
    {
        flags |= KCL::TC_NoMipmap;
    }
    GLB::GLBTextureFactory f;
	KCL::Texture* tex = f.CreateAndSetup( KCL::Texture_Cube, name, flags);

	if( tex)
	{
		CubeEnvMap *cem = new CubeEnvMap( -1);
		cem->m_texture = tex;

		return cem;
	}

	return 0;
}


void CubeEnvMap::Save( KCL::uint32 idx, KCL::uint32 target, KCL::uint32 size, const char* path )
{
	char filename[256];
	sprintf(filename, "%senvmap%03d_%x.png", path, idx, target);

	unsigned char* m = new unsigned char[size * size * 4];
	glReadPixels(0, 0, size, size, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m);

	convertRGBAtoBGR(m, size*size);
	savePng( filename, size, size, m, 1);
	delete[] m;
}


CubeEnvMap::~CubeEnvMap()
{
	if( m_id)
	{
		glDeleteTextures(1, &m_id);
	}
	delete m_texture;
}


ParaboloidEnvMap::ParaboloidEnvMap( int size) : m_id( 0)
{
}


ParaboloidEnvMap* ParaboloidEnvMap::Load( KCL::uint32 idx, const char* path, bool mipmap )
{
	return 0;
}


void ParaboloidEnvMap::Save( KCL::uint32 idx, KCL::uint32 target, KCL::uint32 size, const char* path )
{
	char filename[256];
	sprintf(filename, "%senvmap%03d_%x.png", path, idx, target);

	unsigned char* m = new unsigned char[size * size * 4];
	glReadPixels(0, 0, size, size, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m);

	convertRGBAtoBGR(m, size*size);
	savePng( filename, size, size, m, 1);
	delete[] m;
}


ParaboloidEnvMap::~ParaboloidEnvMap()
{
	if( m_id)
	{
		glDeleteTextures(1, &m_id);
	}
	delete m_texture;
}


FboEnvMap::FboEnvMap(int cubemapSize) : m_cubemapSize(cubemapSize)
{
	glGenFramebuffers( 1, &m_id );
	glGenRenderbuffers(1, &m_rboid);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rboid);
	glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_cubemapSize, m_cubemapSize);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

FboEnvMap::~FboEnvMap()
{
	glDeleteRenderbuffers(1, &m_rboid);
	glDeleteFramebuffers(1, &m_id );
}
	
void FboEnvMap::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}
	
void FboEnvMap::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO::GetGlobalFBO()->getName() );
}
	
void FboEnvMap::AttachCubemap(CubeEnvMap *const cubemap, size_t face)
{
	assert(face < 6);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboid);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, cubemap->m_id, 0);
}

void FboEnvMap::AttachParaboloid(ParaboloidEnvMap *const paraboloidmap, size_t face)
{
#if defined HAVE_GLES3 || defined HAVE_GLEW
	assert(face < 2);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboid);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, paraboloidmap->m_id, 0, face);
#else
	assert(0);
#endif
}

void FboEnvMap::DetachCubemap(size_t face)
{
	assert(face < 6);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, 0, 0);
}

void FboEnvMap::DetachParaboloid(size_t face)
{
#if defined HAVE_GLES3 || defined HAVE_GLEW
	assert(face < 2);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0, face);
#else
	assert(0);
#endif
}

CubeEnvMap* FboEnvMap::CreateCubeEnvMapRGBA()
{
	CubeEnvMap *result = new CubeEnvMap(m_cubemapSize);

	glGenTextures(1, &result->m_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, result->m_id);

	for(int i=0; i<6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA, m_cubemapSize, m_cubemapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return result;
}


CubeEnvMap* FboEnvMap::CreateCubeEnvMapWithFormat(KCL::uint32 texture_format)
{
#if defined HAVE_GLES3 || defined HAVE_GLEW
	CubeEnvMap *result = new CubeEnvMap(m_cubemapSize);

	KCL::uint32 m_uiMipMapCount = 1;

	KCL::uint32 kk = m_cubemapSize;
	while( kk > 1)
	{
		m_uiMipMapCount++;
		kk /= 2;
	}

	glGenTextures(1, &result->m_id);

	glBindTexture(GL_TEXTURE_CUBE_MAP, result->m_id);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, m_uiMipMapCount);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, m_uiMipMapCount, texture_format, m_cubemapSize, m_cubemapSize);

#ifdef GL_TEXTURE_CUBE_MAP_SEAMLESS
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef GL_TEXTURE_WRAP_R
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return result;
#endif

	return nullptr;
}

ParaboloidEnvMap* FboEnvMap::CreateParaboloidEnvMapWithFormat(KCL::uint32 texture_format)
{
#if defined HAVE_GLES3 || defined HAVE_GLEW
    ParaboloidEnvMap *result = new ParaboloidEnvMap(m_cubemapSize);

	KCL::uint32 m_uiMipMapCount = 1;

	KCL::uint32 kk = m_cubemapSize;
	while( kk > 1)
	{
		m_uiMipMapCount++;
		kk /= 2;
	}

	glGenTextures(1, &result->m_id);

	glBindTexture(GL_TEXTURE_2D_ARRAY, result->m_id);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, m_uiMipMapCount);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, m_uiMipMapCount, texture_format, m_cubemapSize, m_cubemapSize, 2);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef GL_TEXTURE_WRAP_R
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	return result;
#endif
	return nullptr;
}


bool GenerateNormalisationCubeMap( KCL::uint32 &texture_object, KCL::uint32 size)
{
	return true;
}
