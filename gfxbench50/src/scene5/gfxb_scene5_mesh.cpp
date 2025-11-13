/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_scene5_mesh.h"
#include "common/gfxb_light.h"

#include <kcl_camera2.h>

using namespace GFXB;

Scene5Mesh::Scene5Mesh(const std::string &name, KCL::Node *parent, KCL::Object *owner) : Mesh(name, parent, owner)
{
	m_emissive_intensity = 0.0f;

	m_csm_frame_counter = 0;

	m_fire_time = 0.0f;
	m_fire_time_offset = 0;

	m_dither_value = -1.0f;
}


void Scene5Mesh::Animate(KCL::uint32 animation_time, const KCL::Camera2 *camera)
{
	m_emissive_intensity = m_material->m_emissive_intensity;

	if (m_material->m_emissive_track)
	{
		m_material->m_animation_time = animation_time / 1000.0f;
		KCL::Vector4D result;
		KCL::_key_node::Get(result, m_material->m_emissive_track, m_material->m_animation_time, m_material->m_animation_time_base, false);
		m_emissive_intensity = result.x;
	}

	if (m_material->m_material_type == KCL::Material::FIRE)
	{
		m_fire_time = (float)(animation_time + m_fire_time_offset) / 1000.0f;

		GFXB::Light* light = (GFXB::Light*)(m_parent);
		m_emissive_intensity = light->m_uniform_intensity;
		if (m_emissive_intensity > 0.0f)
		{
			const float fire_transition_range = 3.0f;
			m_emissive_intensity /= fire_transition_range;
			m_emissive_intensity = KCL::Min(m_emissive_intensity, 1.0f);
		}
	}

	m_dither_value = -1.0f;
	if (m_material->m_is_dithered)
	{
		KCL::Vector3D center;
		KCL::Vector3D size;
		m_aabb.CalculateHalfExtentCenter(size, center);

		const float d = KCL::Vector3D::distance(center, camera->GetEye());

		m_dither_value = (d - m_material->m_dither_start_distance) / KCL::Max(m_material->m_dither_interval, 0.01f);
	}
}


KCL::Mesh *Scene5MeshFactory::Create(const std::string &name, KCL::Node *parent, KCL::Object *owner)
{
	return new Scene5Mesh(name, parent, owner);
}