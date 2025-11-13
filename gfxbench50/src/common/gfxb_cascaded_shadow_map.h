/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_CASCADED_SHADOW_MAP
#define GFXB_CASCADED_SHADOW_MAP

#include <kcl_base.h>
#include <kcl_math3d.h>
#include <kcl_camera2.h>
#include <ngl.h>

#include "gfxb_frustum_cull.h"

namespace GFXB
{
	class SceneBase;
	class FrustumCull;

	// Mesh Filter
	class ShadowMeshFilter : public MeshFilter
	{
		virtual KCL::int32 FilterMesh(KCL::Camera2 *camera, KCL::Mesh *mesh, KCL::OverlapResult overlap_result) override;
		virtual KCL::uint32 GetMaxMeshTypes() override;
		virtual FrustumCull::PFN_MeshCompare GetMeshSortFunction(KCL::uint32 mesh_Type) override;
	};

	// Stable paralell split cascaded shadows
	class CascadedShadowMap
	{
	public:
		enum FitMode
		{
			FIT_SPHERE = 0,
			FIT_OBB,
		};

		enum SelectionMode
		{
			SELECTION_MAP_BASED = 0,
			SELECTION_DISTANCE_BASED
		};

		CascadedShadowMap(SceneBase *scene, KCL::uint32 shadow_map_size, NGL_format depth_component_format, MeshFilter *mesh_filter);
		~CascadedShadowMap();

		CascadedShadowMap &SetShadowNearFar(float near_distance, float far_distance);
		CascadedShadowMap &SetShadowNegativeRange(float range);
		CascadedShadowMap &SetShadowPositiveRange(float range);
		CascadedShadowMap &SetFitMode(FitMode mode);
		CascadedShadowMap &SetSelectionMode(SelectionMode mode);
		CascadedShadowMap &AddCascade(float near_distance);
		void FinalizeCascades();

		void DeletePipelines();

		void BuildFrustums(const KCL::Camera2 *camera, const KCL::Vector3D &light_dir);

		KCL::uint32 RenderShadow(KCL::uint32 command_buffer, KCL::uint32 cascade_index);

		void ExcludeActor(KCL::Actor *actor);

		void *GetUniformShadowMap();
		void *GetUniformShadowMatrix(KCL::uint32 cascade_index);
		void *GetUniformShadowMatrixes();
		void *GetUniformFrustumDistances();

		KCL::Camera2 *GetCamera(KCL::uint32 cascade_index)
		{
			return &m_frustums[cascade_index].camera;
		}
		KCL::uint32 GetCascadeCount() const
		{
			return m_cascade_count;
		}
		const KCL::Vector4D &GetFrustumDistances() const
		{
			return m_frustum_distances;
		}
		KCL::uint32 GetRenderer(KCL::uint32 cascade_index) const
		{
			return m_shadow_renderers[cascade_index];
		}
		const KCL::Matrix4x4 *GetShadowMatrices() const
		{
			return m_shadow_matrices.data();
		}
		const KCL::Matrix4x4 &GetShadowMatrix(KCL::uint32 cascade_index) const
		{
			return m_shadow_matrices[cascade_index];
		}
		KCL::Matrix4x4 &GetShadowMatrix(KCL::uint32 cascade_index)
		{
			return m_shadow_matrices[cascade_index];
		}
		const KCL::uint32 GetShadowMap() const
		{
			return m_shadow_map;
		}
		KCL::uint32 &GetShadowMap()
		{
			return m_shadow_map;
		}
		KCL::uint32 GetDepthComponentFormat() const
		{
			return m_depth_component_format;
		}
		FitMode GetFitMode() const
		{
			return m_fit_mode;
		}
		SelectionMode GetSelectionMode() const
		{
			return m_selection_mode;
		}
		float GetShadowNear() const
		{
			return m_shadow_near;
		}
		float GetShadowFar() const
		{
			return m_shadow_far;
		}
		const KCL::Vector3D &GetDebugColor(KCL::uint32 cascade_index) const
		{
			return m_frustums[cascade_index].color;
		}

	private:
		struct Frustum
		{
			// Z split planes (-view space)
			float near_distance;
			float far_distance;

			// Points of the frustum split
			KCL::Vector3D points[8];

			// View space AABB
			KCL::Vector3D view_space_aabb_min;
			KCL::Vector3D view_space_aabb_max;

			// Light space AABB
			KCL::Vector3D light_space_aabb_min;
			KCL::Vector3D light_space_aabb_max;

			KCL::Vector3D world_space_obb[8];
			KCL::Vector4D cull_planes[8];

			// Bounding sphere
			float radius;
			KCL::Vector3D target;

			// Shadow camera
			KCL::Camera2 camera;

			//  Debugging color
			KCL::Vector3D color;
		};

		void InitNGLResources();
		void Update();
		void SplitFrustumsLogaritmic();
		void ExecuteFrustumCull(KCL::uint32 cascade_index);

		static void ExpandBoundingBox(KCL::Vector3D &min, KCL::Vector3D &max, const KCL::Vector3D &p);
		static KCL::Vector4D CreatePlane(const KCL::Vector3D &a, const KCL::Vector3D &b);
		static KCL::Vector4D CreatePlane(const KCL::Vector3D &a, const KCL::Vector3D &b, const KCL::Vector3D &c);
		static void CreateCullPlanes(const KCL::Vector3D &min, const KCL::Vector3D &max, const KCL::Matrix4x4 &matrix, KCL::Vector4D *planes);

		FitMode m_fit_mode;
		SelectionMode m_selection_mode;

		SceneBase *m_scene;

		float m_shadow_near;
		float m_shadow_far;
		float m_shadow_negative_range;
		float m_shadow_positive_range;

		KCL::Vector3D m_light_dir;
		const KCL::Camera2 *m_scene_camera;
		MeshFilter* m_shadow_mesh_filter;

		KCL::uint32 m_shadow_map_size;
		NGL_format m_depth_component_format;

		KCL::uint32 m_cascade_count;
		std::vector<Frustum> m_frustums;
		std::vector<FrustumCull*> m_frustum_culls;
		KCL::Vector3D m_view_frustum_points[8];
		KCL::Vector4D m_frustum_distances;

		KCL::Matrix4x4 m_bias_matrix;

		KCL::uint32 m_shadow_caster_shader;
		KCL::uint32 m_shadow_caster_alpha_test_shader;
		KCL::uint32 m_shadow_caster_skeletal_shader;
		KCL::uint32 m_shadow_caster_skeletal_alpha_test_shader;

		std::vector<KCL::Matrix4x4> m_shadow_matrices;
		std::vector<KCL::uint32> m_shadow_renderers;
		KCL::uint32 m_shadow_map;
	};
};

#endif
