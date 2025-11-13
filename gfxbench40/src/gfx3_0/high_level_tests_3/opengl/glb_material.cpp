/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_material.h"

#ifdef OPT_TEST_GFX40
#include "opengl/glb_material4.h"
#endif

#ifndef NOT_USE_GFXB
#include "xiph_decode/glb_ogg_decoder.h"
#endif

#include "kcl_scene_version.h"

namespace GLB
{
Material::Material( const char *name) : 
	KRL::Material(name),
	m_planar_map(0),
	m_ogg_decoder( 0),
	m_ubo_id (-1),
	m_ubo_offset (-1)
{	
}

GLB::Material::~Material() 
{
#ifndef NOT_USE_GFXB
	delete m_ogg_decoder;
#endif
}


void GLB::Material::LoadVideo( const char *filename)
{
#ifndef NOT_USE_GFXB
	m_ogg_decoder = new GLB_ogg_decoder( filename);
#endif
}


void GLB::Material::PlayVideo( float time_in_sec)
{
	m_video_time_in_sec = time_in_sec;

#ifndef NOT_USE_GFXB
	m_ogg_decoder->Play( m_video_time_in_sec);
#endif
}

void GLB::Material::DecodeVideo()
{
#ifndef NOT_USE_GFXB
	m_ogg_decoder->DecodePBO( m_video_time_in_sec);
#endif
}

}

KCL::Material *GLB::MaterialFactory::Create(const std::string& material_name, KCL::Node *parent, KCL::Object *owner)
{
#ifdef OPT_TEST_GFX40
	if (m_scene_version < KCL::SceneVersion::SV_40)
	{
		return new GLB::Material(material_name.c_str());
	}
	else
	{
		return new GLB::Material4(material_name.c_str());
	}
#else
	return new GLB::Material(material_name.c_str());
#endif
	
	return nullptr;
}
