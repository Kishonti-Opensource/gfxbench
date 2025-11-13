/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_TEXTURE_H
#define GFXB_TEXTURE_H

#include "kcl_texture.h"
#include <ngl.h>

namespace GFXB
{
	class TextureFactory;

	class Texture : public KCL::Texture
	{
	public:
		Texture(TextureFactory *factory);
		virtual ~Texture();

		virtual long commit() override;
		virtual long release() override;
		virtual long bind(KCL::uint32 slotId) override;
		virtual long setVideoTime(float time) override;
		virtual KCL::uint32 textureObject() override;

		void SetAnisotropyValue(KCL::uint32 value);

		static NGL_format GetNGLTextureFormat(const KCL::_ImageFormat format, bool is_srgb, bool &compressed);

		KCL::uint32 m_id;

	private:
		TextureFactory *m_factory;

		KCL::uint32 m_anisotropy_value;

		KCL::KCL_Status Commit2D();
		void GetLevelDimensions(KCL::uint32 level, KCL::uint32 &width, KCL::uint32 &height);
		KCL::uint32 GetDataSize(KCL::uint32 level, KCL::uint32 block_size, KCL::uint32 block_dim_x, KCL::uint32 block_dim_y);
		static bool MipmapRequired(KCL::TextureFilter filter);
	};
}

#endif
