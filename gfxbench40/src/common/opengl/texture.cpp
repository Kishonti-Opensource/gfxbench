/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file texture.cpp
	Implementation of texture support functions.
*/
#include "texture.h"
#include "opengl/ext.h"

using namespace KCL;
using namespace GLB;

Texture::Texture ()
{
	m_scale = Vector2D (1, 1);
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


void Texture::setMinMagFilter (GLenum minFilter, GLenum magFilter)
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


Texture2D::Texture2D (GLB::Image2D *image)
{
	m_image = image;
	m_referenced = true;
	m_textureType = COLOR;
}


Texture2D::Texture2D ()
{
	m_image = NULL;
	m_referenced = false;
	m_textureType = COLOR;
}


void Texture2D::setImage (GLB::Image2D *image, bool addReference)
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


void Texture2D::bind () const
{
	glBindTexture (GL_TEXTURE_2D, m_image->getId ());
	if (m_paramsChanged)
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFilter);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFilter);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapS);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapT);
		m_paramsChanged = false;
	}
}



TextureCube::TextureCube (GLB::ImageCube *image)
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


void TextureCube::bind () const
{
	glBindTexture (GL_TEXTURE_CUBE_MAP, m_image->getId());
	if (m_paramsChanged)
	{
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, m_minFilter);
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, m_magFilter);
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, m_wrapS);
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, m_wrapT);
		m_paramsChanged = false;
	}
}

void TextureCube::setImage (GLB::ImageCube *image, bool addReference)
{
	if (m_referenced) delete m_image;
	m_referenced = addReference;
	m_image = image;
}
