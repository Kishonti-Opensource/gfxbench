/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_OVR_MESH_FILTERS_H
#define GFXB_OVR_MESH_FILTERS_H

#include "common/gfxb_frustum_cull.h"

namespace GFXB
{
	class OvrMainMeshFilter : public MeshFilter
	{
	public:
		enum MeshType
		{
			MESH_OPAQUE = 0,
			MESH_SKY_LEFT, MESH_SKY_RIGHT,
			MESH_TYPE_MAX
		};

		OvrMainMeshFilter();
		void SetAnimationTime(KCL::uint32 animation_time);

		virtual KCL::int32 FilterMesh(KCL::Camera2 *camera, KCL::Mesh *mesh, KCL::OverlapResult overlap_result) override;
		virtual KCL::uint32 GetMaxMeshTypes() override;
		virtual FrustumCull::PFN_MeshCompare GetMeshSortFunction(KCL::uint32 mesh_Type) override;

	private:
		KCL::uint32 m_animation_time;
	};
}

#endif