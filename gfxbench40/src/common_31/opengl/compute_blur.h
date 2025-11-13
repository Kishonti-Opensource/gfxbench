/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef COMPUTE_BLUR
#define COMPUTE_BLUR

#include "kcl_base.h"
#include "glb_shader2.h"
#include "glb_filter.h"

class GLB_Scene_ES2_;

class ComputeBlur 
{
public:
	enum PackType
	{
		PACK_RGB_SELECTOR = 0,
		PACK_RG_SELECTOR,
		PACK_R_SELECTOR,

		PACK_TYPE_COUNT
	};

	ComputeBlur(KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, KCL::uint32 texture_format, PackType pack_type);
	virtual ~ComputeBlur();

	void Init(GLB_Scene_ES2_* scene);
	void Execute(GLuint pre_barrier, GLuint post_barrier);

	void SetInputTexture(KCL::uint32 in_tex);
    KCL::uint32 GetOutputTexture() const;

private:
	KCL::uint32 m_in_texture;
    KCL::uint32 m_sampler;    
	KCL::uint32 m_out_texture;
	KCL::uint32 m_temp_texture;

	KCL::uint32 m_width, m_height;
	KCL::uint32 m_texture_format;
	PackType    m_pack_type;

	KCL::uint32 m_blur_in_bind, m_blur_out_bind;
	KCL::uint32 m_blur_work_group_size, m_blur_kernel_size;

	GLB::GLBShader2 * m_blur_shader_h;
	GLB::GLBShader2 * m_blur_shader_v;

	GLB::GLBFilter m_blur_filter_h;
	GLB::GLBFilter m_blur_filter_v;
    //for statistics
    GLB_Scene_ES2_* m_scene;

	void SetupBuilder(GLB::GLBShaderBuilder &sb, int ks, std::string weights, std::string offsets) ;
	
	struct PackDescriptor
	{
		std::string m_vectype_string;
		std::string m_pack_source;
		std::string m_unpack_source;
	};

	static PackDescriptor m_pack_descriptors[PACK_TYPE_COUNT];
};

#endif