/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GL_FILL2_H
#define GL_FILL2_H

#include "fill2_base.h"
#include "kcl_base.h"
#include "opengl/glb_texture.h"
#include "opengl/glb_shader2.h"

#include <vector>

class CompressedFillTest2 : public CompressedFillTest2_Base
{
public:
	CompressedFillTest2(const GlobalTestEnvironment* const gte);
	virtual ~CompressedFillTest2();

protected:
	virtual KCL::KCL_Status init ();
	virtual bool render();
	virtual void renderApiFinish();

	virtual void FreeResources();

private:
	struct RenderPass
	{
		GLB::GLBTextureES3 * textures[4];
		KCL::Vector4D offsets;
		RenderPass()
		{
			Clear();
		}
		void Clear()
		{
			textures[0] = textures[1] = textures[2] = textures[3] = NULL;
		}
	};
	static const KCL::uint32 m_render_pass_count = 4;
	RenderPass m_render_passes[m_render_pass_count];
	
	void CreateDepthTexture();
	void CreateFullScreenTextures(KCL::uint32 src_texture1, KCL::uint32 src_texture2);
	GLB::GLBTextureES3 * CreateTexture(KCL::Image * image);
	
	std::vector<GLB::GLBTextureES3 *> m_textures;

    KCL::uint32 m_vao;
	KCL::uint32 m_vao_portrait;
    KCL::uint32 m_vao_landscape;
	KCL::uint32 m_vertex_buffer_portait;
    KCL::uint32 m_vertex_buffer_landscape;
	KCL::uint32 m_index_buffer;

	KCL::uint32 m_fullscreen_background;
	KCL::uint32 m_fullscreen_mask;
	GLB::GLBTextureES3 * m_cube_texture;
	KCL::uint32 m_depth_texture;
	KCL::uint32 m_near_sampler;

	GLB::GLBShader2 *m_shader;
	GLB::GLBShader2 *m_shader_cube;
	GLB::GLBShader2 *m_shader_resize;
	KCL::int32 m_offsets_location;
	KCL::int32 m_rotation_location;
	KCL::int32 m_rotation_location_cube;

	KCL::uint32 m_viewport_width;
	KCL::uint32 m_viewport_height;
};

#endif