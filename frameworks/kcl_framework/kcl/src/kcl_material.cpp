/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_material.h>
#include <kcl_image.h>

using namespace KCL;


KCL::Material::Material(const char *name) : Object(name, MATERIAL), m_texture_array(NULL), m_translate_u_track(NULL), m_translate_v_track(NULL), m_emissive_track(nullptr)
{
	SetDefaults();
}

KCL::Material::~Material()
{
    //note: images are deleted by scene handler!
  	delete m_texture_array;	
	delete m_emissive_track;
	delete m_translate_u_track;
	delete m_translate_v_track;
}

void KCL::Material::SetDefaults()
{
	m_material_type = DEFAULT;
	for( KCL::uint32 i = 0; i < MAX_IMAGE_TYPE; i++)
	{
		m_textures[i] = 0;
	}
	m_diffuse_intensity = 1.0f;
	m_specular_intensity = 0.0f;
	m_reflect_intensity = 0.0f;
	m_specular_exponent = 1.0f;
	m_transparency = 0.0f;
	m_emissive_intensity = 0.0f;
	m_alpha_test_threshold = 0.5f;
	m_gloss = 0.001f;
	m_softness = 0.0f;
	m_dither_start_distance = 10.0f;
	m_dither_interval = 1.5f;

	m_is_shadow_caster = false;
    m_is_reflection_caster = false;
    m_is_shadow_only = false;
	m_is_transparent = false;
	m_is_decal = false;
	m_is_reflective = false;
    m_is_aniso = false;
    m_is_billboard = false;
    m_translucent_lighting_strength = 0.0f;
    m_is_two_sided = false;
	m_is_texture_density = false;
	m_is_debug_mipmap_level = false;
    m_is_tesselated = false;
    m_has_emissive_channel = false;
    m_is_car_paint = false;
    m_use_world_ao = false;
    m_has_car_ao = false;
	m_is_occluder = false;
	m_is_dithered = false;
	m_animation_time = 0.0f;
	m_animation_time_base = 0.0f;
	m_frame_when_animated = 0;

    m_tessellation_factor = KCL::Vector4D(1.0f,1.0f,0.0f,100000.0f); //multiplier, scale, bias, max-distance

	m_shader_names[0] = "";
	m_shader_names[1] = "";
    m_shader_names[2] = "";

	m_shader_defs[0].clear();
	m_shader_defs[1].clear();
    m_shader_defs[2].clear();

	m_fresnel_params = KCL::Vector3D(0.0f, 0.0f, 0.0f);

    m_displacement_mode = NO_DISPLACEMENT;
    m_opacity_mode = NO_OPACITY;
	
	// Delete dynamic members
	delete m_texture_array;
	m_texture_array = NULL;
	
	delete m_emissive_track;
	m_emissive_track = NULL;

	delete m_translate_u_track;
	m_translate_u_track = NULL;

	delete m_translate_v_track;
	m_translate_v_track = NULL;
}

void KCL::Material::InitImages()
{
	for( KCL::uint32 i = 0; i < MAX_IMAGE_TYPE; i++)
	{
        if( m_textures[i] && !m_textures[i]->isCommitted() )
		{
            m_textures[i]->commit();
		}
	}

    if (m_texture_array)
    {
        m_texture_array->commit();
    }
}

bool KCL::Material::GetTextureImageTypeByName(const std::string & name, KCL::Material::TextureImageType & type)
{
	if( name == "color" || name == "texture0")
	{
		type = KCL::Material::COLOR;
	}
	else if( name == "lightmap" || name == "texture1")
	{
		type = KCL::Material::LIGHTMAP2;
	}
	else if( name == "mask" || name == "specular" || name == "texture2")
	{
		type = KCL::Material::MASK;
	}
	else if( name == "normal" || name == "texture3")
	{
		type = KCL::Material::BUMP;
	}	
	else if( name == "emission" || name == "texture4")
	{
		type = KCL::Material::EMISSION;
	}
    else if(name == "aux0" || name == "texture5")
    {
        type = KCL::Material::AUX0;
    }
    else if( name == "aux1" || name == "texture6")
    {
        type = KCL::Material::AUX1;
    }
    else if( name == "aux2" || name == "texture7")
    {
        type = KCL::Material::AUX2;
    }
    else if( name == "aux3" || name == "texture8")
    {
        type = KCL::Material::AUX3;
    }
    else if( name == "aux4" || name == "texture9")
    {
        type = KCL::Material::AUX4;
    }
    else if( name == "aux5" || name == "texture10")
    {
        type = KCL::Material::AUX5;
    }
    else if( name == "aux6" || name == "texture11")
    {
        type = KCL::Material::AUX6;
    }
    else if( name == "aux7" || name == "texture12")
    {
        type = KCL::Material::AUX7;
    }
	else
	{
		return false;
	}

	return true;
}

int KCL::Material::GetShaderIndexTypeByName(const std::string & name)
{
	if (name == "shader" || name == "shader0")
	{
		return 0;
	}
	else if (name == "shader1")
	{
		return 1;
	}	
	else if (name == "shader2")
	{
		return 2;
	}	
	else
	{
		return -1;
	}
}

int KCL::Material::GetDefineIndexTypeByName(const std::string & name)
{
	if (name == "define" || name == "define0")
	{
		return 0;
	}
	else if (name == "define1")
	{
		return 1;
	}
	else if (name == "define2")
	{
		return 2;
	}
	else
	{
		return -1;
	}
}

bool KCL::Material::GetMaterialTypeByName(const std::string & name, MaterialType & type)
{	
	if( name == "default")
	{
		type = KCL::Material::DEFAULT;
	}
	else if( name == "sky")
	{
		type = KCL::Material::SKY;
	}
	else if( name == "water")
	{
		type = KCL::Material::WATER;
	}
	else if( name == "lightshaft")
	{
		type = KCL::Material::LIGHTSHAFT;
	}	
	else if( name == "foliage")
	{
		type = KCL::Material::FOLIAGE;
	}
    else if( name == "flame")
    {
        type = KCL::Material::FLAME;
    }
    else if( name == "fire")
    {
        type = KCL::Material::FIRE;
    }
    else if( name == "omilight")
    {
        type = KCL::Material::OMNILIGHT;
    }
	else if( name == "shadow_caster0")
    {
        type = KCL::Material::SHADOWCASTER0;
	}
	else if( name == "shadow_receiver0")
    {
        type = KCL::Material::SHADOWRECEIVER0;
    }
	else if( name == "shadow_caster1")
    {
        type = KCL::Material::SHADOWCASTER1;
    }
	else if( name == "shadow_receiver1")
    {
        type = KCL::Material::SHADOWRECEIVER1;
	}
	else if( name == "planar_reflection")
    {
        type = KCL::Material::PLANAR_REFLECTION;
    }
	else if( name == "smoke")
    {
        type = KCL::Material::SMOKE;
	}
	else if( name == "decal")
    {
        type = KCL::Material::DECAL;
    }        
	else if( name == "steam")
    {
        type = KCL::Material::STEAM;
    } 
	else if( name == "glass")
    {
        type = KCL::Material::GLASS;
    } 
	else if( name == "xray")
    {
        type = KCL::Material::XRAY;
    } 
	else
	{
		return false;
	}

	return true;	
}


void KCL::Material::Serialize(JsonSerializer& s)
{
	Object::Serialize(s);

	s.Serialize("material_type", m_material_type);
	s.Serialize("material_shader0", m_shader_names[0]);
	s.Serialize("material_opacity_mode", m_opacity_mode);
	s.Serialize("material_emissive_intensity", m_emissive_intensity);
	s.Serialize("material_has_emissive_channel", m_has_emissive_channel);
	s.Serialize("material_alpha_test_threshold", m_alpha_test_threshold);
	s.Serialize("material_gloss", m_gloss);
	s.Serialize("material_softness", m_softness);
	s.Serialize("material_two_sided", m_is_two_sided);
	s.Serialize("material_dithered", m_is_dithered);
	s.Serialize("material_dither_start_distance", m_dither_start_distance);
	s.Serialize("material_dither_interval", m_dither_interval);

	if (s.IsWriter == false)
	{
		m_is_transparent = m_opacity_mode == ALPHA_BLEND;
	}
}

KCL::MaterialFactory::~MaterialFactory()
{
}
