/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "compute_blur.h"

#include "platform.h"
#include "opengl/glb_discard_functions.h"
#include "glb_scene_.h"
#include "gauss_blur_helper.h"

#include <sstream>
#include <iomanip>

using namespace GLB;

ComputeBlur::ComputeBlur(KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, KCL::uint32 texture_format, PackType pack_type)
{
	m_width = width;
	m_height = height;
	m_texture_format = texture_format;
	m_pack_type = pack_type;

	m_blur_kernel_size = blur_kernel_size;
	
	m_blur_shader_h = NULL;
	m_blur_shader_v = NULL;
}

ComputeBlur::~ComputeBlur()
{
	glDeleteTextures(1,&m_temp_texture);
	glDeleteTextures(1,&m_out_texture);
	glDeleteSamplers(1,&m_sampler);
}

void ComputeBlur::Init(GLB_Scene_ES2_* scene)
{
    m_scene = scene;

	glGenTextures(1,&m_temp_texture);
	glBindTexture(GL_TEXTURE_2D,m_temp_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexStorage2D(GL_TEXTURE_2D,1,m_texture_format,m_width,m_height);

	glGenTextures(1,&m_out_texture);
	glBindTexture(GL_TEXTURE_2D,m_out_texture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexStorage2D(GL_TEXTURE_2D,1,m_texture_format,m_width,m_height);

	m_blur_in_bind = 0;
	m_blur_out_bind = 1;
	m_blur_work_group_size = 128;

	// Load the shaders
	KCL::KCL_Status error;
	GLBShaderBuilder sb;

	std::vector<float> gauss_weights = COMMON31::GaussBlurHelper::GetGaussWeights(m_blur_kernel_size, true);

	int ks = m_blur_kernel_size + 1;
    std::vector<float> packed_weights = COMMON31::GaussBlurHelper::CalcPackedWeights(gauss_weights);
    std::string weights_str = COMMON31::GaussBlurHelper::GaussFloatListToString(packed_weights);

	std::vector<float> horizontal_packed_offsets = COMMON31::GaussBlurHelper::CalcPackedOffsets(m_width,m_blur_kernel_size,gauss_weights);
	std::string horizontal_offset_str = COMMON31::GaussBlurHelper::GaussFloatListToString(horizontal_packed_offsets);

	std::vector<float> vertical_packed_offsets = COMMON31::GaussBlurHelper::CalcPackedOffsets(m_height,m_blur_kernel_size,gauss_weights);
	std::string vertical_offset_str = COMMON31::GaussBlurHelper::GaussFloatListToString(vertical_packed_offsets);

	
	sb.FileCs("gauss_blur_fetch.shader");
	SetupBuilder(sb,ks,weights_str,horizontal_offset_str);
	sb.AddDefine("HORIZONTAL");
	m_blur_shader_h = sb.Build(error);

	sb.FileCs("gauss_blur_fetch.shader");
	SetupBuilder(sb,ks,weights_str,vertical_offset_str);
	sb.AddDefine("VERTICAL");
	m_blur_shader_v = sb.Build(error);

	glGenSamplers(1, &m_sampler);
	glSamplerParameteri(m_sampler,GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glSamplerParameteri(m_sampler,GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glSamplerParameteri(m_sampler,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(m_sampler,GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Setup the filters
	m_blur_filter_v.m_shader = m_blur_shader_v;
	m_blur_filter_v.m_internal_format = m_texture_format;
	m_blur_filter_v.m_compute_out_tex = m_temp_texture;
	m_blur_filter_v.m_input_samplers[0] = m_sampler;
	m_blur_filter_v.m_compute_group_x = m_width;
	m_blur_filter_v.m_compute_group_y = m_height / m_blur_work_group_size + 1;
    m_blur_filter_v.m_scene = m_scene;

	m_blur_filter_h.m_shader = m_blur_shader_h;
	m_blur_filter_h.m_internal_format = m_texture_format;
	m_blur_filter_h.m_input_samplers[0] = m_sampler;
	m_blur_filter_h.m_input_textures[0] = m_temp_texture;
	m_blur_filter_h.m_compute_out_tex = m_out_texture;
	m_blur_filter_h.m_compute_group_x = m_width / m_blur_work_group_size+ 1;
	m_blur_filter_h.m_compute_group_y = m_height + 1;
    m_blur_filter_h.m_scene = m_scene;
}

void ComputeBlur::SetupBuilder(GLB::GLBShaderBuilder &sb, int ks, std::string weights_str, std::string offsets_str) 
{
	sb.AddDefineInt("KS",ks);
	sb.AddDefine("GAUSS_WEIGHTS "+weights_str);
	sb.AddDefine("GAUSS_OFFSETS "+offsets_str);


    sb.AddDefineInt("OUT_TEX_BIND",m_blur_out_bind);
	
	sb.AddDefine("GLSL_TEX_FORMAT "+GLBShader2::GLSLTextureFormat(m_texture_format));
	sb.AddDefine("VEC_TYPE "+m_pack_descriptors[m_pack_type].m_vectype_string);
	sb.SourceCs(m_pack_descriptors[m_pack_type].m_pack_source.c_str());
	sb.SourceCs(m_pack_descriptors[m_pack_type].m_unpack_source.c_str());

	sb.AddDefineInt("LOCAL_WORK_SIZE",m_blur_work_group_size);

	sb.AddDefineFloat("STEP_X",1.0/(float)m_width);
	sb.AddDefineFloat("STEP_Y",1.0/(float)m_height);
}

void ComputeBlur::Execute(GLuint pre_barrier, GLuint post_barrier) 
{
	if (pre_barrier != 0)
	{
		glMemoryBarrierProc(pre_barrier);
	}

#ifdef OCCLUSION_QUERY_BASED_STAT
    m_blur_filter_v.Render(m_scene->m_glGLSamplesPassedQuery);
#else
	m_blur_filter_v.Render();
#endif

	glMemoryBarrierProc(GL_TEXTURE_FETCH_BARRIER_BIT);

#ifdef OCCLUSION_QUERY_BASED_STAT
	m_blur_filter_h.Render(m_scene->m_glGLSamplesPassedQuery);
#else
	m_blur_filter_h.Render();
#endif

	if (post_barrier != 0)
	{
		glMemoryBarrierProc(post_barrier);
	}	
}

void ComputeBlur::SetInputTexture(KCL::uint32 in_tex)
{
	m_in_texture = in_tex;
	m_blur_filter_v.m_input_textures[0] = in_tex;
}

KCL::uint32 ComputeBlur::GetOutputTexture() const
{
    return m_out_texture;
}

ComputeBlur::PackDescriptor ComputeBlur::m_pack_descriptors[PACK_TYPE_COUNT] =
{
	//PACK_RGB_SELECTOR,
	{	
		"vec3",
		"vec4 pack(vec3 v) { return vec4(v,0.0); }",
		"vec3 unpack(vec4 v) { return v.xyz; }"
	},

	//PACK_RG_SELECTOR,
	{
		"vec2",
		"vec4 pack(vec2 v) { return vec4(v,0.0,0.0); }",
		"vec2 unpack(vec4 v) { return v.xy; }"
	},
	
	//PACK_R_SELECTOR,
	{
		"float",
		"float pack(float v) { return vec4(v,0.0,0.0,0.0); }",
		"float unpack(vec4 v) { return v.x; }"
	}
};

