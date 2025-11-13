/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_FILTER_H
#define GLB_FILTER_H

#include "glb_kcl_adapter.h"
#include "render_statistics_defines.h"

#include "kcl_texture.h"
#include "opengl/glb_image.h"
#include "glb_shader2.h"
#include "krl_cubemap.h"
#include "opengl/cubemap.h"

#include <vector>

class GLB_Scene_ES2_;
class GLSamplesPassedQuery;

namespace GLB
{
class GLBFilter
{
public:
	static const int MAX_INPUT_TEXTURES = 8;
	static const int MAX_INPUT_DEPTH_TEXTURES = 1;
	static const int MAX_INPUT_IMAGE_TEXTURES = 4;

	GLBFilter();
	~GLBFilter();
	
	void Init(KCL::uint32 vao,KCL::uint32 vbo, KCL::uint32 depth_attachment, KCL::uint32 width, KCL::uint32 height, bool onscreen, KCL::uint32 maxmipcount, int dir, KCL::uint32 internal_format);
	
#if defined OCCLUSION_QUERY_BASED_STAT
    void Render(GLSamplesPassedQuery *glGLSamplesPassedQuery);
#else
    void Render();
#endif
    			
	void SetClearBitmask(KCL::uint32 mask);
	void SetClearColor(const KCL::Vector4D &color);

	KCL::uint32 Width() const;
	KCL::uint32 Height() const;

    KCL::uint32 GetFramebufferObject(KCL::uint32 lod_level) const;

	KCL::uint32 m_vao;
	KCL::uint32 m_vbo;
	KCL::uint32 m_color_texture;

	GLBShader2 *m_shader;

	KCL::uint32 m_input_textures[MAX_INPUT_TEXTURES];
	KCL::uint32 m_input_samplers[MAX_INPUT_TEXTURES];

	KCL::uint32 m_input_depth_textures[MAX_INPUT_DEPTH_TEXTURES];

	KCL::int32 m_input_image_textures[MAX_INPUT_IMAGE_TEXTURES];

    bool m_reconstructPosInWS;

	bool m_clear_to_color;
	KCL::Vector4D m_clear_color;

	int m_dir;
    KCL::uint32 m_mipmap_count; // ~0 means max mipmap, 0 and 1 means no mipmap
	bool m_is_mipmapped;
	KCL::Camera2 *m_active_camera;
	float m_focus_distance;
		
	KCL::uint32 m_clear_bitmask; // 0 means no clear
	KCL::uint32 m_internal_format;

	KCL::uint32 m_compute_group_x;
	KCL::uint32 m_compute_group_y;
	KCL::uint32 m_compute_group_z;

	KCL::uint32 m_compute_out_tex;

	KCL::uint32 m_shadow_texture;
	KCL::uint32 m_shadow_sampler;
	KCL::Vector4D m_shadow_distances;
	const KCL::Matrix4x4 *m_shadow_matrices;

	float m_ssao_projection_scale;

    KCL::int32 m_render_target_level;
    KCL::uint32 m_gauss_lod_level;

	uint32 m_static_cubemaps_object;
	uint32 m_dynamic_cubemap_objects[2];

    GLB_Scene_ES2_* m_scene;

private:
	void CreateRenderTarget(KCL::uint32 width, KCL::uint32 height, bool linear_filtering, KCL::int32 format, KCL::uint32 depth_attachment);	

    void CalcLodDimensions(KCL::uint32 width, KCL::uint32 height, KCL::uint32 lod_count);

    void ReleaseResources();
    void SetDefaults();

	bool m_onscreen;

	KCL::uint32 m_width;
	KCL::uint32 m_height;
    // Separate framebuffer objects for the render target's LOD levels
    std::vector<KCL::uint32> m_fbos;
    // LOD level dimensions 
    std::vector<KCL::Vector2D> m_lod_dims;
};

}

#endif
