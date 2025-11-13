/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "krl_material.h"
#include <string>

using namespace std;
using namespace KCL;

KRL::Material::Material( const char *name) : KCL::Material(name) 
{	
	SetDefaults();
}

void KRL::Material::SetDefaults()
{
	KCL::Material::SetDefaults();
	m_shaders[0][0] = 0;
	m_shaders[0][1] = 0;
	m_shaders[0][2] = 0;
	m_shaders[1][0] = 0;
	m_shaders[1][1] = 0;
	m_shaders[1][2] = 0;
    m_tessellation_factor = KCL::Vector4D(16.0f, 1.0f, 0.0f, 100000.0f); //multiplier, scale, bias, max-distance
}


KCL::KCL_Status KRL::Material::InitShaders(const char* path, const std::string &max_joint_num_per_mesh)
{
	if( !m_name.c_str())
	{	
		return KCL_TESTERROR_FILE_NOT_FOUND;
	}



	KCL::KCL_Status result = KCL_TESTERROR_NOERROR;
	string vs_name[3];
	string fs_name[3];
	set<string> def2[3];

    if( strstr( path, "SV_30"))
    {
        def2[0].insert("SV_30");
        def2[1].insert("SV_30");
        def2[2].insert("SV_30");
    }
    if( strstr( path, "SV_31"))
    {
        def2[0].insert("SV_31");
        def2[1].insert("SV_31");
        def2[2].insert("SV_31");
    }

	def2[1].insert("SKELETAL" + max_joint_num_per_mesh);


#if defined HAVE_GLEW || defined HAVE_DX
	def2[0].insert("UBYTE_NORMAL_TANGENT");
	def2[1].insert("UBYTE_NORMAL_TANGENT");
#endif

	if( path)
	{
		if( strstr( path, "shadow_depth_map_depth"))
		{
			def2[0].insert("SHADOW_MAP");
			def2[1].insert("SHADOW_MAP");
		}
		if( strstr( path, "shadow_depth_map_color"))
		{
			def2[0].insert("SHADOW_MAP");
			def2[1].insert("SHADOW_MAP");

			def2[0].insert("RGB_ENCODED");
			def2[1].insert("RGB_ENCODED");
		}
		if( strstr( path, "soft_shadow"))
		{
			def2[0].insert("SOFT_SHADOW");
			def2[1].insert("SOFT_SHADOW");
		}
	}

	if( strstr( m_name.c_str(), "sky"))
	{
		vs_name[0] = "sky.vs";
		fs_name[0] = "sky.fs";
		vs_name[1] = "sky.vs";
		fs_name[1] = "sky.fs";
	}
	else if( strstr( m_name.c_str(), "planar_reflection"))
	{
		m_material_type = PLANAR_REFLECTION;

		m_transparency = 0.5f;

		def2[0].insert("TRANSPARENCY");
		def2[1].insert("TRANSPARENCY");
		def2[0].insert("MASK");
		def2[1].insert("MASK");
		def2[0].insert("DEP_TEXTURING");
		def2[1].insert("DEP_TEXTURING");

		vs_name[0] = "planar_reflection.vs";
		fs_name[0] = "planar_reflection.fs";

		vs_name[1] = "planar_reflection.vs";
		fs_name[1] = "planar_reflection.fs";
	}
	else if( strstr( m_name.c_str(), "omni_light_material"))
	{
		m_material_type = OMNILIGHT;

		vs_name[0] = "1.vs";
		fs_name[0] = "1.fs";

		vs_name[1] = "1.vs";
		fs_name[1] = "1.fs";
	}
	else if( strstr( m_name.c_str(), "shadow_caster_projective"))
	{
		m_material_type = SHADOWCASTER0;

		vs_name[0] = "shadow_caster0.vs";
		fs_name[0] = "shadow_caster0.fs";
		vs_name[1] = "shadow_caster0.vs";
		fs_name[1] = "shadow_caster0.fs";
	}
	else if( strstr( m_name.c_str(), "shadow_receiver_projective"))
	{
		m_material_type = SHADOWRECEIVER0;

		vs_name[0] = "shadow_receiver0.vs";
		fs_name[0] = "shadow_receiver0.fs";
		vs_name[1] = "shadow_receiver0.vs";
		fs_name[1] = "shadow_receiver0.fs";
	}
	else if( strstr( m_name.c_str(), "shadow_caster_depth_depth"))
	{
		m_material_type = SHADOWCASTER1;

		vs_name[0] = "shadow_caster0.vs";
		fs_name[0] = "shadow_caster0.fs";
		vs_name[1] = "shadow_caster0.vs";
		fs_name[1] = "shadow_caster0.fs";
	}
	else if( strstr( m_name.c_str(), "shadow_caster_depth_color"))
	{
		m_material_type = SHADOWCASTER1;

		vs_name[0] = "shadow_caster0.vs";
		fs_name[0] = "shadow_caster0.fs";
		vs_name[1] = "shadow_caster0.vs";
		fs_name[1] = "shadow_caster0.fs";
	}
	else if( strstr( m_name.c_str(), "shadow_receiver_depth_depth"))
	{
		m_material_type = SHADOWRECEIVER1;

		vs_name[0] = "shadow_receiver0.vs";
		fs_name[0] = "shadow_receiver1.fs";
		vs_name[1] = "shadow_receiver0.vs";
		fs_name[1] = "shadow_receiver1.fs";
	}
	else if( strstr( m_name.c_str(), "shadow_receiver_depth_color"))
	{
		m_material_type = SHADOWRECEIVER1;

		vs_name[0] = "shadow_receiver0.vs";
		fs_name[0] = "shadow_receiver1.fs";
		vs_name[1] = "shadow_receiver0.vs";
		fs_name[1] = "shadow_receiver1.fs";
	}
	else if( strstr( m_name.c_str(), "instanced_fire"))
	{
		m_material_type = FIRE;

		vs_name[0] = "fire.vs";
		fs_name[0] = "fire.fs";

		vs_name[1] = "fire.vs";
		fs_name[1] = "fire.fs";
	}
	else if( strstr( m_name.c_str(), "instanced_smoke"))
	{
		m_material_type = SMOKE;

		vs_name[0] = "fire.vs";
		fs_name[0] = "fire.fs";

		vs_name[1] = "fire.vs";
		fs_name[1] = "fire.fs";
	}
	else if( strstr( m_name.c_str(), "instanced_spark"))
	{
		m_material_type = FLAME;

		vs_name[0] = "spark.vs";
		fs_name[0] = "spark.fs";

		vs_name[1] = "spark.vs";
		fs_name[1] = "spark.fs";
	}
	else if( strstr( m_name.c_str(), "torch"))
	{
		vs_name[0] = "billboard.vs";
		fs_name[0] = "flame.fs";
		vs_name[1] = "billboard.vs";
		fs_name[1] = "flame.fs";
	}
	else if( strstr( m_name.c_str(), "glow"))
	{
		vs_name[0] = "billboard.vs";
		fs_name[0] = "glow.fs";
		vs_name[1] = "billboard.vs";
		fs_name[1] = "glow.fs";
	}
	else if( strstr( m_name.c_str(), "smoke"))
	{
		m_material_type = SMOKE;

		vs_name[0] = "particle.vs";
		fs_name[0] = "smoke.fs";
		vs_name[1] = "particle.vs";
		fs_name[1] = "smoke.fs";
	}
	else if( strstr( m_name.c_str(), "steam"))
	{
		m_material_type = STEAM;
		
		vs_name[0] = "particle.vs";
		fs_name[0] = "steam.fs";
		vs_name[1] = "particle.vs";
		fs_name[1] = "steam.fs";
	}
	else if( strstr( m_name.c_str(), "lens_flare"))
	{
		vs_name[0] = "lens_flare.vs";
		fs_name[0] = "lens_flare.fs";
		vs_name[1] = "lens_flare.vs";
		fs_name[1] = "lens_flare.fs";
	}
	else if( strstr( m_name.c_str(), "foliage_mat"))
	{
		vs_name[0] = "jungle_background.vs";
		fs_name[0] = "jungle_background.fs";
		vs_name[1] = "jungle_background.vs";
		fs_name[1] = "jungle_background.fs";
	}
	else if( m_name == "bike_track_decal_mat1" )
	{
		vs_name[0] = "bike_track.vs";
		fs_name[0] = "bike_track.fs";

		vs_name[1] = "bike_track.vs";
		fs_name[1] = "bike_track.fs";
	}
	else if( m_name == "mblur" )
	{
		def2[0].insert( "DEP_TEXTURING");
		def2[1].insert( "DEP_TEXTURING");

		vs_name[0] = "mblur.vs";
		fs_name[0] = "mblur.fs";

		vs_name[1] = "mblur.vs";
		fs_name[1] = "mblur.fs";
	}
	else
	{

		if( strstr( path, "SV_30") || strstr( path, "SV_31"))
		{
			if( m_is_transparent)
			{
				vs_name[0] = "forward.vs";
				fs_name[0] = "forward.fs";
				vs_name[1] = "forward.vs";
				fs_name[1] = "forward.fs";
			}
			else
			{
				vs_name[0] = "gbuffer.vs";
				fs_name[0] = "gbuffer.fs";
				vs_name[1] = "gbuffer.vs";
				fs_name[1] = "gbuffer.fs";
			}
		}
		else
		{
			vs_name[0] = "1.vs";
			fs_name[0] = "1.fs";
			vs_name[1] = "1.vs";
			fs_name[1] = "1.fs";
		}

		def2[0].insert("FOG");
		def2[1].insert("FOG");

		if( strstr( path, "SV_25"))
		{
			if( m_textures[LIGHTMAP2])
			{
				def2[0].insert("LIGHTMAP");
				def2[1].insert("LIGHTMAP");
			}
			else
			{
				if( m_textures[BUMP])
				{
					def2[0].insert("REFLECTION");
					def2[1].insert("REFLECTION");

					def2[0].insert("LIGHTING");
					def2[1].insert("LIGHTING");
				}
			}
		}
		else
		{
			if( m_textures[LIGHTMAP2])
			{
				def2[0].insert("LIGHTMAP");
				def2[1].insert("LIGHTMAP");
			}
			if( m_textures[BUMP])
			{
				def2[0].insert("REFLECTION");
				def2[1].insert("REFLECTION");

				def2[0].insert("LIGHTING");
				def2[1].insert("LIGHTING");
			}
		}

		if( m_textures[MASK])
		{
			def2[0].insert("MASK");
			def2[1].insert("MASK");
		}
		if( m_textures[EMISSION])
		{
			def2[0].insert("EMISSION");
			def2[1].insert("EMISSION");
		}

		if( m_translate_u_track || m_translate_v_track)
		{
			def2[0].insert("TRANSLATE_UV");
			def2[1].insert("TRANSLATE_UV");
		}
	}

	def2[0].insert( m_defines.begin(), m_defines.end());
	def2[1].insert( m_defines.begin(), m_defines.end());
	
	vs_name[2] = vs_name[0];
	fs_name[2] = fs_name[0];
	def2[2] = def2[0];
	def2[2].insert( "INSTANCING");

	for( KCL::uint32 i=0; i<3; i++)
	{
		set<string> def_set = def2[i];

		if( strstr( path, "SV_27"))
		{
			if(!strstr( m_name.c_str(), "foliage_mat"))
			{
				def_set.erase("ALPHA_TEST");
			}
		}

		m_shaders[0][i] = CreateShader( vs_name[i].c_str(), fs_name[i].c_str(), &def_set, result);
        
	}


#if 1
	/* bank 2 shaders for zprepass */
	for( KCL::uint32 i=0; i<2; i++)
	{
		def2[i].insert("ZPREPASS");

		m_shaders[1][i] = CreateShader( vs_name[i].c_str(), fs_name[i].c_str(), &def2[i], result);

	}
#endif

	return result;
}


Shader* KRL::Material::CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error)
{
	INFO("Material:CreateShader: unimplemented");
	return 0;
}
