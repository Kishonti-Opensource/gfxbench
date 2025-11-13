/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_BLOOM_MOBILE_H
#define GFXB_BLOOM_MOBILE_H

#include <kcl_base.h>
#include <kcl_math3d.h>
#include <ngl.h>
#include "gfxb_bloom.h"
#include "common/gfxb_bloom_downsample_blur.h"
#include "common/gfxb_bloom_upsample_blur.h"

namespace GFXB
{
	class Shapes;
	class ComputeHDR;
	class GenMipmaps;
	class FragmentBlur;

	class BloomMobile : public Bloom
	{
	public:
		BloomMobile();
		~BloomMobile();
		void Init( Shapes *shapes, ComputeHDR *hdr, KCL::uint32 width, KCL::uint32 height, KCL::uint32 layers, KCL::uint32 strength_down, KCL::uint32 strength_up, NGL_format texture_format );
		KCL::uint32 ExecuteUpsample( KCL::uint32 command_buffer, KCL::uint32 input_layer );
		KCL::uint32 ExecuteDownsample( KCL::uint32 command_buffer, KCL::uint32 input_layer );
		KCL::uint32 ExecuteBrightPass( KCL::uint32 command_buffer, KCL::uint32 input_texture );

		void *GetUniformBloomTexture( KCL::uint32 layer );
		KCL::uint32 GetBloomTexture( KCL::uint32 layer ) const;
		void DeletePipelines();
		void Resize( KCL::uint32 blur_strength, KCL::uint32 width, KCL::uint32 height );
	protected:
		std::vector<BloomDownsampleBlur*> m_downsample_blurs;
		std::vector<BloomUpsampleBlur*> m_upsample_blurs;

		// Gauss components
		std::vector<KCL::Vector4D> packed_offsets;
		std::vector<KCL::Vector4D> packed_weights;
		KCL::uint32 m_blur_kernel_size;
		std::vector<KCL::Vector2D> m_stepuv;
	private:
		KCL::uint32 ExectureVerticalBlur( KCL::uint32 layer )
		{
			return -1;
		}
		KCL::uint32 ExectureHorizontalBlur( KCL::uint32 layer )
		{
			return -1;
		}
	};
}

#endif