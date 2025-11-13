/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_scene5_mesh_filters.h"
#include "gfxb_scene5_mesh.h"

#include <kcl_mesh.h>
#include <kcl_material.h>

using namespace GFXB;

Scene5MainMeshFilter::Scene5MainMeshFilter()
{
	m_animation_time = 0;
}


void Scene5MainMeshFilter::SetAnimationTime(KCL::uint32 animation_time)
{
	m_animation_time = animation_time;
}


KCL::int32 Scene5MainMeshFilter::FilterMesh(KCL::Camera2 *camera, KCL::Mesh *mesh, KCL::OverlapResult overlap_result)
{
	if ((mesh->m_flags & KCL::Mesh::OF_SHADOW_ONLY) == KCL::Mesh::OF_SHADOW_ONLY)
	{
		// This mesh is only renderer in shadow pass
		return -1;
	}

	Scene5Mesh *scene5_mesh = (Scene5Mesh*)mesh;

	scene5_mesh->Animate(m_animation_time, camera);

	if (scene5_mesh->m_dither_value >= 1.0f)
	{
		// Fully dithered out
		return -1;
	}

	const KCL::Material *material = mesh->m_material;

	if (material->m_opacity_mode == KCL::Material::ALPHA_TEST)
	{
		return MESH_ALPHA_TESTED;
	}

	if (material->m_is_transparent)
	{
		if (!material->m_has_emissive_channel || scene5_mesh->m_emissive_intensity > 0.0f)
		{
			// Only render emissive meshes if emissive intensity is positive
			return MESH_TRANSPARENT;
		}
		else
		{
			return -1;
		}
	}
	else if (material->m_has_emissive_channel)
	{
		if (scene5_mesh->m_emissive_intensity > 0.0f)
		{
			// Only render emissive meshes if emissive intensity is positive
			return MESH_OPAQUE_EMISSIVE;
		}
		else
		{
			return MESH_OPAQUE;
		}
	}

	return MESH_OPAQUE;
}


KCL::uint32 Scene5MainMeshFilter::GetMaxMeshTypes()
{
	return MESH_TYPE_MAX;
}


FrustumCull::PFN_MeshCompare Scene5MainMeshFilter::GetMeshSortFunction(KCL::uint32 mesh_type)
{
	return mesh_type == MESH_TRANSPARENT ? FrustumCull::ReverseDepthCompare : FrustumCull::DepthCompare;
}


Scene5ShadowMeshFilter::Scene5ShadowMeshFilter()
{
	m_force_shadow_caster_all = false;
}


KCL::int32 Scene5ShadowMeshFilter::FilterMesh(KCL::Camera2 *camera, KCL::Mesh *mesh, KCL::OverlapResult overlap_result)
{
	if ((mesh->m_flags & KCL::Mesh::OF_SHADOW_CASTER) == 0 && !m_force_shadow_caster_all)
	{
		return -1;
	}

	if (mesh->m_material->m_opacity_mode == KCL::Material::ALPHA_BLEND)
	{
		return -1;
	}

	return 0;
}


KCL::uint32 Scene5ShadowMeshFilter::GetMaxMeshTypes()
{
	return 1;
}


FrustumCull::PFN_MeshCompare Scene5ShadowMeshFilter::GetMeshSortFunction(KCL::uint32 mesh_Type)
{
	return FrustumCull::DepthCompareAlphaTest;
}


void Scene5ShadowMeshFilter::SetForceShadowCasterAll(bool value)
{
	m_force_shadow_caster_all = value;
}


GFXB::Scene5CSMMeshFilter::Scene5CSMMeshFilter()
{
	m_force_shadow_caster_all = false;
	m_frame_counter = 0;
}


KCL::int32 Scene5CSMMeshFilter::FilterMesh(KCL::Camera2 *camera, KCL::Mesh *m, KCL::OverlapResult overlap_result)
{
	Scene5Mesh *mesh = (Scene5Mesh*)m;
	if (mesh->m_csm_frame_counter == m_frame_counter)
	{
		return -1;
	}

	if ((mesh->m_flags & KCL::Mesh::OF_SHADOW_CASTER) == 0 && !m_force_shadow_caster_all)
	{
		return -1;
	}

	if (mesh->m_material->m_opacity_mode == KCL::Material::ALPHA_BLEND)
	{
		return -1;
	}

	if (overlap_result == KCL::OVERLAP_INSIDE)
	{
		// TODO: this should not do for actors
		mesh->m_csm_frame_counter = m_frame_counter;
	}
	return 0;
}


KCL::uint32 Scene5CSMMeshFilter::GetMaxMeshTypes()
{
	return 1;
}


FrustumCull::PFN_MeshCompare Scene5CSMMeshFilter::GetMeshSortFunction(KCL::uint32 mesh_type)
{
	return FrustumCull::DepthCompareAlphaTest;
}


void Scene5CSMMeshFilter::SetForceShadowCasterAll(bool value)
{
	m_force_shadow_caster_all = value;
}


void Scene5CSMMeshFilter::IncreaseFrameCounter()
{
	m_frame_counter++;
}
