/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dxb_material.h"

#include "d3d11/dxb_image.h"

#include "d3d11/shader.h"

#define SAFE_RELEASE(x)	if ((x)) { (x)->Release(); (x) = NULL; }
#define EXIT_IF_FAILED(x) if (FAILED(x)) { return -1; }

DXB::Material::Material( const char *name) : 
	KRL::Material(name),
	m_planar_map(NULL),
	m_videoTexture(NULL),
	m_video(NULL)
{
	for( KCL::uint32 i = 0; i < 2; i++)
		for( KCL::uint32 j = 0; j < 2; j++)
		{
			m_shaders[i][j] = 0;
		}
}

DXB::Material::~Material()
{
	if (m_videoTexture)
	{
		delete m_videoTexture;
		m_videoTexture = NULL;
	}

	if (m_video)
	{
		delete m_video;
		m_video = NULL;
	}
}

void DXB::Material::PlayVideo( float time_in_sec)
{
	if (NULL != m_videoTexture)
	{
		m_videoTexture->setVideoTime(time_in_sec);
	}
}

KCL::Material *DXB::MaterialFactory::Create(const std::string& material_name, KCL::Node *parent, KCL::Object *owner)
{
	return new DXB::Material( material_name.c_str());
}


Shader* DXB::Material::CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error)
{
	return Shader::CreateShader(vsfile, fsfile, defines, error);
}