/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_light.h"

GLB::Light::Light ( const std::string& light_name, KCL::Node *parent, KCL::Object *owner) : KCL::Light(light_name, parent, owner) 
{
    memset( m_query_objects, 0, QUERY_COUNT * sizeof(KCL::uint32));
    memset( m_query_initialized, 0, QUERY_COUNT * sizeof(bool));
    m_current_query_index = 0;

	m_ubo_handle = -1;
}

KCL::uint32 GLB::Light::GetCurrentQueryObject()
{
	m_query_initialized[m_current_query_index] = true;

	return m_query_objects[m_current_query_index];
}

KCL::uint32 GLB::Light::GetPreviousQueryObject()
{
	KCL::uint32 idx = GetPreviousQueryIndex();

	return m_query_objects[idx];
}

KCL::uint32 GLB::Light::GetPreviousQueryIndex()
{
	KCL::uint32 idx = m_current_query_index + QUERY_COUNT;
	idx -= (QUERY_COUNT-1);
	idx = idx % QUERY_COUNT;
	
	return idx;
}

void GLB::Light::NextQueryObject()
{
	m_current_query_index++;
	m_current_query_index = m_current_query_index % QUERY_COUNT;
}

bool GLB::Light::IsPreviousQueryObjectInitialized()
{
	KCL::uint32 idx = GetPreviousQueryIndex();

	return m_query_initialized[idx];
}


KCL::Light *GLB::GLBLightFactory::Create(const std::string& light_name, KCL::Node *parent, KCL::Object *owner)
{ 
	return new GLB::Light(light_name,parent,owner); 
}
