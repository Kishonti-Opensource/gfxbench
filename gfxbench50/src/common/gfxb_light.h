/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_LIGHT_H
#define GFXB_LIGHT_H

#include "kcl_light2.h"

namespace GFXB
{
	class ShadowMap;
	class MeshFilter;
	class Lightshaft;
	class LightFactory;

	class Light : public KCL::Light
	{
	public:
		Light(LightFactory *factory, const std::string& light_name, Node *parent, Object *owner);
		virtual ~Light();

		void Animate(KCL::uint32 animation_time);

		virtual std::string GetParameterFilename() const override;
		virtual void Serialize(JsonSerializer& s) override;

		ShadowMap *GetShadowMap() const;
		void SetShadowMap(ShadowMap* shadow_map) { m_shadow_map = shadow_map; }

		Lightshaft *GetLightshaft() const;

		KCL::AABB m_aabb;
		KCL::Vector3D m_pos;
		KCL::Vector3D m_dir;

		KCL::Vector3D m_animated_color;

		std::string m_foreign_shadow_light_name;
		float m_shadow_near;
		float m_shadow_far;

		float m_intensity_multiplier;

		float m_uniform_intensity;
		KCL::Vector4D m_uniform_pos;
		KCL::Vector4D m_uniform_dir;
		KCL::Vector4D m_uniform_color;
		KCL::Vector4D m_uniform_attenuation_parameters;

		KCL::Vector4D m_uniform_spot_cos;
		KCL::Matrix4x4 m_uniform_shape_world_pom;
		float m_spot_focus_length;

		KCL::uint32 m_frame_counter;

	private:
		LightFactory *m_light_factory;
		ShadowMap *m_shadow_map;
		Lightshaft *m_lightshaft;
	};
}

#endif
