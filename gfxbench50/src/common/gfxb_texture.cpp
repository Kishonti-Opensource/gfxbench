/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_texture.h"
#include "gfxb_factories.h"
#include <cassert>
#include <kcl_os.h>
#include <ngl.h>

using namespace GFXB;

Texture::Texture(TextureFactory *factory)
{
	m_factory = factory;

	m_id = 0;
	m_releaseUponCommit = true;//HACK always free memory after commit
	m_anisotropy_value = 1u;
}


Texture::~Texture()
{
}


long Texture::bind(KCL::uint32 slotId)
{
	return 0;
}


long Texture::commit()
{
	KCL::KCL_Status status = KCL::KCL_TESTERROR_NOERROR;
	switch (m_image->getType())
	{
	case KCL::Image_2D:
		status = Commit2D();
		break;

	default:
		INFO("Texture - Commit: Unsupported image type! %s", m_image->getName().c_str());
		status = KCL::KCL_TESTERROR_UNKNOWNERROR;
		break;
	}

	if (m_releaseUponCommit)
	{
		delete m_image;
		m_image = nullptr;
	}

	return status == KCL::KCL_TESTERROR_NOERROR ? 0L : -1L;
}


NGL_format Texture::GetNGLTextureFormat(const KCL::_ImageFormat format, bool is_srgb, bool &compressed)
{
	compressed = false;

	switch (format)
	{
	case KCL::Image_RGB9E5:
	{
		return NGL_R9_G9_B9_E5_SHAREDEXP;
	}
	case KCL::Image_RGB888:
	{
		return (!is_srgb) ? NGL_R8_G8_B8_UNORM : NGL_R8_G8_B8_UNORM_SRGB;
	}
	case KCL::Image_RGBA8888:
	{
		return (!is_srgb) ? NGL_R8_G8_B8_A8_UNORM : NGL_R8_G8_B8_A8_UNORM_SRGB;
	}
	case KCL::Image_RGBA_32F:
	{
		return NGL_R16_G16_B16_A16_FLOAT;
		return NGL_R32_G32_B32_A32_FLOAT;
	}
	case KCL::Image_ETC2_RGB:
	{
		compressed = true;
		return (!is_srgb) ? NGL_R8_G8_B8_ETC2_UNORM : NGL_R8_G8_B8_ETC2_UNORM_SRGB;
	}
	case KCL::Image_ETC2_RGB_A1:
	{
		compressed = true;
		return (!is_srgb) ? NGL_R8_G8_B8_A1_ETC2_UNORM : NGL_R8_G8_B8_A1_ETC2_UNORM_SRGB;
	}
	case KCL::Image_ETC2_RGBA8888:
	{
		compressed = true;
		return (!is_srgb) ? NGL_R8_G8_B8_A8_ETC2_UNORM : NGL_R8_G8_B8_A8_ETC2_UNORM_SRGB;
	}
	case KCL::Image_DXT1:
	{
		compressed = true;
		//HACK: we should distinguish the NGL_R8_G8_B8_DXT1_UNORM and NGL_R8_G8_B8_A1_DXT1_UNORM
		return (!is_srgb) ? NGL_R8_G8_B8_A1_DXT1_UNORM : NGL_R8_G8_B8_A1_DXT1_UNORM_SRGB;
	}
	case KCL::Image_DXT1_RGBA:
	{
		compressed = true;
		return (!is_srgb) ? NGL_R8_G8_B8_A1_DXT1_UNORM : NGL_R8_G8_B8_A1_DXT1_UNORM_SRGB;
	}
	case KCL::Image_DXT5:
	{
		compressed = true;
		return (!is_srgb) ? NGL_R8_G8_B8_A8_DXT5_UNORM : NGL_R8_G8_B8_A8_DXT5_UNORM_SRGB;
	}
	case KCL::Image_RGBA_ASTC_4x4:
	{
		compressed = true;
		return (!is_srgb) ? NGL_R8_G8_B8_A8_ASTC_4x4_UNORM : NGL_R8_G8_B8_A8_ASTC_4x4_UNORM_SRGB;
	}
	case KCL::Image_RGBA_ASTC_6x6:
	{
		compressed = true;
		return (!is_srgb) ? NGL_R8_G8_B8_A8_ASTC_6x6_UNORM : NGL_R8_G8_B8_A8_ASTC_6x6_UNORM_SRGB;
	}

	default:
	{
		INFO("Texture - Unhandled texture format: %d", format);
		return NGL_UNDEFINED;
	}
	}
}


KCL::KCL_Status Texture::Commit2D()
{
	NGL_texture_descriptor texture_layout;
	texture_layout.m_name = m_name;
	texture_layout.m_type = NGL_TEXTURE_2D;

	bool compressed = false;
	NGL_format texture_format = GetNGLTextureFormat(m_image->getFormat(), m_isSRGB, compressed);
	if (texture_format == NGL_UNDEFINED)
	{
		INFO("Error! Texture::Commit2D - Unhandled KCL format: %d for image: %s", m_image->getFormat(), m_image->getName().c_str());
		return KCL::KCL_TESTERROR_UNKNOWNERROR;
	}

	texture_layout.m_format = texture_format;
	texture_layout.m_wrap_mode = (m_wrapS == KCL::TextureWrap_Repeat) ? NGL_REPEAT_ALL : NGL_CLAMP_ALL;

	KCL::uint32 levels = m_image->getMipCount() + 1;
	bool mipmap_required = MipmapRequired(m_mipFilter);
	if (m_name.find("OVR") != std::string::npos)
	{
		mipmap_required = false;
		m_minFilter = KCL::TextureFilter_Linear;
	}

	if (m_name.find("sky") != std::string::npos)
	{
		if (compressed)
		{
			INFO("Warning! Texture::Commit2D - Sky texture: %s is compressed!", m_name.c_str());
		}
		else
		{
			INFO("Sky texture: %s", m_name.c_str());
			mipmap_required = false;
			m_anisotropy_value = 1;
			m_minFilter = KCL::TextureFilter_Linear;
			texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;
		}
	}

	if (mipmap_required)
	{
		if (compressed || levels > 1)
		{
			// Check if we have enough LOD levels when the image is compressed or uncompressed with LOD levels
			KCL::uint32 required_levels = (KCL::uint32)KCL::texture_levels(m_width, m_height);
			if (required_levels != levels)
			{
				INFO("Error! Texture::Commit2D - Does not have enough LOD levels: ", m_image->getName().c_str());
				assert(false);
				return KCL::KCL_TESTERROR_UNKNOWNERROR;
			}
		}

		texture_layout.m_filter = (m_minFilter == KCL::TextureFilter_Linear)
			? NGL_LINEAR_MIPMAPPED
			: NGL_NEAREST_MIPMAPPED;
	}
	else
	{
		// Only upload the first LOD level
		levels = 1;

		texture_layout.m_filter = (m_minFilter == KCL::TextureFilter_Linear)
			? NGL_LINEAR
			: NGL_NEAREST;
	}

	// Turn on anisotropic if enabled
	if (m_anisotropy_value > 1 && isAnisotropicFilterEnabled())
	{
		texture_layout.m_filter = NGL_ANISO_4;
	}

	// Dimensions of a block in texels
	KCL::uint32 block_dim_x, block_dim_y, block_dim_z;
	m_image->getBlockDimensions(block_dim_x, block_dim_y, block_dim_z);

	// Size of a block in bits
	KCL::uint32 block_size = m_image->getBlockSize();

	KCL::uint32 data_offset = 0;
	KCL::uint32 levels_to_skip = 0;
	KCL::uint32 mip_level_limit = m_factory->GetMipLevelLimit();
	if (mip_level_limit && levels > mip_level_limit)
	{
		levels_to_skip = levels - mip_level_limit;

		for (KCL::uint32 i = 0; i < levels_to_skip; i++)
		{
			data_offset += GetDataSize(i, block_size, block_dim_x, block_dim_y);
		}

		levels = mip_level_limit;

		GetLevelDimensions(levels_to_skip, texture_layout.m_size[0], texture_layout.m_size[1]);
	}
	else
	{
		texture_layout.m_size[0] = m_image->getWidth();
		texture_layout.m_size[1] = m_image->getHeight();
	}

	std::vector<std::vector<uint8_t> > data(levels);

	for (KCL::uint32 i = 0; i < levels; i++)
	{
		KCL::uint32 data_size = GetDataSize(i + levels_to_skip, block_size, block_dim_x, block_dim_y);

		uint8_t* from = (uint8_t*)m_image->getData() + data_offset;
		data[i].resize(data_size);
		memcpy(data[i].data(), from, data_size);

		data_offset += data_size;
	}

	texture_layout.m_size[2] = 1;
	texture_layout.m_num_levels = levels;
	texture_layout.m_num_array = 1;
	texture_layout.m_wrap_mode = (m_wrapS == KCL::TextureWrap_Clamp)?NGL_CLAMP_ALL:NGL_REPEAT_ALL;

	m_id = 0;
	if (nglGenTexture(m_id, texture_layout, &data) == false)
	{
		INFO("Error! Texture::Commit2D - Can not commit texture to NGL: %s", m_image->getName().c_str());
		return KCL::KCL_TESTERROR_UNKNOWNERROR;
	}

	m_type = KCL::Texture_2D;
	m_mipLevels = texture_layout.m_num_levels;

	return KCL::KCL_TESTERROR_NOERROR;
}


long Texture::release()
{
	return 0;
}


long Texture::setVideoTime(float time)
{
	return 0;
}


KCL::uint32 Texture::textureObject()
{
	return m_id;
}


void Texture::SetAnisotropyValue(KCL::uint32 value)
{
	m_anisotropy_value = KCL::Max(value, 1u);
}


bool Texture::MipmapRequired(KCL::TextureFilter filter)
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


void Texture::GetLevelDimensions(KCL::uint32 level, KCL::uint32 &width, KCL::uint32 &height)
{
	// Calculate the dimensions of the mipmap
	width = m_width / (1 << level);
	height = m_height / (1 << level);
	width = width ? width : 1;
	height = height ? height : 1;
}


KCL::uint32 Texture::GetDataSize(KCL::uint32 level, KCL::uint32 block_size, KCL::uint32 block_dim_x, KCL::uint32 block_dim_y)
{
	KCL::uint32 mipmap_width;
	KCL::uint32 mipmap_height;

	GetLevelDimensions(level, mipmap_width, mipmap_height);

	// Calculate the number of blocks for the mipmap
	KCL::uint32 block_count_x = (mipmap_width + block_dim_x - 1) / block_dim_x;
	KCL::uint32 block_count_y = (mipmap_height + block_dim_y - 1) / block_dim_y;

	// Calculate the compressed data size
	KCL::uint32 data_size = block_count_x * block_count_y * block_size; // in bits
	data_size /= 8; // in bytes

	return data_size;
}
