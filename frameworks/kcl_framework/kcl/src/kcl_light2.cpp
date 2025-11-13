/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_light2.h"
#include "kcl_io.h"

#include <sstream>
#include <string>

using namespace KCL;


KCL::Light::Light( const std::string& light_name, Node *parent, Object *owner) : Node( light_name, LIGHT, parent, owner), m_light_name( light_name)
{
	m_light_shape = nullptr;
	m_type = LIGHT;
	m_constAtt = 1;
	m_linearAtt = 0;
	m_quadAtt = 0;
#ifdef GFXB_DEPRECATED
	m_intensity = 1;
	m_radius = -1.0f;
	m_spotAngle = 180;
	m_light_type = OMNI;
	m_is_shadow_caster = false;
	m_diffuse_color.set( 1, 1, 1);
#endif
	m_intensity_flag = INTENSITY_OVERRIDDEN_BY_TRACK;
	m_spotExponent = 0;
	m_has_glow = false;
	m_has_lensflare = false;
	m_is_flickering = false;
	m_intensity_track = nullptr;
	m_radius_track = nullptr;
	m_attenuation_track = nullptr;
	m_color_track = nullptr;
	t = 0.0f;
	tb = 0.0f;
	m_has_lightshaft = false;
}


KCL::Light::~Light()
{
	delete m_intensity_track;
	delete m_radius_track;
	delete m_attenuation_track;
	delete m_color_track;
}


float KCL::Light::GetAnimatedIntensity(KCL::uint32 animation_time)
{
	if (m_intensity_track == nullptr)
	{
		return m_light_shape->m_intensity;
	}
	else
	{
		// Read the intensity from the animation track
		float time = float(animation_time) * 0.001f;
		float time_base = 0;
		KCL::Vector4D v;
		KCL::_key_node::Get(v, m_intensity_track, time, time_base, true);

		switch (m_intensity_flag)
		{
			case INTENSITY_OVERRIDDEN_BY_TRACK:
				return v.x;

			case INTENSITY_MULTIPLIED_BY_TRACK:
				return v.x * m_light_shape->m_intensity;

			default:
				INFO("Unknown light intensity flag: %d for light: %s", m_intensity_flag, m_name.c_str());
				return m_light_shape->m_intensity;
		}
	}
}


float KCL::Light::GetAnimatedRadius(KCL::uint32 animation_time)
{
	if (m_radius_track == nullptr)
	{
		return m_light_shape->m_radius;
	}
	else
	{
		// Read the radius from the animation track
		float time = float(animation_time) * 0.001f;
		float time_base = 0;
		KCL::Vector4D v;
		KCL::_key_node::Get(v, m_radius_track, time, time_base, true);
		return v.x;
	}
}


float KCL::Light::GetAnimatedAttenuation(KCL::uint32 animation_time)
{
	if (m_attenuation_track == nullptr)
	{
		return m_light_shape->m_attenuation;
	}
	else
	{
		// Read the intensity from the animation track
		float time = float(animation_time) * 0.001f;
		float time_base = 0;
		KCL::Vector4D v;
		KCL::_key_node::Get(v, m_attenuation_track, time, time_base, true);
		return v.x;
	}
}


KCL::Vector3D KCL::Light::GetAnimatedColor(KCL::uint32 animation_time)
{
	if (m_color_track == nullptr)
	{
		return m_light_shape->m_color;
	}
	else
	{
		float time = float(animation_time) * 0.001f;
		float time_base = 0;
		KCL::Vector4D v;
		KCL::_key_node::Get(v, m_color_track, time, time_base, true);

		return Vector3D(v.x, v.y, v.z);
	}
}


void KCL::Light::Serialize(JsonSerializer& s)
{
	Node::Serialize(s);

	s.Serialize("has_lightshaft", m_has_lightshaft);

	/*
#ifdef GFXB_DEPRECATED
	SerializeHexaColor(s, "light_color", m_diffuse_color);
	s.Serialize("light_radius", m_radius);
	s.Serialize("light_cast_shadow", m_is_shadow_caster);
	s.Serialize("light_type", m_light_type);
#endif
	s.Serialize("light_intensity_flag", m_intensity_flag);
	s.Serialize("light_glow", m_has_glow);
	s.Serialize("light_lens_flare", m_has_lensflare);
	*/
}


std::string KCL::Light::GetParameterFilename() const
{
	return "lights/" + m_light_name + ".json";
}

/************************************************************************/
/* LightFactory                                                         */
/************************************************************************/
KCL::LightFactory::~LightFactory()
{
}

/************************************************************************/
/* LightShape                                                           */
/************************************************************************/
KCL::LightShape::LightShape(const std::string& light_name)
{
	m_light_name = light_name;
	m_intensity = 0.0f;
	m_light_type = OMNI;
	m_fov = 0.0f;

	m_is_shadow_caster = false;
	m_color = Vector3D(1.0f, 1.0f, 1.0f);
	m_radius = 0.0f;
	m_width = 0.0f;
	m_height = 0.0f;
	m_depth = 0.0f;
	m_attenuation = 1.0f;
	m_fov_attenuation = 1.0f;
	m_has_glow = false;
	m_has_fire = false;
	m_box_light = false;
}


KCL::LightShape::~LightShape()
{
}


std::string KCL::LightShape::GetParameterFilename() const
{
	return "lights/" + m_light_name + ".json";
}


void KCL::LightShape::Serialize(JsonSerializer& s)
{
	if (s.IsWriter && m_fov > 179.0f)
	{
		m_fov = 179.0f;
	}
	SerializeHexaColor(s, "light_color", m_color);
	s.Serialize("light_intensity", m_intensity);
	s.Serialize("light_radius", m_radius);
	s.Serialize("light_type", m_light_type);
	s.Serialize("light_fov", m_fov);
	s.Serialize("light_attenuation", m_attenuation);
	s.Serialize("light_fov_attenuation", m_fov_attenuation);
	s.Serialize("light_shape_name", m_light_name);
	s.Serialize("light_has_fire", m_has_fire);
	s.Serialize("light_glow", m_has_glow);
	s.Serialize("light_width", m_width);
	s.Serialize("light_height", m_height);
	s.Serialize("light_depth", m_depth);
	s.Serialize("light_cast_shadow", m_is_shadow_caster);
	//s.Serialize("light_shape_name", m_light_name);//this one passed by constructor

	m_box_light = m_width > 0.0f && m_height > 0.0f  && m_depth > 0.0f && m_light_type == DIRECTIONAL;
}


bool KCL::LightShape::operator==(const KCL::LightShape &rhs) const
{
	return m_light_name == rhs.m_light_name &&
		m_intensity == rhs.m_intensity &&
		m_light_type == rhs.m_light_type &&
		m_fov == rhs.m_fov &&
		m_attenuation == rhs.m_attenuation &&
		m_fov_attenuation == rhs.m_fov_attenuation &&
		m_is_shadow_caster == rhs.m_is_shadow_caster &&
		m_has_fire == rhs.m_has_fire &&
		m_has_glow == rhs.m_has_glow &&
		m_color == rhs.m_color &&
		m_radius == rhs.m_radius &&
		m_width == rhs.m_width &&
		m_height == rhs.m_height &&
		m_depth == rhs.m_depth;
}


bool KCL::LightShape::operator!=(const KCL::LightShape &rhs) const
{
	return !(*this==rhs);
}
