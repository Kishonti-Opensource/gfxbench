/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_ovr_mesh_filters.h"
#include "gfxb_ovr_mesh.h"

#include <kcl_mesh.h>
#include <kcl_material.h>

using namespace GFXB;

OvrMainMeshFilter::OvrMainMeshFilter()
{
	m_animation_time = 0;
}


void OvrMainMeshFilter::SetAnimationTime(KCL::uint32 animation_time)
{
	m_animation_time = animation_time;
}


KCL::int32 OvrMainMeshFilter::FilterMesh(KCL::Camera2 *camera, KCL::Mesh *mesh, KCL::OverlapResult overlap_result)
{
	if ((mesh->m_flags & KCL::Mesh::OF_SHADOW_ONLY) == KCL::Mesh::OF_SHADOW_ONLY)
	{
		return -1;
	}

	const OvrMesh* m = (OvrMesh*)mesh;

	if( m->m_is_left_eye_sky )
	{
		return MESH_SKY_LEFT;
	}
	else if( m->m_is_right_eye_sky )
	{
		return MESH_SKY_RIGHT;
	}
	else
	{
		return MESH_OPAQUE;
	}
}


KCL::uint32 OvrMainMeshFilter::GetMaxMeshTypes()
{
	return MESH_TYPE_MAX;
}


FrustumCull::PFN_MeshCompare OvrMainMeshFilter::GetMeshSortFunction(KCL::uint32 mesh_type)
{
	return /*mesh_type == MESH_TRANSPARENT ? FrustumCull::ReverseDepthCompare : */FrustumCull::DepthCompareAlphaTest;
}