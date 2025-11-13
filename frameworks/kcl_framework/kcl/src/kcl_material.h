/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_MATERIAL_H
#define KCL_MATERIAL_H

#include <kcl_base.h>
#include <kcl_object.h>
#include <kcl_math3d.h>
#include <kcl_texture.h>
#include <kcl_animation4.h>
#include <kcl_factory_base.h>

//#include "test_error.h"

#include <string>
#include <vector>
#include <set>


namespace KCL
{
	class Material : public Object
	{
	public:		

		enum TextureImageType
		{
			COLOR = 0,
			LIGHTMAP2,
			MASK,
			BUMP,
			EMISSION,
			AUX0,
			AUX1,
			AUX2,
			AUX3,
            AUX4,
            AUX5,
            AUX6,
            AUX7,
			MAX_IMAGE_TYPE,
			ARRAY = MAX_IMAGE_TYPE
		};

		//static Material *Create(const char *name, const int m_scene_version = 8);
		//static std::vector<std::pair<KCL::Material::TextureImageType, std::string> > s_imageFilesToLoad;
		//static std::string s_ImagesDirectory;


		enum MaterialType
		{
			DEFAULT = 0,
			SKY,
			WATER,
			LIGHTSHAFT,
			FOLIAGE,
			FLAME,
			FIRE,
			OMNILIGHT,
			SHADOWCASTER0,
			SHADOWRECEIVER0,
			SHADOWCASTER1,
			SHADOWRECEIVER1,
			PLANAR_REFLECTION,
			SMOKE,
			DECAL,
			STEAM,
			GLASS,
			XRAY,
			GLOW
		};


		struct RepeatWrapmode
		{
			bool s;
			bool t;

			RepeatWrapmode() : s(false), t(false) {}
		};
		
        enum DisplacementMode
        {
            NO_DISPLACEMENT = 0,
            DISPLACEMENT_LOCAL,
			DISPLACEMENT_BEZIER,
            DISPLACEMENT_ABS = DISPLACEMENT_BEZIER
        };
        enum OpacityMode
        {
            NO_OPACITY = 0,
            ALPHA_TEST,
            ALPHA_BLEND,
        };

		MaterialType m_material_type;
        KCL::Texture* m_textures[MAX_IMAGE_TYPE];
		KCL::Vector3D m_image_scales[MAX_IMAGE_TYPE];
		RepeatWrapmode m_image_repeat_wrapmodes[MAX_IMAGE_TYPE];

		std::string m_image_array_name;
		KCL::Texture* m_texture_array;

		float m_diffuse_intensity;
		float m_specular_intensity;
		float m_reflect_intensity;
		float m_specular_exponent;
		float m_transparency;
		float m_emissive_intensity;
		float m_alpha_test_threshold;
		float m_gloss;
		float m_softness; //softness of transparent materials
		float m_dither_start_distance;
		float m_dither_interval;

        KCL::Vector4D m_tessellation_factor; //multiplier, scale, bias, max-distance

		std::string m_shader_names[3];
		std::set<std::string> m_shader_defs[3];

		KCL::Vector3D m_fresnel_params; //intensity, exponent, colorboost

        bool m_is_billboard;
        float m_translucent_lighting_strength;
		bool m_is_two_sided;
		bool m_is_texture_density;
		bool m_is_debug_mipmap_level;
		
		bool m_is_shadow_caster;
        bool m_is_reflection_caster;
        bool m_is_shadow_only;
		bool m_is_transparent;
		bool m_is_decal;
		bool m_is_reflective;
        bool m_is_aniso;
        bool m_is_tesselated;
        bool m_has_emissive_channel;
        bool m_is_car_paint;
        bool m_use_world_ao;
        bool m_has_car_ao;
		bool m_is_occluder;
		bool m_is_dithered;
        DisplacementMode m_displacement_mode;
        OpacityMode m_opacity_mode;
		
		std::string m_u_track_filename;
		std::string m_v_track_filename;

		_key_node *m_translate_u_track;
		_key_node *m_translate_v_track;
		_key_node *m_emissive_track;

		KCL::uint32 m_frame_when_animated;
		float m_animation_time;
		float m_animation_time_base;
		KCL::Vector2D m_uv_offset;
		std::set<std::string> m_defines;
		
		virtual void preInit( KCL::uint32 &texture_num, int type, int pass_type) { }
		virtual void postInit() {}

		virtual void LoadVideo( const char *filename) {}
		virtual void PlayVideo( float time_in_sec) {}
		virtual void DecodeVideo() {}
		void InitImages();

		virtual ~Material();		
		virtual void SetDefaults();

		static bool GetTextureImageTypeByName(const std::string &name, TextureImageType &type);
		static bool GetMaterialTypeByName(const std::string &name, MaterialType &type);
		static int GetShaderIndexTypeByName(const std::string &name);
		static int GetDefineIndexTypeByName(const std::string &name);

		virtual void Serialize(JsonSerializer& s);
		virtual std::string GetParameterFilename() const
		{
			return "materials/" + m_name + ".json";
		}
	protected:
		Material( const char *name);
	private:
		Material(const Material&);
		Material& operator=(const Material&);
	};

	class MaterialFactory : public KCL::FactoryBase
	{
	public:
		virtual ~MaterialFactory();
		virtual KCL::Material *Create(const std::string& material_name, Node *parent, Object *owner) = 0;
	};
}
#endif
