/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_BLOOM_H
#define GFXB_BLOOM_H

#include <kcl_base.h>
#include <kcl_math3d.h>
#include <ngl.h>

namespace GFXB
{
	class Shapes;
	class ComputeHDR;
	class GenMipmaps;
	class FragmentBlur;

	class Bloom
	{
	public:
		Bloom();
		~Bloom();

		void Init(Shapes *shapes, ComputeHDR *hdr, KCL::uint32 width, KCL::uint32 height, KCL::uint32 layers, KCL::uint32 blur_strength, NGL_format texture_format);

		void Resize(KCL::uint32 blur_strength, KCL::uint32 width, KCL::uint32 height);
		void DeletePipelines();

		void SetColorCorrection(const KCL::Vector4D &color_correction);
		void ExecuteLumianceBufferBarrier(KCL::uint32 command_buffer);

		KCL::uint32 ExecuteBrightPass(KCL::uint32 command_buffer, KCL::uint32 input_texture);
		KCL::uint32 ExecuteDownsample(KCL::uint32 command_buffer, KCL::uint32 input_layer);
		KCL::uint32 ExectureVerticalBlur(KCL::uint32 command_buffer, KCL::uint32 layer);
		KCL::uint32 ExectureHorizontalBlur(KCL::uint32 command_buffer, KCL::uint32 layer);

		KCL::uint32 GetBrightTexture(KCL::uint32 layer) const;
		KCL::uint32 GetBloomTexture(KCL::uint32 layer) const;

		void *GetUniformBrightTexture(KCL::uint32 layer);
		void *GetUniformBloomTexture(KCL::uint32 layer);

		KCL::uint32 GetLayerCount() const;

	protected:
		Shapes *m_shapes;
		ComputeHDR *m_hdr;

		KCL::uint32 m_width;
		KCL::uint32 m_height;

		KCL::uint32 m_layers;
		KCL::uint32 m_blur_strength;
		NGL_format m_texture_format;

		std::vector<KCL::uint32> m_bright_textures;

		KCL::uint32 m_bright_pass;
		KCL::uint32 m_bright_pass_manual_exposure_shader;
		KCL::uint32 m_bright_pass_auto_exposure_shader;

		GenMipmaps *m_gen_mipmaps;
		std::vector<FragmentBlur*> m_blurs;

		KCL::Vector4D m_color_correction;

		void GetLayerSize(KCL::uint32 layer, KCL::uint32 texture_size[3]);
	};
}

#endif