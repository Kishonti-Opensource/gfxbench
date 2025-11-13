/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_sun_light.h"
#include "common/gfxb_mesh_shape.h"
#include "common/gfxb_texture.h"
#include "common/gfxb_frustum_cull.h"
#include "common/gfxb_shadow_map.h"
#include <ngl.h>

using namespace GFXB;

SunLight::SunLight(KCL::SceneHandler *scene, MeshFilter *shadow_mesh_filter) : m_light(0)
{
	m_scene = scene;

	const KCL::uint32 shadow_map_size = 512;
	m_shadow_map = new ShadowMap();
	m_shadow_map->Init("sun_shadow", scene, ShadowMap::DIRECTIONAL, shadow_map_size, shadow_map_size, NGL_D24_UNORM, shadow_mesh_filter);
}


SunLight::~SunLight()
{
	delete m_shadow_map;
}


void SunLight::Update()
{
	m_light = FindSunLight();

	if (m_light)
	{
		m_light_dir = KCL::Vector3D(m_light->m_world_pom.v[8], m_light->m_world_pom.v[9], m_light->m_world_pom.v[10]);
	}
	else
	{
		m_light_dir.set(0.7f, 0.8f, 0.2f);
	}

	/*
	{
		m_light_dir.set(0.0f, 1.0f, 0.0f);
		m_light_dir.normalize();
		m_light->m_world_pom.v[8] = m_light_dir.x;
		m_light->m_world_pom.v[9] = m_light_dir.y;
		m_light->m_world_pom.v[10] = m_light_dir.z;
	}
	*/


	m_light_dir.normalize();
	m_scene->m_light_dir = m_light_dir;

	//m_camera.Ortho(-20.0f, 20.0f, -20.0f, 20.0f, -100.0f, 100.0f);
	//m_shadow_map->GetCamera().Ortho(m_scene->m_aabb.GetMinVertex().x, m_scene->m_aabb.GetMaxVertex().x, m_scene->m_aabb.GetMinVertex().z, m_scene->m_aabb.GetMaxVertex().z, -20.0f, 45.0f);
	m_shadow_map->GetCamera().Ortho(m_scene->m_aabb.GetMinVertex().x, m_scene->m_aabb.GetMaxVertex().x, m_scene->m_aabb.GetMinVertex().z, m_scene->m_aabb.GetMaxVertex().z, -10.0f, 15.0f);
	m_shadow_map->GetCamera().LookAt(KCL::Vector3D(0.0f, 0.0f, 0.0f), -m_light_dir, KCL::Vector3D(0.0f, 1.0f, 0.0f));
	m_shadow_map->GetCamera().Update();
}


void SunLight::ShadowRender(KCL::uint32 command_buffer)
{
	m_shadow_map->Render(command_buffer);
}


KCL::Light *SunLight::FindSunLight()
{
	KCL::Actor *actor;
	KCL::Light *light;
	for (size_t i = 0; i < m_scene->m_actors.size(); i++)
	{
		actor = m_scene->m_actors[i];
		for (size_t j = 0; j < actor->m_lights.size(); j++)
		{
			light = actor->m_lights[j];
			if (light->m_light_shape->m_light_type == KCL::LightShape::DIRECTIONAL)
			{
				return light;
			}
		}
	}
	return nullptr;
}


ShadowMap *SunLight::GetShadowMap()
{
	return m_shadow_map;
}
