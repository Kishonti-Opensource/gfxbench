/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_FRUSTUM_CULL_H
#define GFXB_FRUSTUM_CULL_H

#include <vector>
#include <set>
#include <kcl_math3d.h>
#include <kcl_room.h>

namespace KCL
{
	class Camera2;
	class SceneHandler;
	class Mesh;
	class EnvProbe;
	class XRoom;
}

namespace GFXB
{
	class MeshFilter;
	class Light;

	class FrustumCull
	{
	public:
		static const KCL::uint32 MAX_INSTANCES = 64;

		FrustumCull(KCL::SceneHandler *scene, MeshFilter *mesh_filter);
		virtual ~FrustumCull();

		void Reset();
		void Cull(KCL::Camera2 *camera);
		float GetNear() const;
		float GetFar() const;

		void SetDefaultNearFar(float default_near, float default_far);
		void SetPartitionsEnabled(bool value);
		bool IsPartitionsEnabled() const;
		void SetPVSEnabled(bool value);
		bool IsPVSEnabled() const;
		void SetCullLights(bool value);
		void SetCullActors(bool value);
		void SetCullProbes(bool value);
		void SetCullWithNearFar(bool value);
		void ExcludeActor(KCL::Actor *actor);

		KCL::XRoom *m_camera_room;
		std::set<KCL::XRoom*> m_visible_rooms;
		std::vector<std::vector<KCL::Mesh*>> m_visible_meshes;
		std::vector<KCL::Light*> m_visible_lights;
		std::vector<KCL::Light*> m_visible_light_shafts;

		KCL::uint32 m_visible_actors;

		std::vector<KCL::EnvProbe*> m_visible_probes;
		std::vector<KCL::EnvProbe*> m_visible_inside_probes;
		std::vector<KCL::EnvProbe*> m_visible_outside_probes;

		struct MeshSortInfo
		{
			KCL::Mesh *m_mesh;
			float m_vertex_center_dist;

			MeshSortInfo()
			{
				m_mesh = nullptr;
				m_vertex_center_dist = 0.0f;
			}
		};
		typedef bool(*PFN_MeshCompare)(const FrustumCull::MeshSortInfo *A, const FrustumCull::MeshSortInfo *B);

		// Front to back
		static bool DepthCompare(const MeshSortInfo *A, const MeshSortInfo *B);

		// | Opaque front to back | Alpha test front to back |
		static bool DepthCompareAlphaTest(const MeshSortInfo *A, const MeshSortInfo *B);

		// Back to front
		static bool ReverseDepthCompare(const MeshSortInfo *A, const MeshSortInfo *B);

		static void ResetInstanceCounter();

	private:
		static KCL::uint32 s_instance_counter;

		KCL::uint32 m_instance_id;

		KCL::SceneHandler *m_scene;
		KCL::Camera2 m_camera;
		MeshFilter *m_mesh_filter;

		float m_near;
		float m_far;
		float m_default_near;
		float m_default_far;

		bool m_partition_enabled;
		bool m_pvs_enabled;
		bool m_cull_lights;
		bool m_cull_actors;
		bool m_cull_probes;
		bool m_cull_with_near_far;

		std::set<KCL::Actor*> m_excluded_actors; // TODO: remove this

		KCL::uint32 m_frame_counter;
		std::vector<KCL::XRoom*> m_cull_path;
		std::vector<KCL::uint32> m_actor_frame_counters;

		std::vector<MeshSortInfo> m_sort_meshes;
		std::vector<MeshSortInfo*> m_sort_mesh_pointers;

		struct LightSortInfo
		{
			KCL::Light *m_light;
			float m_distance;
		};
		std::vector<LightSortInfo> m_sort_lights;
		std::vector<LightSortInfo*> m_sort_light_pointers;

		FrustumCull(const FrustumCull &other) { }

		void CullActors();
		void CullLight(GFXB::Light *light);
		void CullRoom(KCL::XRoom *room, const KCL::Vector4D *planes, KCL::uint32 plane_count, bool partitions_enabled);
		bool IsRoomInPath(KCL::XRoom *r);
		static bool Inside(const KCL::Vector3D &p, const KCL::Vector4D &plane);
		static void Intersect(const KCL::Vector3D &p0, const KCL::Vector3D &p1, const KCL::Vector4D &plane, KCL::Vector3D &result);

		void SortMeshes(std::vector<KCL::Mesh*> &meshes, PFN_MeshCompare sort_function);
		void SortLights(std::vector<KCL::Light*> &lights);

		static bool LightReverseDepthCompare(const FrustumCull::LightSortInfo *A, const FrustumCull::LightSortInfo *B);

		inline void AdjustNearFar(const KCL::AABB &aabb)
		{
			// Adjust camera near-far
			KCL::Vector2D mm = aabb.DistanceFromPlane(m_camera.GetCullPlane(KCL::CULLPLANE_NEAR));

			if (mm.y != mm.y || mm.y >= FLT_MAX)
			{
#ifdef _DEBUG
				printf("AdjustNearFar value overflow");
#endif
				return;
			}

			m_near = KCL::Min(m_near, mm.x);
			m_far = KCL::Max(m_far, mm.y);
		}
	};


	class MeshFilter
	{
	public:
		virtual ~MeshFilter() {}

		// Gets the mesh type or -1 if the mesh should be culled (for example: shadow only mesh in main pass)
		virtual KCL::int32 FilterMesh(KCL::Camera2 *camera, KCL::Mesh *mesh, KCL::OverlapResult overlap_result) = 0;

		// Gets the number of mesh types (Opaque, Transparent...)
		virtual KCL::uint32 GetMaxMeshTypes() = 0;

		// Gets the sort function for the mesh type. Returns nullptr if sort is not needed.
		virtual FrustumCull::PFN_MeshCompare GetMeshSortFunction(KCL::uint32 mesh_type) = 0;
	};


	class PassthroughMeshFilter : public MeshFilter
	{
	public:
		virtual KCL::int32 FilterMesh(KCL::Camera2 *camera, KCL::Mesh *mesh, KCL::OverlapResult overlap_result) override;
		virtual KCL::uint32 GetMaxMeshTypes() override;
		virtual FrustumCull::PFN_MeshCompare GetMeshSortFunction(KCL::uint32 mesh_type) override;
	};

}

#endif
