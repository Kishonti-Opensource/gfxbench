/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_STRIDE_BLUR_H
#define GLB_STRIDE_BLUR_H

#include <kcl_math3d.h>
#include <kcl_camera2.h>
#include "opengl/glb_shader2.h"

#include <vector>
#include <string>

namespace GLB
{

class StrideBlur
{
public:
	StrideBlur();
	~StrideBlur();

	void Init(KCL::uint32 width, KCL::uint32 height, KCL::uint32 stride, KCL::uint32 kernel_size);
	void Render(KCL::uint32 fullscreen_vao, KCL::Camera2 *camera, KCL::uint32 input_texture, KCL::uint32 depth_texture);

	KCL::uint32 m_color_texture;

private:
	GLBShader2 *m_shader_h;
	GLBShader2 *m_shader_v;

	KCL::uint32 m_temp_texture;
	KCL::uint32 m_sampler;
	KCL::uint32 m_depth_sampler;

	KCL::uint32 m_fbo1;
	KCL::uint32 m_fbo2;

	static std::vector<float> GetGaussWeights(unsigned int kernel_size);
	static std::string GetGaussWeightsString(unsigned int kernel_size);
	static std::string GetOffsetsString(KCL::uint32 kernel_size, float stride, KCL::uint32 width, bool y);
};

}

#endif