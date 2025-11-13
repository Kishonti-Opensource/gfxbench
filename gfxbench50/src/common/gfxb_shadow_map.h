/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_SHADOW_MAP_H
#define GFXB_SHADOW_MAP_H

#include <kcl_math3d.h>
#include <kcl_camera2.h>
#include <ngl.h>

namespace GFXB
{
	class MeshFilter;
	class FrustumCull;
	class Light;

	class ShadowMap
	{
	public:
		enum ShadowType
		{
			DIRECTIONAL = 0,
			PERSPECTIVE,
			CUBE,
			PARABOLOID
		};

		ShadowMap();
		~ShadowMap();

		void Init(const char *name, KCL::SceneHandler *scene, ShadowType shadow_type, KCL::uint32 width, KCL::uint32 height, NGL_format depth_component_format, MeshFilter *mesh_filter = nullptr);

		void Warmup(KCL::uint32 command_buffer, const std::vector<KCL::Mesh*> &meshes);

		void Animate(KCL::uint32 animation_time);

		// Clear the shadow map
		void Clear(KCL::uint32 command_buffer);

		// Render scene with frustum cull
		KCL::uint32 Render(KCL::uint32 command_buffer);

		void DeletePipelines();
		void Resize(KCL::uint32 width, KCL::uint32 height);

		const std::string &GetName();
		ShadowType GetType() const;

		void SetPointLightCamera(const KCL::Vector3D pos, float radius, float near_plane);

		KCL::Camera2 &GetCamera();
		FrustumCull *GetFrustumCull();

		KCL::uint32 GetWidth() const;
		KCL::uint32 GetHeight() const;

		KCL::uint32 GetShadowTexture() const;
		const KCL::Matrix4x4 &GetShadowMatrix() const;

		void *GetUniformShadowTexture();
		void *GetUniformShadowMatrix();

		const std::vector<NGL_texture_subresource> &GetRenderTargets() const;

		Light* GetLight() { return m_light; }

		void IncludeActorsOnUpdate() { m_include_actors_on_update = true; }

		static GFXB::ShadowMap* CreateShadowMap(KCL::SceneHandler *scene, GFXB::Light* owner_light, MeshFilter *shadow_filter, KCL::uint32 shadow_map_size, NGL_format format);

	private:
		std::string m_name;

		ShadowType m_type;

		KCL::SceneHandler *m_scene;

		KCL::Camera2 m_camera;

		KCL::Vector3D m_point_pos;
		float m_point_radius;
		float m_point_near;

		KCL::uint32 m_width;
		KCL::uint32 m_height;

		FrustumCull *m_frustum_cull;
		Light* m_light;

		std::vector<KCL::uint32> m_jobs;
		std::vector<NGL_texture_subresource> m_render_targets;
		KCL::uint32 m_shaders[3];
		KCL::uint32 m_shadow_texture;
		KCL::Matrix4x4 m_shadow_matrix;
		KCL::Matrix4x4 m_bias_matrix;

		enum CullState
		{
			INVALID = 0,
			STATIC,
			DYNAMIC
		};

		struct UpdateState
		{
			CullState m_cull_state;
			KCL::Matrix4x4 m_vp;

			UpdateState()
			{
				m_cull_state = INVALID;
			}
		};

		std::vector<UpdateState> m_state;
		bool m_include_actors_on_update;

		void Render(KCL::uint32 command_buffer, KCL::uint32 job_index, const KCL::Camera2 &camera, const std::vector<KCL::Mesh*> &m_meshes);
		static void LookAtLeftHeaded(KCL::Matrix4x4 &M, const KCL::Vector3D &eye, const KCL::Vector3D &forward, const KCL::Vector3D &up);
		bool NeedToRedraw(UpdateState &current_state);
	};
}

#endif