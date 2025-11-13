/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dxb_light.h"

DXB::Light::Light ( const std::string& light_name, KCL::Node *parent, KCL::Object *owner) : KCL::Light(light_name, parent, owner) 
{
    memset( m_query_objects, 0, QUERY_COUNT * sizeof(ID3D11Query*));
    memset( m_query_initialized, 0, QUERY_COUNT * sizeof(bool));
    m_current_query_index = 0;
}

ID3D11Query* DXB::Light::GetCurrentQueryObject()
{
	m_query_initialized[m_current_query_index] = true;

	return m_query_objects[m_current_query_index];
}

ID3D11Query* DXB::Light::GetPreviousQueryObject()
{
	KCL::uint32 idx = GetPreviousQueryIndex();

	return m_query_objects[idx];
}

void DXB::Light::SetQueryObjects(ID3D11Query* queries[])
{
    memcpy(m_query_objects, queries, QUERY_COUNT * sizeof(ID3D11Query*));
}

KCL::uint32 DXB::Light::GetPreviousQueryIndex()
{
	KCL::uint32 idx = m_current_query_index + QUERY_COUNT;
	idx -= (QUERY_COUNT-1);
	idx = idx % QUERY_COUNT;
	
	return idx;
}

void DXB::Light::NextQueryObject()
{
	m_current_query_index++;
	m_current_query_index = m_current_query_index % QUERY_COUNT;
}

bool DXB::Light::IsPreviousQueryObjectInitialized()
{
	KCL::uint32 idx = GetPreviousQueryIndex();

	return m_query_initialized[idx];
}

KCL::Light *DXB::DXBLightFactory::Create(const std::string& light_name, KCL::Node *parent, KCL::Object *owner)
{ 
	return new DXB::Light(light_name,parent,owner); 
}