/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "vdb_material.h"
#include "common/opengl/shader.h"

VDB::Material::Material( const char *name) : 
	KRL::Material(name),
	m_ubo_id (-1),
	m_ubo_offset (-1)
{	
}

VDB::Material::~Material() 
{
}

KCL::Material* KCL::Material::Create( const char *name, const int scene_version)
{
	return new VDB::Material(name);
}

Shader* VDB::Material::CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error)
{
	return Shader::CreateShader(vsfile, fsfile, defines, error);
}
