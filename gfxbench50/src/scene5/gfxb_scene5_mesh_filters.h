/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_MESH_FILTERS_H
#define GFXB_MESH_FILTERS_H

#include "common/gfxb_frustum_cull.h"

namespace GFXB
{
	class Scene5MainMeshFilter : public MeshFilter
	{
	public:
		enum MeshType
		{
			MESH_OPAQUE = 0,
			MESH_OPAQUE_EMISSIVE,
			MESH_ALPHA_TESTED,
			MESH_TRANSPARENT,
			MESH_TYPE_MAX
		};

		Scene5MainMeshFilter();
		void SetAnimationTime(KCL::uint32 animation_time);

		virtual KCL::int32 FilterMesh(KCL::Camera2 *camera, KCL::Mesh *mesh, KCL::OverlapResult overlap_result) override;
		virtual KCL::uint32 GetMaxMeshTypes() override;
		virtual FrustumCull::PFN_MeshCompare GetMeshSortFunction(KCL::uint32 mesh_Type) override;

	private:
		KCL::uint32 m_animation_time;
	};


	// Simple shadow map mesh filter
	class Scene5ShadowMeshFilter : public MeshFilter
	{
	public:
		Scene5ShadowMeshFilter();
		virtual KCL::int32 FilterMesh(KCL::Camera2 *camera, KCL::Mesh *mesh, KCL::OverlapResult overlap_result) override;
		virtual KCL::uint32 GetMaxMeshTypes() override;
		virtual FrustumCull::PFN_MeshCompare GetMeshSortFunction(KCL::uint32 mesh_Type) override;

		void SetForceShadowCasterAll(bool value);
	private:
		bool m_force_shadow_caster_all;
	};


	// Cascaded Shadow map filter
	class Scene5CSMMeshFilter : public MeshFilter
	{
	public:
		Scene5CSMMeshFilter();
		virtual KCL::int32 FilterMesh(KCL::Camera2 *camera, KCL::Mesh *mesh, KCL::OverlapResult overlap_result) override;
		virtual KCL::uint32 GetMaxMeshTypes() override;
		virtual FrustumCull::PFN_MeshCompare GetMeshSortFunction(KCL::uint32 mesh_Type) override;

		void SetForceShadowCasterAll(bool value);
		void IncreaseFrameCounter();

	private:
		bool m_force_shadow_caster_all;
		KCL::uint32 m_frame_counter;
	};
}

#endif