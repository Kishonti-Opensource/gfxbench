/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "opengl/ubo_manager.h"

#include "platform.h"
#include "opengl/shader.h"
#include "kcl_os.h"
#include "kcl_math3d.h"
#include "glb_scene_opengl31.h"

using namespace GLB;


//TODO LightBufferObject 3.0
void UBOManager::SetLBO(LightBufferObject * light, const UBOMesh * data, const UBOStaticMesh * static_data)
{
	if (light->m_ubo_handle == -1)
	{
		light->m_ubo_handle = CreateHandle();
	}
	
	BufferBind & bind = m_ubo_handles[m_current_camera][light->m_ubo_handle];	
	bind.m_offset = StoreData((void*)data, sizeof(UBOMesh), m_mesh_asize);
	bind.m_id =  m_ubos[m_ubo_index].m_id;
	bind.m_static_offset = StoreData((void*)static_data, sizeof(UBOStaticMesh), m_static_mesh_asize);
	bind.m_static_id =  m_ubos[m_ubo_index].m_id;
}


void UBOManager::SetFilter(Filter31 * filter, const UBOFilter * data)
{
	if (filter->m_ubo_handle == -1)
	{
		filter->m_ubo_handle = CreateHandle();
	}
	
	BufferBind & bind = m_ubo_handles[0][filter->m_ubo_handle];	
	bind.m_offset = StoreData((void*)data, sizeof(UBOFilter), m_filter_asize);
	bind.m_id =  m_ubos[m_ubo_index].m_id;
}


void UBOManager::BindLBO(LightBufferObject * light)
{
	BufferBind & bind = m_ubo_handles[m_current_camera][light->m_ubo_handle];
	glBindBufferRange(GL_UNIFORM_BUFFER,
		Shader::sUBOnames::Mesh,
			bind.m_id,
			bind.m_offset,
			sizeof(UBOMesh));	
	glBindBufferRange(GL_UNIFORM_BUFFER,
			Shader::sUBOnames::StaticMesh,
			bind.m_static_id,
			bind.m_static_offset,
			sizeof(UBOStaticMesh));	
}


void UBOManager::BindFilter(Filter31 * filter)
{
	BufferBind & bind = m_ubo_handles[0][filter->m_ubo_handle];
	glBindBufferRange(GL_UNIFORM_BUFFER,
		Shader::sUBOnames::FilterConsts,
			bind.m_id,
			bind.m_offset,
			sizeof(UBOFilter));	
}
