/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_LIGHT2_H
#define KCL_LIGHT2_H


#include <kcl_math3d.h>
#include <kcl_node.h>
#include "kcl_factory_base.h"
#include "kcl_texture.h"


/************************************************************************/
/* temporary solution until manhattan's lights will be refactored       */
/* #define GFXB_DEPRECATED  //this added to CMakeLists.txt              */
/************************************************************************/
#ifndef GFXB_DEPRECATED
#define GFXB_DEPRECATED
#endif

namespace KCL
{
	struct _key_node;

	class LightShape : public KCL::Serializable
	{
	public:
		enum LightShapeType
		{
			DIRECTIONAL = 0,
			OMNI,
			SPOT,
			MAX_LIGHT_SHAPES,
		};
		LightShapeType m_light_type;

		std::string m_light_name;

		Vector3D m_color;
		float m_intensity;
		float m_fov;
		float m_radius;
		float m_width;
		float m_height;
		float m_depth;
		float m_attenuation;
		float m_fov_attenuation;
		bool m_is_shadow_caster;
		bool m_has_glow;
		bool m_has_fire;
		bool m_box_light;

		LightShape(const std::string& light_name);
		virtual  ~LightShape();

		virtual std::string GetParameterFilename() const;
		virtual void Serialize(JsonSerializer& s);

		bool operator==(const KCL::LightShape &rhs) const;
		bool operator!=(const KCL::LightShape &rhs) const;
	};


	class Light : public Node
	{
	public:
		enum LightType
		{
			AMBIENT = 128,
			DIRECTIONAL = 129,
			OMNI = 130,
			SPOT = 131,
			SMALL_OMNI = 132,
			SSAO = 133,
			SHADOW_DECAL = 134
		};

		LightShape* m_light_shape;

		virtual ~Light();
#ifdef GFXB_DEPRECATED
		float m_intensity;
		float m_radius;
		LightType m_light_type;
		float m_spotAngle;
		Vector3D m_diffuse_color;
		bool m_is_shadow_caster;
#endif
		enum LightIntensityFlag
		{
			INTENSITY_OVERRIDDEN_BY_TRACK = 0,
			INTENSITY_MULTIPLIED_BY_TRACK
		};
		LightIntensityFlag m_intensity_flag;

		float m_constAtt;
		float m_linearAtt;
		float m_quadAtt;
		Vector3D m_ambient_color;
		Vector3D m_dir;
		float m_spotExponent;
		bool m_has_glow;
		bool m_has_lensflare;
		bool m_is_flickering;
		bool m_has_lightshaft;

		float t, tb;
		_key_node *m_intensity_track;
		_key_node *m_radius_track;
		_key_node *m_attenuation_track;
		_key_node *m_color_track;

		std::vector<Mesh*> visible_meshes[2];

		std::string m_light_name;

		KCL::Vector3D m_frustum_vertices[8];
		KCL::Matrix4x4 m_light_projection;
		KCL::Matrix4x4 m_inv_light_projection;

		virtual float GetAnimatedIntensity(KCL::uint32 animation_time);
		virtual float GetAnimatedRadius(KCL::uint32 animation_time);
		virtual float GetAnimatedAttenuation(KCL::uint32 animation_time);
		virtual KCL::Vector3D GetAnimatedColor(KCL::uint32 animation_time);

		virtual void Serialize(JsonSerializer& s);
		virtual std::string GetParameterFilename() const;

	protected:
		Light ( const std::string& light_name, Node *parent, Object *owner);
	};


	class LightFactory : public KCL::FactoryBase
	{
	public:
		virtual ~LightFactory();
		virtual KCL::Light *Create(const std::string& light_name, Node *parent, Object *owner) = 0;
	};
}//namespace KCL

#endif //
