/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <cassert>
#include "kcl_texture.h"

KCL::ImageFormat KCL::Texture::decodeSource = KCL::ImageTypeAny;
KCL::ImageFormat KCL::Texture::decodeTarget = KCL::ImageTypeAny;

KCL::Texture::Texture() :
    m_video(NULL),
    m_image(NULL),
	m_type(KCL::Texture_Unknown),
	m_mipLevels(0),
    m_minFilter(TextureFilter_Linear),
    m_magFilter(TextureFilter_Linear),
    m_mipFilter(TextureFilter_Linear),
	m_wrapS(TextureWrap_Repeat),
	m_wrapT(TextureWrap_Repeat),
    m_wrapU(TextureWrap_Repeat),
    m_scale(1, 1, 1),
    m_releaseUponCommit(false),
    m_isCommitted(false),
	m_isSRGB(false),
    m_sRGB_values(false),
	m_anisotropic_filter_enabled(false)
{
}

KCL::Texture::Texture(const Image* image, bool releaseUponCommit) :
    m_name(image->getName()),
    m_video(NULL),
    m_image(image),
	m_type(getType(image->getType())),
	m_mipLevels(0),
    m_minFilter(TextureFilter_Linear),
    m_magFilter(TextureFilter_Linear),
    m_mipFilter(TextureFilter_Linear),
	m_wrapS(TextureWrap_Repeat),
	m_wrapT(TextureWrap_Repeat),
    m_wrapU(TextureWrap_Repeat),
    m_scale(1, 1, 1),
    m_releaseUponCommit(releaseUponCommit),
    m_isCommitted(false),
    m_width(m_image->getWidth()),
    m_height(m_image->getHeight()),
	m_isSRGB(false),
    m_sRGB_values(false),
	m_anisotropic_filter_enabled(false)
{
}

KCL::Texture::Texture(_ogg_decoder* video, bool releaseUponCommit) :
    m_name(video->getName()),
    m_video(video),
    m_image(NULL),
	m_type(Texture_Video),
    m_mipLevels(0),
    m_minFilter(TextureFilter_Linear),
    m_magFilter(TextureFilter_Linear),
    m_mipFilter(TextureFilter_Linear),
	m_wrapS(TextureWrap_Repeat),
	m_wrapT(TextureWrap_Repeat),
    m_wrapU(TextureWrap_Repeat),
    m_scale(1, 1, 1),
    m_releaseUponCommit(releaseUponCommit),
    m_isCommitted(false),
    m_width(video->width),
    m_height(video->height),
	m_isSRGB(false),
    m_sRGB_values(false),
	m_anisotropic_filter_enabled(false) 
{
}

KCL::Texture::Texture(const KCL::Image *image, KCL::TextureType type, bool releaseUponCommit) :
    m_name(image->getName()),
    m_video(NULL),
    m_image(image),
	m_type(type),
    m_mipLevels(0),
    m_minFilter(TextureFilter_Linear),
    m_magFilter(TextureFilter_Linear),
    m_mipFilter(TextureFilter_Linear),
	m_wrapS(TextureWrap_Repeat),
	m_wrapT(TextureWrap_Repeat),
    m_wrapU(TextureWrap_Repeat),
    m_scale(1, 1, 1),
    m_releaseUponCommit(releaseUponCommit),
    m_isCommitted(false),
    m_width(m_image->getWidth()),
    m_height(m_image->getHeight()),
	m_isSRGB(false),
    m_sRGB_values(false),
	m_anisotropic_filter_enabled(false)
{
}

KCL::Texture::~Texture(void)
{
    //NOTE: if releaseUponCommit is not set, the app MUST release the image outside
}

KCL::TextureType KCL::Texture::getType(ImageType imageType)
{
	switch (imageType)
	{
	case KCL::Image_1D:
		return KCL::Texture_1D;

	case KCL::Image_2D:
		return KCL::Texture_2D;

	case KCL::Image_3D:
		return KCL::Texture_3D;

	case KCL::Image_Array:
		return KCL::Texture_Array;

	case KCL::Image_Cube:
		return KCL::Texture_Cube;

	case KCL::Image_CubeArray:
		return KCL::Texture_CubeArray;

	default:
		return KCL::Texture_Unknown;
	}
}

void KCL::Texture::setImage(const KCL::Image *image)
{
	//KCL::TextureType type = image ? getType(image->getType()) : KCL::Texture_Unknown;
	setImage(image, m_type);
}

void KCL::Texture::setImage(const KCL::Image *image, KCL::TextureType type)
{
	release();
	m_image = image;
	m_type = type;
    m_name = image->getName();
    m_width = m_image->getWidth();
    m_height = m_image->getHeight();
}

long KCL::Texture::setVideoTime(float time)
{
	if (!m_video)
	{
		return 0;
	}

	if (m_videoTime == time)
	{
		return 0;
	}

	m_video->Play(time);

	return m_video->m_need_refresh > 0 ? 1 : 0;
}

KCL::Texture* KCL::TextureFactory::CreateAndSetup(TextureType type, const char* file, KCL::uint32 createFlags)
{
    bool flip = createFlags & TC_Flip;
    bool nearest = createFlags & TC_NearestFilter;
    bool nomipmap = createFlags & TC_NoMipmap;
    bool clamp = createFlags & TC_Clamp;
	bool isSRGB = createFlags & TC_IsSRGB ;
    //bool commit = createFlags & TC_Commit;
    bool commit = true;

    Image* img = new Image();
    bool res = false;
    if(type == Texture_2D)
    {
        res = img->load(file, flip);
    }
    else if(type == Texture_3D)
    {
        res = img->load3D(file);
    }
    else if(type == Texture_Array)
    {
        res = img->loadArray(file);
    }
    else if(type == Texture_Cube)
    {
        res = img->loadCube(file);
    }
	else if(type == Texture_CubeArray)
	{
		res = img->loadCubeArray(file);
	}
    else
    {
        assert(0); //not implemented
    }

    if (res)
    {
        Texture* tex = CreateTexture(img, true);
        tex->setMinFilter(nearest ? KCL::TextureFilter_Nearest : KCL::TextureFilter_Linear);
	    tex->setMagFilter(nearest ? KCL::TextureFilter_Nearest : KCL::TextureFilter_Linear);
        tex->setMipFilter(nomipmap ? KCL::TextureFilter_NotApplicable : KCL::TextureFilter_Linear);
        tex->setWrapS(clamp ? KCL::TextureWrap_Clamp : KCL::TextureWrap_Repeat);
	    tex->setWrapT(clamp ? KCL::TextureWrap_Clamp : KCL::TextureWrap_Repeat);
        tex->setWrapU(clamp ? KCL::TextureWrap_Clamp : KCL::TextureWrap_Repeat);
		tex->setSRGB(isSRGB);
		tex->setAnisotropicFilterEnabled(createFlags & TC_AnisotropicFilter);
        if(commit)
        {
            tex->commit();
        }

        return tex;
    }
    else
    {
		delete img;
        return NULL;
    }
}
