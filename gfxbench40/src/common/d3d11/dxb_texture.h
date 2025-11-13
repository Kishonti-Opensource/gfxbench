/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once

#include <d3d11_1.h>
#include "kcl_texture.h"
#include "DXUtils.h"
#include "DX.h"

namespace DXB	{

	class DXBTexture : public KCL::Texture
	{
    friend class KCL::Texture;

	protected:
		ID3D11Device* m_device;
		ID3D11Resource* m_texture;
		ID3D11SamplerState* m_sampler;
		ID3D11ShaderResourceView* m_resource;

	public:

		DXBTexture();
		DXBTexture(const KCL::Image* image, bool releaseUponCommit = false);
		DXBTexture(_ogg_decoder* video);
		DXBTexture(const KCL::Image* image, KCL::TextureType type, bool releaseUponCommit = false);
		virtual ~DXBTexture(void);

		virtual long bind(KCL::uint32 slotId);
		virtual long commit();
		virtual long release();
		inline ID3D11ShaderResourceView*const* getShaderResourceView() const	{ return &m_resource; }

		virtual long setVideoTime(float time);

        virtual KCL::uint32 textureObject() { return 0; }

	protected:
		virtual long commit1D(DXGI_FORMAT format);
		virtual long commit2D(DXGI_FORMAT format);
		virtual long commit3D(DXGI_FORMAT format);
		virtual long commitArray(DXGI_FORMAT format);
		virtual long commitVideo(DXGI_FORMAT format);

		inline DXGI_FORMAT getTextureFormat() const
		{
			if (m_video)
			{
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			}
			else if (m_image)
			{
				switch (m_image->getFormat())
				{
				case KCL::Image_DXT1:
				case KCL::Image_DXT1_RGBA:
					return DXGI_FORMAT_BC1_UNORM;

				case KCL::Image_RGBA8888:
					return DXGI_FORMAT_R8G8B8A8_UNORM;

				default:
					return DXGI_FORMAT_UNKNOWN;
				}
			}
			else
			{
				return DXGI_FORMAT_UNKNOWN;
			}
		};

		void getMipmapSubresource(UINT mipLevel, D3D11_SUBRESOURCE_DATA* subresourceData) const;
	};

	class DXBTextureFactory : public KCL::TextureFactory
	{
		virtual KCL::Texture *CreateTexture(const KCL::Image* img, bool releaseUponCommit = false)
		{
			return new DXBTexture(img, releaseUponCommit);
		};
	};
}