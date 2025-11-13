/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef FRAGMENT_BLUR_H
#define FRAGMENT_BLUR_H

#include "kcl_base.h"
#include "glb_shader2.h"
#include "glb_filter.h"

class GLB_Scene_ES2_;

namespace GLB
{
class FragmentBlur
{
public:
    FragmentBlur();
	virtual ~FragmentBlur();

    void Init(KCL::uint32 vao, KCL::uint32 vbo, KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, KCL::uint32 texture_format, KCL::uint32 lod_levels, GLB_Scene_ES2_* scene);   
	void Execute();		
    
    void SetInputTexture(KCL::uint32 in_tex);
    KCL::uint32 GetOutputTexture() const;    

private:
	KCL::uint32 m_width, m_height;
    KCL::uint32 m_lod_levels;
    bool m_has_lod;

	KCL::uint32 m_sampler;

	GLB::GLBShader2 *m_blur_shader_h;
	GLB::GLBShader2 *m_blur_shader_v;
    
	GLB::GLBFilter m_blur_filter_h;
	GLB::GLBFilter m_blur_filter_v;
    //for statistics
    GLB_Scene_ES2_* m_scene;
};
}

#endif