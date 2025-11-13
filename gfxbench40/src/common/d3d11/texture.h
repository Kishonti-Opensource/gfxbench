/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file texture.h
	Definition of class GLB::Texture and derived classes GLB::Texture2D and GLB::TextureCubemap.
*/
#ifndef TEXTURE_H
#define TEXTURE_H

#include <kcl_math3d.h>

#include "platform.h"
#include "dxb_image.h"
#include "DXUtils.h"


namespace GLB
{

///Holds an OpenGL ES texture object.
class Texture
{
	friend class M3GBufferReader;
public:
	typedef enum _Constants {
	 	FILTER_BASE_LEVEL	= 208,
	 	FILTER_LINEAR		= 209,
	 	FILTER_NEAREST		= 210,
	 	FUNC_ADD 			= 224,
	 	FUNC_BLEND			= 225,
	 	FUNC_DECAL			= 226,
	 	FUNC_MODULATE		= 227,
	 	FUNC_REPLACE		= 228,
	 	WRAP_CLAMP			= 240,
	 	WRAP_REPEAT			= 241
	} Constants;

	Texture ();
	virtual ~Texture ();

	void setTextureType (KCL::uint8 type)
	{
		m_textureType = type;
	}

	KCL::uint8 getTextureType () const
	{
		return m_textureType;
	}

	void setFiltering (KCL::uint8 levelFilter, KCL::uint8 imageFilter);
	void setWrapping (KCL::uint8 wrapS, KCL::uint8 wrapT);
	//void setBlending(KCL::uint8 b) {	m_blending = b;	}

	virtual void bind () = 0;

protected:
	mutable bool m_paramsChanged;
	void setMinMagFilter (KCL::enum_t minFilter, KCL::enum_t magFilter);

public:
	KCL::Vector4D	m_blendColor;

	KCL::uint8		m_blending;
	KCL::uint8		m_levelFilter;
	KCL::uint8		m_imageFilter;
	KCL::uint8		m_textureType;
	KCL::enum_t		m_wrapS;
	KCL::enum_t		m_wrapT;
	KCL::enum_t		m_minFilter;
	KCL::enum_t		m_magFilter;
	KCL::Vector2D	m_scale;
};

///Handles 2D textures.
class Texture2D : public Texture
{
	friend class M3GBufferReader;
	friend class M3GLoader;
public:

	enum
	{
		COLOR = 0,
		LIGHT,
		TRANSPARENCY,
		BUMP,
		SHADOW,
		ALPHA
	};

	Texture2D (DXB::Image2D *image);
	~Texture2D ();

	virtual void bind ();

	void setImage (DXB::Image2D *image, bool addReference);
	void initMipMaps(KCL::enum_t format);
	DXB::Image2D *getImage() const { return m_image; }

protected:
	Texture2D ();

	DXB::Image2D *m_image;
	bool	m_referenced;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
};

///Handles cube map textures.
class TextureCube : public Texture
{
public:
	enum
	{
		ENVMAP,
		RADMAP
	};

	TextureCube (DXB::ImageCube *image);
	~TextureCube ();
	virtual void bind ();
	void setImage (DXB::ImageCube *image, bool addReference);

protected:
	TextureCube ();

	DXB::ImageCube	*m_image;
	bool		m_referenced;
};

}

#endif
