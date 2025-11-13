/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_material.h"
#include "gfxb_shader.h"
#include "gfxb_texture.h"
#include <kcl_mesh.h>

#include <ngl.h>

using namespace GFXB;

Material::Material(const char *name) : KCL::Material(name)
{
	m_shader_programs = nullptr;
	SetDefaults();
}


Material::~Material()
{
}


void Material::SetDefaults()
{
	KCL::Material::SetDefaults();

	m_shader_names[0] = "main";

	for (KCL::uint32 i = 0; i < SHADER_VARIANT_COUNT; i++)
	{
		m_shader_codes[i] = 0;
	}
}


KCL::KCL_Status Material::Init()
{
	const char *desc_name = m_shader_names[0].empty() ? "main" : m_shader_names[0].c_str();

	const ShaderDescriptor *desc_ptr = ShaderFactory::GetInstance()->GetDescriptor(desc_name);
	if (desc_ptr == nullptr)
	{
		INFO("Material::Init - Can not find shader descriptor: %s for material: %s", desc_name, m_name.c_str());
		return KCL::KCL_TESTERROR_UNKNOWNERROR;
	}

	ShaderDescriptor sd = *desc_ptr;
	sd.SetName(nullptr);

	// Add material defines
	if (m_opacity_mode == KCL::Material::ALPHA_TEST)
	{
		sd.AddDefine("ALPHA_TEST");
	}

	if (m_is_two_sided)
	{
		sd.AddDefine("IS_TWO_SIDED");
	}

	/*
	if (m_is_dithered)
	{
		sd.AddDefine("DITHERED");
	}
	*/

	if (m_is_texture_density)
	{
		sd.AddDefine("TEXTURE_DENSITY");
	}

	if (m_is_debug_mipmap_level)
	{
		sd.AddDefine("DEBUG_MIPMAP_LEVEL");
	}

	// Normal mesh
	m_shader_codes[SHADER_NORMAL] = ShaderFactory::GetInstance()->AddDescriptor(sd);

	// Skeletal mesh
	ShaderDescriptor skeletal_sd = sd;
	skeletal_sd.AddDefineInt("SKELETAL", 1);
	skeletal_sd.AddDefineInt("MAX_BONES", 3 * KCL::Mesh3::MAX_BONES); // One bone needs 3 vec4
	m_shader_codes[SHADER_SKELETAL] = ShaderFactory::GetInstance()->AddDescriptor(skeletal_sd);

	// Instanced mesh
	ShaderDescriptor instanced_sd = sd;
	instanced_sd.AddDefine("INSTANCED");
	m_shader_codes[SHADER_INSTANCED] = ShaderFactory::GetInstance()->AddDescriptor(instanced_sd);

	return KCL::KCL_TESTERROR_NOERROR;
}


Texture* Material::GetTexture(KCL::uint32 idx) const
{
	return (Texture*)m_textures[idx];
}


void Material::SetShaderCode(Material::ShaderVariant variant, KCL::uint32 shader_code)
{
	m_shader_codes[variant] = shader_code;
}


KCL::uint32 Material::GetShaderCode(Material::ShaderVariant variant) const
{
	return m_shader_codes[variant];
}
