/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_SCENE_OPENGL4_H
#define GLB_SCENE_OPENGL4_H

#include "glb_scene_.h"
#include "opengl/shader.h"

namespace GLB
{
	class GLBTexture;
} 

struct Filter2;


class VirtualDashboardScene : public GLB_Scene_ES2_
{
public:
	VirtualDashboardScene();
	~VirtualDashboardScene();
	
	virtual KCL::KCL_Status Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples);
	void Render();

private:
	KCL::Texture *m_logo_texture;
	KCL::uint32 m_logo_vbo;
	Shader* m_logo_shader;

	Filter2 *m_filters;

	KCL::uint32 m_fullscreen_quad_vbo;
};


#endif