/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SAKURA_GI_H
#define SAKURA_GI_H

#include "kcl_math3d.h"
#include "kcl_light2.h"
#include "ngl.h"
#include "../../common/gfxb_warmup_helper.h"

namespace KCL
{
	class Mesh;
	class Camera2;
	class EnvProbe;
}


struct GI2
{
	static const char* GI_COMPUTE_SH_WGS_NAME;

	void DebugRender(uint32_t command_buffer, int32_t viewport[4], KCL::Camera2 *c, uint32_t sphere_vbid, uint32_t sphere_ibid);

	void Init(uint32_t max_num_envprobes, uint32_t color_texture, uint32_t depth_texture, bool compute_generate_sh, bool use_texture_sh_atlas);

	std::vector<KCL::Mesh*> m_meshes;
	std::vector<KCL::Light*> m_lights;
	std::vector<KCL::EnvProbe*> m_envprobes;

	uint32_t m_envprobe_indirect_uv_map;
	uint32_t m_envprobe_indirect_uv_depth_map;
	uint32_t m_envprobe_envmap_atlas;
	uint32_t m_envprobe_sh_atlas;
	uint32_t m_envprobe_sh_atlas_texture;
	uint32_t m_envprobe_size;
	int32_t m_envprobe_atlas_viewport[4];

	bool m_use_texture_sh_atlas;

	uint32_t m_lightmap;
	int32_t m_lightmap_viewport[4];

	bool m_render_shadows;

	KCL::Vector4D m_sky_color;
	float m_indirect_lighting_factor;

	struct
	{
		uint32_t m_job;
		uint32_t m_shader;
	}m_envprobe_visualize;

	struct
	{
		uint32_t m_job;

		//enum LightType
		//{
		//	AMBIENT = 128,
		//	DIRECTIONAL = 129,
		//	OMNI = 130,
		//	SPOT = 131,
		//	SMALL_OMNI = 132,
		//	SSAO = 133,
		//	SHADOW_DECAL = 134
		//};

		uint32_t m_shaders[KCL::LightShape::MAX_LIGHT_SHAPES];
		uint32_t m_shadow_shaders[KCL::LightShape::MAX_LIGHT_SHAPES];
	}m_direct_light_evaluation;

	struct
	{
		uint32_t m_job;
		uint32_t m_clear_job;
		uint32_t m_shader;
	}m_envprobe_uv_refresh;

	struct
	{
		uint32_t m_job;
		uint32_t m_shader;
	}m_envprobe_lightmap_atlas_transfer;

	struct
	{
		uint32_t m_job;
		uint32_t m_shader;
		uint32_t m_workgroup_size_x;
	}m_envprobe_generate_sh;

	uint32_t m_color_texture;
	uint32_t m_depth_texture;

	void ClearEnvProbes();
	void AddEnvProbe(KCL::EnvProbe *probe);
	void ReloadShaders();
	void FrustumCull(KCL::Camera2 *c);
	void envprobe_generate_sh(uint32_t command_buffer, uint32_t m_fullscreen_vbid, uint32_t m_fullscreen_ibid, std::vector<KCL::EnvProbe*> &visible_probes);
	void direct_light_evaluation(uint32_t command_buffer);
	void envprobe_uv_refresh(uint32_t command_buffer, std::vector<uint32_t> &indices);
	void envprobe_lightmap_atlas_transfer(uint32_t command_buffer, uint32_t fullscreen_vbid, uint32_t fullscreen_ibid, std::vector<KCL::EnvProbe*> &visible_probes);

	void WarmupGenerateSH(GFXB::WarmupHelper* warmup_helper, KCL::uint32 command_buffer);
	void LoadGenerateSHShader(uint32_t wg_size);

	bool save_textures;

private:
	static void LoadDeferredIrradianceVolumesShader(NGL_job_descriptor &jd, uint32_t shader_code, NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES], std::vector<NGL_shader_uniform> &application_uniforms);

	bool m_compute_generate_sh;
	uint32_t m_max_num_envprobes;
	std::vector<uint32_t> m_active_envprobe_ids;
};


#endif
