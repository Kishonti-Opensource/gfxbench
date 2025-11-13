/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ubo_manager.h"

#include "platform.h"
#include "opengl/shader.h"
#include "kcl_os.h"
#include "kcl_math3d.h"

using namespace GLB;

#define DEFAULT_HANDLE_COUNT 550
#define CACHE_STATIC_MESHES

#define PREALLOCATED_MEMORY 384	// in kbytes

bool UBOManager::log_enabled = false;

UBOManager::UBOManager()
{
	m_ubo_usage = GL_STREAM_DRAW;

	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &m_ubo_max_size);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &m_offset_alignment);	

	m_ubo_max_size = KCL::Min(65536, m_ubo_max_size);

	m_frame_asize =  AlignedSize(sizeof(UBOFrame), m_offset_alignment);
	m_camera_asize = AlignedSize(sizeof(UBOCamera), m_offset_alignment);
	m_mat_asize = AlignedSize(sizeof(UBOMaterial), m_offset_alignment);
	m_mesh_asize = AlignedSize(sizeof(UBOMesh), m_offset_alignment);
	m_static_mesh_asize = AlignedSize(sizeof(UBOStaticMesh), m_offset_alignment);
	m_emitter_advect_asize = AlignedSize(sizeof(UBOEmitterAdvect), m_offset_alignment);
	m_emitter_render_asize = AlignedSize(sizeof(UBOEmitterRender), m_offset_alignment);	
	m_light_shaft_asize = AlignedSize(sizeof(UBOLightShaft), m_offset_alignment);
	m_light_lens_flare_asize = AlignedSize(sizeof(UBOLightLensFlare), m_offset_alignment);
	m_filter_asize = AlignedSize(sizeof(UBOFilter), m_offset_alignment);
	m_translate_uv_asize = AlignedSize(sizeof(UBOTranslateUV), m_offset_alignment);
	m_envmaps_interpolator_asize = AlignedSize(sizeof(UBOEnvmapsInterpolator), m_offset_alignment);

	if (log_enabled)
	{
		int max_binds;
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &max_binds);
		INFO("UBOManager:");
		INFO("GL_MAX_UNIFORM_BUFFER_BINDINGS: %d", max_binds);
		INFO("GL_MAX_UNIFORM_BLOCK_SIZE: %d", m_ubo_max_size);
		INFO("GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT: %d", m_offset_alignment);

		INFO("m_frame_asize: %d -> %d ", sizeof(UBOFrame), m_frame_asize);
		INFO("m_camera_asize: %d -> %d ", sizeof(UBOCamera), m_camera_asize);
		INFO("m_mat_asize: %d -> %d", sizeof(UBOMaterial), m_mat_asize);
		INFO("m_mesh_asize: %d -> %d", sizeof(UBOMesh), m_mesh_asize);
		INFO("m_static_mesh_asize: %d -> %d", sizeof(UBOStaticMesh), m_static_mesh_asize);
		INFO("m_emitter_advect_asize: %d -> %d", sizeof(UBOEmitterAdvect), m_emitter_advect_asize);
		INFO("m_emitter_render_asize: %d -> %d", sizeof(UBOEmitterRender), m_emitter_render_asize);
		INFO("m_light_shaft_asize: %d -> %d", sizeof(UBOLightShaft), m_light_shaft_asize);
		INFO("m_light_lens_flare_asize: %d -> %d", sizeof(UBOLightLensFlare), m_light_lens_flare_asize);
		INFO("m_filter_asize: %d -> %d", sizeof(UBOFilter), m_filter_asize);
		INFO("m_translate_uv_asize: %d -> %d", sizeof(UBOTranslateUV), m_translate_uv_asize);
		INFO("m_envmaps_interpolator_asize: %d -> %d", sizeof(UBOEnvmapsInterpolator), m_envmaps_interpolator_asize);	
	}
	
	m_ubo_index = 0;	 
	m_offset = 0;

	m_ubo_handles[0].resize(DEFAULT_HANDLE_COUNT);
	m_ubo_handles[1].resize(DEFAULT_HANDLE_COUNT);
	m_ubo_handle_counter = 0;
	m_current_camera = 0;

	m_prev_camera_bind = -1;
	m_prev_mesh_bind = NULL;
	m_prev_material_bind = NULL;
}

UBOManager::~UBOManager()
{
	if (log_enabled)
	{
		INFO("UBO handles: %d", m_ubo_handle_counter);
		INFO("Constant UBOs: %d  Dynamic UBOs: %d", m_constant_ubos.size(), m_ubos.size());
	}

	if (!m_constant_ubos.empty())
	{
		glDeleteBuffers(m_constant_ubos.size(), &m_constant_ubos[0]);
	}

	for (unsigned int i = 0; i < m_ubos.size(); i++)
	{
		glDeleteBuffers(1, &m_ubos[i].m_id);
		delete [] m_ubos[i].mem_ptr;
	}	
}

void UBOManager::Precache(KRL_Scene * scene_handler)
{			
	std::vector<KCL::Material*> & materials  = scene_handler->m_materials;
	GLuint id;
	glGenBuffers(1, &id);
	glBindBuffer(GL_UNIFORM_BUFFER, id);
	glBufferData(GL_UNIFORM_BUFFER, m_ubo_max_size, NULL, GL_STATIC_DRAW);
	m_constant_ubos.push_back(id);
	unsigned int offset = 0;

	char * map_ptr = (char*) glMapBufferRange(GL_UNIFORM_BUFFER, 0, m_ubo_max_size, GL_MAP_WRITE_BIT);

	UBOMaterial mat_data;
	for (unsigned int i = 0; i < materials.size(); i++)
	{
		GLB::Material *material = dynamic_cast<GLB::Material*>(materials[i]);
		if (!material)
		{
			continue;
		}		
		if (m_ubo_max_size - offset < m_mat_asize)
		{
			glUnmapBuffer(GL_UNIFORM_BUFFER);

			glGenBuffers(1, &id);
			glBindBuffer(GL_UNIFORM_BUFFER, id);
			glBufferData(GL_UNIFORM_BUFFER, m_ubo_max_size, NULL, GL_STATIC_DRAW);

			m_constant_ubos.push_back(id);
			map_ptr = (char*) glMapBufferRange(GL_UNIFORM_BUFFER, 0, m_ubo_max_size, GL_MAP_WRITE_BIT);
			offset = 0;
		}	

		mat_data.matparams_disiseri = KCL::Vector4D(material->m_diffuse_intensity, material->m_specular_intensity, material->m_specular_exponent, material->m_reflect_intensity);
		mat_data.fresnelXYZ_transp = KCL::Vector4D(material->m_fresnel_params.x, material->m_fresnel_params.y, material->m_fresnel_params.z, material->m_transparency);		
		memcpy(map_ptr + offset, &mat_data, sizeof(UBOMesh));

		material->m_ubo_id = id;
		material->m_ubo_offset = offset;

		offset += m_mat_asize;
	}

#ifdef CACHE_STATIC_MESHES

	UBOStaticMesh static_mesh;
	for (unsigned int i = 0; i < scene_handler->m_rooms.size(); i++)
	{
		for (unsigned int j = 0; j < scene_handler->m_rooms[i]->m_meshes.size(); j++)
		{
			KCL::Mesh * mesh = scene_handler->m_rooms[i]->m_meshes[j];			
			if (m_ubo_max_size - offset < m_static_mesh_asize)
			{
				glUnmapBuffer(GL_UNIFORM_BUFFER);

				glGenBuffers(1, &id);
				glBindBuffer(GL_UNIFORM_BUFFER, id);
				glBufferData(GL_UNIFORM_BUFFER, m_ubo_max_size, NULL, GL_STATIC_DRAW);

				m_constant_ubos.push_back(id);
				map_ptr = (char*) glMapBufferRange(GL_UNIFORM_BUFFER, 0, m_ubo_max_size, GL_MAP_WRITE_BIT);
				offset = 0;
			}	

			static_mesh.model = mesh->m_world_pom;
			static_mesh.inv_model = KCL::Matrix4x4::Invert4x3(mesh->m_world_pom);
			static_mesh.color_pad = KCL::Vector4D(mesh->m_color);
			memcpy(map_ptr + offset, &static_mesh, sizeof(UBOStaticMesh));
		
			mesh->m_ubo_handle = CreateHandle();
			BufferBind & bind = m_ubo_handles[0][mesh->m_ubo_handle];
			bind.m_static_id = id;
			bind.m_static_offset = offset;
			bind.precached = true;

			bind = m_ubo_handles[1][mesh->m_ubo_handle];
			bind.m_static_id = id;
			bind.m_static_offset = offset;
			bind.precached = true;

			offset += m_static_mesh_asize;
		}	
	}
#endif

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
			
	// Preallocate memory. Round up the min buffer count
	unsigned int ubo_count = (PREALLOCATED_MEMORY * 1024 - 1) / m_ubo_max_size + 1;
	for (unsigned int i = 0; i < ubo_count; i++)
	{
		CreateNewBuffer();
	}

	// Bind the frame
	glBindBufferRange(GL_UNIFORM_BUFFER, Shader::sUBOnames::Frame,  m_ubos[0].m_id, 0, m_frame_asize);
}

void UBOManager::BeginFrame()
{
	m_ubo_index = 0;
	m_offset = Camera::CAMERA_COUNT * m_camera_asize + m_frame_asize;

	m_prev_mesh_bind = NULL;
}
void UBOManager::EndFrame()
{
}

void UBOManager::SetFrame(const UBOFrame * data)
{
	Buffer & buffer = m_ubos[0];
	memcpy(buffer.mem_ptr, data, sizeof(UBOFrame));
}

void UBOManager::SetCamera(KCL::uint32 camera_index, const UBOCamera * data)
{
	m_current_camera = camera_index;

	Buffer & buffer = m_ubos[0];
	memcpy(buffer.mem_ptr + camera_index * m_camera_asize + m_frame_asize, data, sizeof(UBOCamera));	
}

void UBOManager::SetMesh(KCL::Mesh * mesh, const UBOMesh * data, const UBOStaticMesh * static_data)
{
	if (mesh->m_ubo_handle == -1)
	{
		mesh->m_ubo_handle = CreateHandle();		
	}

	BufferBind & bind = m_ubo_handles[m_current_camera][mesh->m_ubo_handle];	
	bind.m_offset = StoreData((void*)data, sizeof(UBOMesh), m_mesh_asize);
	bind.m_id =  m_ubos[m_ubo_index].m_id;

	if (!bind.precached)
	{
		bind.m_static_offset = StoreData((void*)static_data, sizeof(UBOStaticMesh), m_static_mesh_asize);
		bind.m_static_id =  m_ubos[m_ubo_index].m_id;
	}
}

void UBOManager::SetTranslateUV(KCL::Mesh * mesh, const UBOTranslateUV * data)
{
	if (mesh->m_ubo_handle == -1)
	{
		mesh->m_ubo_handle = CreateHandle();
	}
	
	BufferBind & bind = m_ubo_handles[0][mesh->m_ubo_handle];	
	bind.m_offset2 = StoreData((void*)data, sizeof(UBOTranslateUV), m_translate_uv_asize);
	bind.m_id2 =  m_ubos[m_ubo_index].m_id;
}
void UBOManager::SetEnvmapsInterpolator(KCL::Mesh * mesh, const UBOEnvmapsInterpolator * data)
{
	if (mesh->m_ubo_handle == -1)
	{
		mesh->m_ubo_handle = CreateHandle();
	}
	
	BufferBind & bind = m_ubo_handles[0][mesh->m_ubo_handle];	
	bind.m_offset3 = StoreData((void*)data, sizeof(UBOEnvmapsInterpolator), m_envmaps_interpolator_asize);
	bind.m_id3 =  m_ubos[m_ubo_index].m_id;
}

	
void UBOManager::SetEmitter(GLB::TF_emitter * emitter, const UBOEmitterAdvect * data, const UBOEmitterRender * data2)
{
	if (emitter->m_ubo_handle == -1)
	{
		emitter->m_ubo_handle = CreateHandle();
	}
	
	BufferBind & bind = m_ubo_handles[0][emitter->m_ubo_handle];	
	bind.m_offset = StoreData((void*)data, sizeof(UBOEmitterAdvect), m_emitter_advect_asize);
	bind.m_id =  m_ubos[m_ubo_index].m_id;
	
	bind.m_offset2 = StoreData((void*)data2, sizeof(UBOEmitterRender), m_emitter_render_asize);
	bind.m_id2 =  m_ubos[m_ubo_index].m_id;
}

void UBOManager::SetLightShaft(GLB::Light * light, const UBOLightShaft * data)
{
	if (light->m_ubo_handle == -1)
	{
		light->m_ubo_handle = CreateHandle();
	}
	
	BufferBind & bind = m_ubo_handles[0][light->m_ubo_handle];	
	bind.m_offset = StoreData((void*)data, sizeof(UBOLightShaft), m_light_shaft_asize);
	bind.m_id =  m_ubos[m_ubo_index].m_id;
}

void UBOManager::SetLightLensFlare(GLB::Light * light, const UBOLightLensFlare * data)
{
	if (light->m_ubo_handle == -1)
	{
		light->m_ubo_handle = CreateHandle();
	}
	
	BufferBind & bind = m_ubo_handles[0][light->m_ubo_handle];	
	bind.m_offset2 = StoreData((void*)data, sizeof(UBOLightLensFlare), m_light_lens_flare_asize);
	bind.m_id2 =  m_ubos[m_ubo_index].m_id;
}

void UBOManager::Upload()
{	
	for (unsigned int i = 0; i < m_ubo_index + 1; i++)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_ubos[i].m_id); 

		unsigned int size = i < m_ubo_index ? m_ubo_max_size : m_offset;
		
	//	Update with SubData
		glBufferSubData(GL_UNIFORM_BUFFER, 0, size, m_ubos[i].mem_ptr);

	//  Update with GL_MAP_INVALIDATE_BUFFER_BIT
	//	void * map_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	//	if (map_ptr)
	//	{
	//		memcpy(map_ptr, m_ubos[i].mem_ptr, size);
	//		glUnmapBuffer(GL_UNIFORM_BUFFER);
	//	}
	//	else
	//	{
	//		INFO("UBOManager::Upload() - map_ptr is null!");
	//	}


	//  Orphaning with SubData
	//	glBufferData(GL_UNIFORM_BUFFER, m_ubo_max_size, NULL, m_ubo_usage);
	//	glBufferSubData(GL_UNIFORM_BUFFER, 0, size, m_ubos[i].mem_ptr);


	//  Orphaning with Map
	//	glBufferData(GL_UNIFORM_BUFFER, m_ubo_max_size, NULL, m_ubo_usage);
	//	void * map_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, size, GL_MAP_WRITE_BIT);
	//	if (map_ptr)
	//	{
	//		memcpy(map_ptr, m_ubos[i].mem_ptr, size);
	//		glUnmapBuffer(GL_UNIFORM_BUFFER);
	//	}
	//	else
	//	{
	//		INFO("UBOManager::Upload() - map_ptr is null!");
	//	}
	}

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBOManager::BindCamera(KCL::uint32 camera_index)
{
	m_current_camera = camera_index;
	if (m_prev_camera_bind != camera_index)
	{
		m_prev_camera_bind = camera_index;
		glBindBufferRange(GL_UNIFORM_BUFFER, Shader::sUBOnames::Camera, m_ubos[0].m_id, camera_index * m_camera_asize + m_frame_asize, sizeof(UBOCamera));		
	}		
}

void UBOManager::BindMesh(KCL::Mesh * mesh, GLB::Material * material)
{	
	if (m_prev_material_bind != material)
	{
		glBindBufferRange(GL_UNIFORM_BUFFER,
			Shader::sUBOnames::Mat,
			material->m_ubo_id,
			material->m_ubo_offset,
			sizeof(UBOMaterial));	
		m_prev_material_bind = material;
	}

	if (m_prev_mesh_bind != mesh)
	{
		BufferBind & bind = m_ubo_handles[m_current_camera][mesh->m_ubo_handle];
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

		m_prev_mesh_bind = mesh;
	}
}


void UBOManager::BindEmitterAdvect(GLB::TF_emitter * emitter)
{
	BufferBind & bind = m_ubo_handles[0][emitter->m_ubo_handle];
	glBindBufferRange(GL_UNIFORM_BUFFER,
		Shader::sUBOnames::EmitterAdvect,
			bind.m_id,
			bind.m_offset,
			sizeof(UBOEmitterAdvect));	
}

void UBOManager::BindEmitterRender(GLB::TF_emitter * emitter)
{
	BufferBind & bind = m_ubo_handles[0][emitter->m_ubo_handle];
	glBindBufferRange(GL_UNIFORM_BUFFER,
		Shader::sUBOnames::EmitterRender,
			bind.m_id2,
			bind.m_offset2,
			sizeof(UBOEmitterRender));	
}

void UBOManager::BindLightShaft(GLB::Light * light)
{
	BufferBind & bind = m_ubo_handles[0][light->m_ubo_handle];
	glBindBufferRange(GL_UNIFORM_BUFFER,
		Shader::sUBOnames::LightShaftConsts,
			bind.m_id,
			bind.m_offset,
			sizeof(UBOLightShaft));	
}

void UBOManager::BindLightLensFlare(GLB::Light * light)
{
	BufferBind & bind = m_ubo_handles[0][light->m_ubo_handle];
	glBindBufferRange(GL_UNIFORM_BUFFER,
		Shader::sUBOnames::LightLensFlareConsts,
			bind.m_id2,
			bind.m_offset2,
			sizeof(UBOLightLensFlare));	
}


void UBOManager::BindTranslateUV(KCL::Mesh * mesh)
{
	BufferBind & bind = m_ubo_handles[0][mesh->m_ubo_handle];
	glBindBufferRange(GL_UNIFORM_BUFFER,
		Shader::sUBOnames::TranslateUVConsts,
			bind.m_id2,
			bind.m_offset2,
			sizeof(UBOTranslateUV));	
}

void UBOManager::BindEnvmapsInterpolator(KCL::Mesh * mesh)
{
	BufferBind & bind = m_ubo_handles[0][mesh->m_ubo_handle];
	glBindBufferRange(GL_UNIFORM_BUFFER,
		Shader::sUBOnames::EnvmapsInterpolatorConsts,
			bind.m_id3,
			bind.m_offset3,
			sizeof(UBOEnvmapsInterpolator));	
}

void UBOManager::BindUBO(KCL::int32 ubo_handle, KCL::int32 slot)
{
    assert(slot > -1); //check correct values

	BufferBind& bind = m_ubo_handles[m_current_camera][ubo_handle];
	glBindBufferRange(GL_UNIFORM_BUFFER,
		    slot,
			bind.m_id,
			bind.m_offset,
			bind.m_structSize);	
}

KCL::uint32 UBOManager::StoreData(const void * data, int size, int asize)
{
	if (m_ubo_max_size - m_offset < asize)
	{
		if (m_ubo_index + 1 < m_ubos.size())
		{
			m_ubo_index++;
		}
		else
		{
			m_ubo_index = CreateNewBuffer();
		}
		m_offset = 0;
	}
    else if(!m_ubos.size())
    {
        m_ubo_index = CreateNewBuffer();
        m_offset = 0;
    }
	memcpy(m_ubos[m_ubo_index].mem_ptr + m_offset , data, size);
	unsigned int old_offset = m_offset;
	m_offset += asize;
	
	return old_offset;
}

KCL::uint32 UBOManager::CreateNewBuffer()
{	
	Buffer buffer;

	glGenBuffers(1, &buffer.m_id);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer.m_id);
	glBufferData(GL_UNIFORM_BUFFER, m_ubo_max_size, NULL, m_ubo_usage);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	buffer.mem_ptr = new char[m_ubo_max_size];
	
	m_ubos.push_back(buffer);

	if (log_enabled)
	{
		INFO("UBOManager - New dynamic UBO created: %d %d", buffer.m_id, m_ubos.size());
	}

	return m_ubos.size() - 1;
}

KCL::uint32 UBOManager::AlignedSize(KCL::uint32 size, KCL::uint32 alignment)
{
	KCL::uint32 mod = size % alignment;
	if (mod == 0)
	{
		return size;
	}
	else
	{
		return size + alignment - mod;
	}
}

KCL::int32 UBOManager::CreateHandle()
{
	KCL::int32 handle = m_ubo_handle_counter;
	m_ubo_handle_counter++;

	if (m_ubo_handle_counter >= m_ubo_handles[0].size()) 
	{
		if (log_enabled)
		{
			INFO("UBOManager - Resize UBOs: %d %d\n", m_ubo_handle_counter, m_ubo_handle_counter * 2);
		}
		m_ubo_handles[0].resize(m_ubo_handle_counter * 2);
		m_ubo_handles[1].resize(m_ubo_handle_counter * 2);
	}
	return handle;
}
