/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef BLOOM_DOWNSAMPLE_BLUR_H
#define BLOOM_DOWNSAMPLE_BLUR_H

#include "kcl_base.h"
#include <vector>
#include "gfxb_fragment_blur.h"

namespace GFXB
{

class Shapes;

class BloomDownsampleBlur : protected FragmentBlur
{
public:

	void Init( Shapes *shapes, KCL::uint32 input_texture, KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, NGL_format texture_format, KCL::uint32 lod_levels, const char* name );
	KCL::uint32 RenderBlurPass( KCL::uint32 command_buffer, KCL::uint32 lod_level = 0u );
	KCL::uint32 GetOutputTexture() const;
	void *GetUniformOutputTexture();
	void DeletePipelines();

	//starting position on circle, number of points, # of current point
	static KCL::Vector2D circle( float start, float points, float point )
	{
		float rad = ( 3.141592f * 2.0f * ( 1.0f / points ) ) * ( point + start );
		return KCL::Vector2D( sinf( rad ), cosf( rad ) );
	}

	void Resize( KCL::uint32 blur_kernel_size, KCL::uint32 width, KCL::uint32 height ) override;

protected:

	void SetKernelSize(KCL::uint32 blur_kernel_size);

	void GenerateBlur( const char *fragment_shader, const int blur_kernel_size );

	void ResizeJobs();

	KCL::uint32 m_blur_shader;
	KCL::uint32 m_blur_texture;
	std::vector<KCL::uint32> m_blur_job;

	float m_weight;
	std::vector<KCL::Vector2D> m_packed_offsets;

	Shapes *m_shapes;

	KCL::uint32 m_width;
	KCL::uint32 m_height;
	NGL_format m_texture_format;
	KCL::uint32 m_lod_levels;
	KCL::uint32 m_input_texture;
	std::vector<KCL::Vector2D> m_stepuv;

	KCL::uint32 m_blur_kernel_size;

private:
	//this blur is not separable!
	KCL::uint32 RenderVerticalPass( KCL::Vector4D depth_parameters, KCL::uint32 lod_level = 0u )
	{
		return -1;
	}
	KCL::uint32 RenderHorizontalPass( KCL::Vector4D depth_parameters, KCL::uint32 lod_level = 0u )
	{
		return -1;
	}
};

}//!namespace GFXB

#endif
