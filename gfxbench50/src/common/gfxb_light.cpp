/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_light.h"
#include "gfxb_lightshaft.h"
#include "gfxb_factories.h"
#include "gfxb_scene_base.h"
#include "gfxb_shadow_map.h"
#include "gfxb_shapes.h"

#include <kcl_os.h>

using namespace GFXB;


Light::Light(LightFactory *factory, const std::string& light_name, Node *parent, Object *owner) : KCL::Light(light_name, parent, owner)
{
	m_light_factory = factory;
	m_lightshaft = new Lightshaft(light_name + "_lightshaft");
	m_shadow_map = nullptr;
	m_shadow_near = 0.01f;
	m_shadow_far = 10.0f;
	m_spot_focus_length = 0.0f;
	m_uniform_intensity = 0.0f;
	m_intensity_multiplier = 1.0f;
	m_frame_counter = 0;
}


Light::~Light()
{
	delete m_lightshaft;
}


void Light::Animate(KCL::uint32 animation_time)
{
	if (m_visible == false)
	{
		return;
	}

	float radius = 0.0f;
	float attenuation = 0.0f;

	if (m_light_factory->GetScene()->IsWarmup())
	{
		m_uniform_intensity = 1.0f;
		m_animated_color.set(1.0f, 1.0f, 1.0f);
		m_radius = 0.1f;
		attenuation = 0.1f;
	}
	else
	{
		m_uniform_intensity = GetAnimatedIntensity(animation_time) * m_intensity_multiplier;
		if (m_uniform_intensity <= 0.0f)
		{
			m_visible = false;
			return;
		}
		m_animated_color = GetAnimatedColor(animation_time);
		radius = GetAnimatedRadius(animation_time);
		attenuation = GetAnimatedAttenuation(animation_time);
	}

	m_pos.set(m_world_pom[12], m_world_pom[13], m_world_pom[14]);
	m_uniform_pos.set(m_pos, 1.0f);

	m_dir.set(m_world_pom.v[8], m_world_pom.v[9], m_world_pom.v[10]);
	m_dir.normalize();

	KCL::Vector3D up(m_world_pom.v[4], m_world_pom.v[5], m_world_pom.v[6]);
	up.normalize();

	m_spot_focus_length = 0.0f;

	switch (m_light_shape->m_light_type)
	{
		case KCL::LightShape::DIRECTIONAL:
		{
			if( m_light_shape->m_box_light )
			{
				KCL::Matrix4x4 scale_matrix;
				KCL::Matrix4x4::Scale(scale_matrix, KCL::Vector3D(m_light_shape->m_width, m_light_shape->m_height, m_light_shape->m_depth));

				m_uniform_shape_world_pom = scale_matrix * m_world_pom;

				m_aabb.Reset();
				for(KCL::uint32 i = 0; i < 8; i++)
				{
					mult4x4(m_uniform_shape_world_pom, Shapes::m_cube_vertices[i], m_frustum_vertices[i]);
					m_aabb.Merge(m_frustum_vertices[i]);
				}
			}

			break;
		}

		case KCL::LightShape::OMNI:
		{
			KCL::Vector3D local_radius(radius, radius, radius);

			m_aabb.SetWithHalfExtentCenter(local_radius, m_pos);

			m_uniform_shape_world_pom = m_world_pom;
			m_uniform_shape_world_pom.scale(local_radius);
			break;
		}

		case KCL::LightShape::SPOT:
		{
			{
				// Calculate the AABB of the light
				m_light_projection.identity();

				KCL::Matrix4x4::PerspectiveGL(m_light_projection, m_light_shape->m_fov, 1.0f, 0.01f, radius);
				KCL::Matrix4x4::Invert4x4(m_light_projection, m_inv_light_projection);

				KCL::Matrix4x4 frustum_matrix = m_inv_light_projection * m_world_pom;
				m_aabb.Reset();
				for (KCL::uint32 i = 0; i < 8; i++)
				{
					mult4x4(frustum_matrix, Shapes::m_cube_vertices[i], m_frustum_vertices[i]);
					m_aabb.Merge(m_frustum_vertices[i]);
				}
			}

			float half_spot_angle = KCL::Math::Rad(m_light_shape->m_fov * 0.5f);

			m_spot_focus_length = radius * tanf(half_spot_angle) * 1.2f; // 1.2 is: extra opening to counter low tess-factor of the cone

			m_uniform_spot_cos.x = cosf(half_spot_angle);
			m_uniform_spot_cos.y = 1.0f / (1.0f - m_uniform_spot_cos.x);

			m_uniform_shape_world_pom.zero();
			m_uniform_shape_world_pom.v33 = radius;
			m_uniform_shape_world_pom.v11 = m_uniform_shape_world_pom.v22 = m_spot_focus_length;
			m_uniform_shape_world_pom.v43 = -m_uniform_shape_world_pom.v33;	// Translate so the top is at the origo
			m_uniform_shape_world_pom.v44 = 1.0f;

			m_uniform_shape_world_pom = m_uniform_shape_world_pom * m_world_pom;
			break;
		}

		default:
			INFO("Unkown light type: %d", m_light_shape->m_light_type);
			break;
	}

	m_uniform_dir = KCL::Vector4D(m_dir.x, m_dir.y, m_dir.z, radius);
	m_uniform_color = KCL::Vector4D(m_animated_color * m_uniform_intensity, 1.0f);

	m_uniform_attenuation_parameters.x = 1.0f / (radius * radius);							// inverse radius
	m_uniform_attenuation_parameters.y = KCL::Max(attenuation, 0.01f);						// distance attenuation
	m_uniform_attenuation_parameters.z = KCL::Max(m_light_shape->m_fov_attenuation, 0.01f);	// fov attenuation
}


std::string Light::GetParameterFilename() const
{
	return "lights/" + m_light_name + ".json";
}


void Light::Serialize(JsonSerializer& s)
{
	KCL::Light::Serialize(s);

	s.Serialize("light_foreign_shadow_light_name", m_foreign_shadow_light_name);
	s.Serialize("light_shadow_near", m_shadow_near);
	s.Serialize("light_shadow_far", m_shadow_far);
	s.Serialize("light_intensity_multiplier", m_intensity_multiplier);

	// Lightshaft
	if (m_has_lightshaft)
	{
		if (s.IsWriter)
		{
			m_lightshaft->SaveParameters();
		}
		else
		{
			m_lightshaft->LoadParameters();
		}
	}
}


ShadowMap* Light::GetShadowMap() const
{
	return m_shadow_map;
}


Lightshaft *Light::GetLightshaft() const
{
	return m_lightshaft;
}
