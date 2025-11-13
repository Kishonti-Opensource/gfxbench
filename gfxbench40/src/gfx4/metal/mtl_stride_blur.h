/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_STRIDE_BLUR_H
#define MTL_STRIDE_BLUR_H

#include <kcl_math3d.h>
#include <kcl_camera2.h>

#include <vector>
#include <string>

#include "mtl_globals.h"
#include "mtl_pipeline.h"
#include "mtl_quadBuffer.h"

namespace MetalRender
{

class StrideBlur
{
public:
	StrideBlur();
	~StrideBlur();

	void Init(id<MTLDevice> device, KCL::uint32 width, KCL::uint32 height, KCL::uint32 stride, KCL::uint32 kernel_size);
	void Render(id<MTLCommandBuffer> command_buffer, MetalRender::QuadBuffer * quad_buffer, KCL::Camera2 * camera, id<MTLTexture> input_texture, id<MTLTexture> depth_texture);

	id<MTLTexture> m_color_texture;

private:
	Pipeline * m_shader_h_pipeline;
	Pipeline * m_shader_v_pipeline;

	id<MTLTexture> m_temp_texture;
	id<MTLSamplerState> m_nearest_sampler;
	id<MTLSamplerState> m_linear_sampler;

	static std::vector<float> GetGaussWeights(unsigned int kernel_size);
	static std::string GetGaussWeightsString(unsigned int kernel_size);
	static std::string GetOffsetsString(KCL::uint32 kernel_size, float stride, KCL::uint32 width, bool y);
};

}

#endif