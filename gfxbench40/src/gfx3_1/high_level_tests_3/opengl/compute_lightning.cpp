/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "compute_lightning.h"

#include <string>
#include <sstream>
#include <set>

#include "platform.h"
#include "kcl_io.h"
#include "glb_scene_.h"
#include "render_statistics_defines.h"
#include "opengl/glb_discard_functions.h"
#include "opengl/glb_opengl_state_manager.h"
#include "opengl/glb_texture.h"
#include "opengl/misc2_opengl.h"

using namespace GLB;

ComputeLightning::ComputeLightning() : m_data_prefix( "lightning_misc/")
{
	m_lightning_count = 0;

	m_vao = 0;
	m_render_buffer = 0;
	m_lightning_buffer = 0;	
	m_endpoint_buffer = 0;
		
	m_noise_texture = NULL;

	m_shader_pass1 = NULL;
	m_shader_pass2 = NULL;
	m_shader_render = NULL;
		
	m_lights_buffer = 0;
	
	m_ground_endoint_offset = 0;
	m_sky_endpoint_offset = 0;

	status = KCL::KCL_TESTERROR_UNKNOWNERROR;
}

ComputeLightning::~ComputeLightning()
{
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_render_buffer);
	glDeleteBuffers(1, &m_lightning_buffer);
	glDeleteBuffers(1, &m_endpoint_buffer);
	delete m_noise_texture;
}

void ComputeLightning::Init(KCL::uint32 lights_buffer, GLB_Scene_ES2_ *scene)
{
    m_scene = scene;

	CheckGLErrors("ComputeLightning: before init");

	status = KCL::KCL_TESTERROR_NOERROR;

	m_lightning_count = 0;

	m_lights_buffer = lights_buffer;

	// Load the endpoints
	m_ground_endoint_offset = LoadEndpoints("lightning_points_poles.txt");	
	m_sky_endpoint_offset = LoadEndpoints("lightning_points_new.txt");
	LoadEndpoints("lightning_points_up.txt");
  
	// Create GL objects		
	struct DrawArraysIndirectCmd
	{
		GLuint count;
		GLuint instance_count;
		GLuint first;
		GLuint reserved;
	};

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_render_buffer);	
	glGenBuffers(1, &m_lightning_buffer);
	glGenBuffers(1, &m_endpoint_buffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_render_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, RENDER_BUFFER_SIZE * sizeof(KCL::Vector4D) + sizeof(DrawArraysIndirectCmd), NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Create and zero out the lightning headers
	float headers[LIGHTNING_COUNT + 64]; // SSBO's can be larger than 9 floats so just mmake it bigger
	memset(headers, 0, sizeof(headers));
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightning_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(headers) + LIGHTNING_COUNT * LIGHTNING_BUFFER_SIZE * sizeof(KCL::Vector4D), NULL, GL_DYNAMIC_COPY);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(headers), headers);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_endpoint_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_endpoints.size() * sizeof(KCL::Vector4D), &m_endpoints[0], GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Setup the render VAO
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_render_buffer);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(KCL::Vector4D) * 2, (GLvoid*)sizeof(DrawArraysIndirectCmd)); // Position
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(KCL::Vector4D) * 2, (GLvoid*)(sizeof(KCL::Vector4D) + sizeof(DrawArraysIndirectCmd))); // Texture coordinates + intensity
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// Load the shaders
	m_shader_pass1 = LoadShader("lightning_pass1.shader");
	m_shader_pass2 = LoadShader("lightning_pass2.shader");
	m_shader_render = LoadShader("lightning_render.shader");
		
	// Load the noise texture
	GLB::GLBTextureFactory f;
	KCL::Texture *noise_texture = f.CreateAndSetup(KCL::Texture_2D, std::string( m_data_prefix + "lightning_data2.png").c_str() , KCL::TC_Commit | KCL::TC_NearestFilter | KCL::TC_NoMipmap);
	m_noise_texture = dynamic_cast<GLB::GLBTexture*>(noise_texture);
	if (!m_noise_texture)
	{
		INFO("ComputeLightning - ERROR: lightning_data.png not found!\n");
	}

	CheckGLErrors("ComputeLightning: after init");
}

KCL::uint32 ComputeLightning::LoadEndpoints(const char *filename)
{
	std::string fn = m_data_prefix + filename;

	KCL::AssetFile pos_endpoints_file(fn.c_str());	
	if(pos_endpoints_file.GetLastError())
	{
		INFO("ComputeLightning - ERROR: %s not found!\n", fn.c_str());
		return m_endpoints.size();
	}

	std::stringstream ss(pos_endpoints_file.GetBuffer());		
	while(true)
	{
		KCL::Vector4D pos;
		if (!(ss >> pos.x))
		{
			break;
		}
		ss >> pos.y;
		ss >> pos.z;

		// Filter out the lower points
		if (pos.y <= 0.5f)
		{
			continue;
		}

		m_endpoints.push_back(pos);
	}
	return m_endpoints.size();
}

GLBShader2 *ComputeLightning::LoadShader(const char *filename)
{
	GLBShaderBuilder sb;
	KCL::KCL_Status error;

	sb.AddDefineInt("LIGHTNING_COUNT", LIGHTNING_COUNT);
	sb.AddDefineInt("BUFFER_SIZE", LIGHTNING_BUFFER_SIZE);
	sb.AddDefineInt("ENDPOINT_COUNT", m_endpoints.size());
	sb.AddDefineInt("GROUND_ENDPOINT_OFFSET", m_ground_endoint_offset);
	sb.AddDefineInt("SKY_ENDPOINT_OFFSET", m_sky_endpoint_offset);
	sb.AddDefineInt("MAX_LIGHTNING_BUFFER", (LIGHTNING_BUFFER_SIZE * LIGHTNING_COUNT));
	sb.AddDefineInt("MAX_RENDER_BUFFER", RENDER_BUFFER_SIZE);
			
	GLBShader2 *shader = sb.ShaderFile(filename).Build(error);
	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		status = error;
	}
	return shader;
}

void ComputeLightning::Run(float animation_time, KCL::Actor *focus)
{			
	if (!IsInited())
	{
		return;
	}

	// Update the lightning count
	if (animation_time < 2000)
		m_lightning_count = 1;

	else if (animation_time < 3000)
		m_lightning_count = 5;

	else if (animation_time < 9000)
		m_lightning_count = 7;
	else
		m_lightning_count = LIGHTNING_COUNT;

	m_lightning_count = m_lightning_count > LIGHTNING_COUNT ? LIGHTNING_COUNT : m_lightning_count;
	
	// Get the model matrix of the actor
	KCL::Matrix4x4 model_matrix = focus->m_root->m_world_pom;

	// Calculate the right vector of the actor
	KCL::Vector3D actor_pos(&model_matrix.v[12]);
	KCL::Vector4D actor_right_vector_h = model_matrix * KCL::Vector4D(0.0f, 0.0f, 42.0f, 1.0); // Z points to the right in the model coordinate system
	KCL::Vector3D actor_right_vector(actor_right_vector_h.x / actor_right_vector_h.w, actor_right_vector_h.y / actor_right_vector_h.w, actor_right_vector_h.z / actor_right_vector_h.w);
	actor_right_vector = actor_right_vector - actor_pos;

	// Get the bones of the actor
	m_bone_segments.clear();

	GetBonesForActor(focus->m_root);
	
	// Dispatch the first pass. Calculate the lightnings as line segments
	OpenGLStateManager::GlUseProgram(m_shader_pass1->m_p);

	// Set the uniforms	
	glUniform1f(0, animation_time);	

	glUniform3f(1, actor_pos.x, actor_pos.y, actor_pos.z);
	glUniform3f(2, actor_right_vector.x, actor_right_vector.y, actor_right_vector.z);

	glUniform1ui(3, m_bone_segments.size() / 2);	
	glUniform4fv(4, m_bone_segments.size(), m_bone_segments[0].v);

	// Bind the storage buffers
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightning_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_endpoint_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_lights_buffer);     

	// Bind the noise texture
	glBindImageTextureProc(0, m_noise_texture->textureObject(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);

#ifdef TEXTURE_COUNTING
    m_scene->m_textureCounter.insert( ((GLBTexture*)m_noise_texture)->textureObject());
#endif

	glDispatchComputeProc(m_lightning_count, 1, 1);
	
    // Clean up 
	OpenGLStateManager::GlUseProgram(0);
}

void ComputeLightning::Draw(KCL::Camera2 *camera)
{
	if (!IsInited())
	{
		return;
	}

	KCL::Matrix4x4 mvp = camera->GetViewProjection();
		
	// Dispatch the second pass. Create quads
	OpenGLStateManager::GlUseProgram(m_shader_pass2->m_p);
	glUniform1i(1, m_lightning_count);
	glUniformMatrix4fv(2, 1, 0, mvp.v);	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightning_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_render_buffer);

	// Wait for result of the first pass 
	glMemoryBarrierProc(GL_SHADER_STORAGE_BARRIER_BIT);	

	glDispatchComputeProc(m_lightning_count, 1, 1);
	
	// Render the lightning with indirect glDrawArraysIndirect
	OpenGLStateManager::GlUseProgram(m_shader_render->m_p);
	glBindVertexArray(m_vao);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_render_buffer);

	// Wait for result of the second pass 
	glMemoryBarrierProc(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);		

#if STATISTICS_LOGGING_ENABLED
    typedef  struct 
    {
        unsigned int  count;
        unsigned int  primCount;
        unsigned int  first;
        unsigned int  reserved;
    } DrawArraysIndirectCommand;

    //get the draw call verts and tris
    DrawArraysIndirectCommand* ptr = (DrawArraysIndirectCommand*)glMapBuffer(GL_DRAW_INDIRECT_BUFFER, GL_READ_ONLY);
        
    m_scene->m_num_triangles += ptr->count * ptr->primCount;
    m_scene->m_num_vertices += ptr->count * ptr->primCount * 3;

    glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
#endif

	OpenGLStateManager::GlDepthMask(GL_FALSE);
	OpenGLStateManager::Commit();
	glDrawArraysIndirectProc(GL_TRIANGLES, 0);	
		
	OpenGLStateManager::GlDepthMask(GL_TRUE);
	OpenGLStateManager::GlUseProgram(0);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	glBindVertexArray(0);
}


KCL::uint32 ComputeLightning::GetLightCount()
{
	// Every lightning creates two lights
	return m_lightning_count * 2;	
}

void ComputeLightning::GetBonesForActor(KCL::Node *node)
{
	if (node->m_name.find("jnt") != node->m_name.size() - 3)
	{
		// Bones with _jnt are not part of the robot
		return;
	}

	// Skip the fingers
	if (node->m_name.find("L_1") != std::string::npos)
	{
		return;
	}

	if (node->m_name.find("L_2") != std::string::npos)
	{
		return;
	}

	// Skip the toes and feet
	if (node->m_name.find("toe") != std::string::npos)
	{
		return;
	}

	if (node->m_name.find("foot") != std::string::npos)
	{
		return;
	}	

	if (node->m_name.find("leg") != std::string::npos)
	{
		return;
	}	

	if (node->m_name.find("hip") != std::string::npos)
	{
		return;
	}	

	
	if (node->m_parent) 
	{
		KCL::Vector4D pos1;
		KCL::Vector4D pos2;
		if (node->m_parent->m_name.find("chest") != std::string::npos)
		{
			pos2 = KCL::Vector4D(&node->m_parent->m_world_pom.v[12]);
			pos1 = KCL::Vector4D(&node->m_world_pom.v[12]);
		}
		else
		{
			pos1 = KCL::Vector4D(&node->m_parent->m_world_pom.v[12]);
			pos2 = KCL::Vector4D(&node->m_world_pom.v[12]);
		}

		pos1.w = 1.0f;
		pos2.w = 1.0f;
		
		// Put the gun to the list more time lower_arm_R
		if (node->m_name.find("gun") != std::string::npos)
		{				
			m_bone_segments.push_back(pos2);
			m_bone_segments.push_back(pos1);

			m_bone_segments.push_back(pos1);
			m_bone_segments.push_back(pos2);

			m_bone_segments.push_back(pos2);
			m_bone_segments.push_back(pos1);
		}

		m_bone_segments.push_back(pos1);
		m_bone_segments.push_back(pos2);	
	}

	for( KCL::uint32 i=0; i<node->m_children.size(); i++)
	{
		GetBonesForActor(node->m_children[i]);
	}
}
