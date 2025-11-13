/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gl_alu2.h"

#include "kcl_io.h"
#include "kcl_image.h"
#include "kcl_math3d.h"

#include "platform.h"
#include "opengl/glb_opengl_state_manager.h"

#include <vector>
#include <sstream>
#include <limits.h> // UINT_MAX

using namespace GLB;

#define NUM_LIGHTS_PER_LOOP 16

//
// Created from Manhattan frame: 59500ms
//
ALUTest2::ALUTest2(const GlobalTestEnvironment* const gte) : ALUTest2_Base(gte)
{
	m_LightsVao = 0;
    m_EmissiveVao = 0;
	m_index_buffer = 0;
	m_vertex_buffer = 0;

	m_color_texture = 0;
	m_normal_texture = 0;
	m_depth_texture = 0;
	m_sampler = 0;

	m_shaderLights = 0;
    m_shaderAmbEmit = 0;

    m_uniform_light_pos_array = 0;
    m_uniform_light_color_array = 0;
}

ALUTest2::~ALUTest2()
{
	FreeResources();
}

KCL::KCL_Status ALUTest2::init()
{			
	KCL::KCL_Status init_status = KCL::KCL_TESTERROR_NOERROR;

	m_score = 0;
	
	// Create near sampler for G buffer sampling
	glGenSamplers(1, &m_sampler);
	glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
	
	// Load the textures
	m_color_texture = LoadTexture("alu2/alu2_albedo_emission.png");	
	if (!m_color_texture)
	{
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}	
	m_normal_texture = LoadTexture("alu2/alu2_normal_normalized.png");
	if (!m_normal_texture)
	{
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}		

	// Load the depth texture
	KCL::AssetFile depth_file("alu2/alu2_depth.raw");
	if(depth_file.GetLastError())
	{
		INFO("ALUTest2 - ERROR: alu2/alu2_depth.raw not found!\n");
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}

	const float * depth_data = (float*) depth_file.GetBuffer(); 
	std::vector<KCL::uint32> depth_uints;
	depth_uints.resize(G_BUFFER_WIDTH * G_BUFFER_HEIGHT);
	for (int i = 0; i < G_BUFFER_WIDTH * G_BUFFER_HEIGHT; i++)
	{		
		depth_uints[i] = KCL::uint32(UINT_MAX * double(depth_data[i]));
	}

	glGenTextures(1, &m_depth_texture);
	glBindTexture(GL_TEXTURE_2D, m_depth_texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, G_BUFFER_WIDTH, G_BUFFER_HEIGHT);
	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, G_BUFFER_WIDTH, G_BUFFER_HEIGHT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, depth_uints.data());		
	glBindTexture(GL_TEXTURE_2D, 0);

	// Load the scene
	init_status = LoadScene();
	if (init_status != KCL::KCL_TESTERROR_NOERROR)
	{
		return init_status;
	}

	// Load the shaders
	GLB::GLBShader2::InitShaders(KCL::SV_30, GetSetting().m_force_highp);
	GLB::GLBShaderBuilder sb;		
	m_shaderLights = sb
		.AddShaderDir("shaders_40/lowlevel2/")
        .AddDefineInt("DO_EMISSIVE_AND_AMBIENT", 0)
        .AddDefineInt("NUM_LIGHTS", NUM_LIGHTS_PER_LOOP)
		.AddDefineInt("NUM_COLORS", m_light_color_array.size())
		.ShaderFile("alu2.shader")
		.Build(init_status);
	if (init_status != KCL::KCL_TESTERROR_NOERROR)
	{
		INFO("ALUTest2 - ERROR: Can not load ALU Lights shader!\n");
		return init_status;
	}
    m_shaderAmbEmit = sb
        .AddShaderDir("shaders_40/lowlevel2/")
        .AddDefineInt("DO_EMISSIVE_AND_AMBIENT", 1)
        .ShaderFile("alu2.shader")
        .Build(init_status);
    if (init_status != KCL::KCL_TESTERROR_NOERROR)
    {
        INFO("ALUTest2 - ERROR: Can not load Emissive shader!\n");
		return init_status;
	}

	// Setup the Lights shader uniforms
	OpenGLStateManager::GlUseProgram(m_shaderLights->m_p);
	glUniform1i(m_shaderLights->m_uniform_locations[GLB::uniforms::texture_unit0], 0); // color map
	glUniform1i(m_shaderLights->m_uniform_locations[GLB::uniforms::texture_unit1], 1); // normal map
	glUniform1i(m_shaderLights->m_uniform_locations[GLB::uniforms::texture_unit3], 2); // depth map
 
	// Set the camera
	GLint loc_view_pos = glGetUniformLocation(m_shaderLights->m_p, "view_pos");
	GLint loc_depth_parameters = glGetUniformLocation(m_shaderLights->m_p, "depth_parameters");
	glUniform3fv(loc_view_pos, 1, m_camera.view_pos.v);
	glUniform4fv(loc_depth_parameters, 1, m_camera.depth_parameters.v);

	// Set the lights	
	m_uniform_light_pos_array = glGetUniformLocation(m_shaderLights->m_p, "light_pos_atten_array");
    m_uniform_light_color_array = glGetUniformLocation(m_shaderLights->m_p, "light_color_array");
    glUniform3fv(m_uniform_light_color_array, NUM_LIGHTS_PER_LOOP, m_light_color_array[0].v);
        
    // Setup the Ambient and Emissive shader uniforms
    OpenGLStateManager::GlUseProgram(m_shaderAmbEmit->m_p);
    glUniform1i(m_shaderAmbEmit->m_uniform_locations[GLB::uniforms::texture_unit0], 0); // color map    
	OpenGLStateManager::GlUseProgram(0);

	// Calculate the aspect ratio and the uv scaling
	float viewport_width = getViewportWidth();
	float viewport_height = getViewportHeight();				
	bool landscape;
	KCL::Vector4D scaled_uv = FitViewportToGBuffer(viewport_width, viewport_height, landscape);

	// Upload the vertex data
	struct Vertex
	{
		KCL::Vector4D pos_uv;
		KCL::Vector4D corners; // the corners of the view frustum
	};
	Vertex vertices[4];
	
    vertices[0].pos_uv.z = scaled_uv.x; vertices[0].pos_uv.w = scaled_uv.y;
	vertices[1].pos_uv.z = scaled_uv.x; vertices[1].pos_uv.w = scaled_uv.w;
	vertices[2].pos_uv.z = scaled_uv.z; vertices[2].pos_uv.w = scaled_uv.y;
	vertices[3].pos_uv.z = scaled_uv.z; vertices[3].pos_uv.w = scaled_uv.w;		
				
	vertices[0].corners = KCL::Vector4D(m_camera.corners[0], 0.0f);
	vertices[1].corners = KCL::Vector4D(m_camera.corners[2], 0.0f);
	vertices[2].corners = KCL::Vector4D(m_camera.corners[1], 0.0f);
	vertices[3].corners = KCL::Vector4D(m_camera.corners[3], 0.0f);

	if (landscape)
	{		
	    vertices[0].pos_uv.x = -1; vertices[0].pos_uv.y = -1;
	    vertices[1].pos_uv.x = -1; vertices[1].pos_uv.y = 1;
	    vertices[2].pos_uv.x = 1; vertices[2].pos_uv.y = -1;
	    vertices[3].pos_uv.x = 1; vertices[3].pos_uv.y = 1;	
	}
	else
	{    
        vertices[0].pos_uv.x = 1; vertices[0].pos_uv.y = -1;
	    vertices[1].pos_uv.x = -1; vertices[1].pos_uv.y = -1;
	    vertices[2].pos_uv.x = 1; vertices[2].pos_uv.y = 1;
	    vertices[3].pos_uv.x = -1; vertices[3].pos_uv.y = 1;
	}
	
	glGenVertexArrays(1, &m_LightsVao);
	glBindVertexArray(m_LightsVao);

	glGenBuffers(1, &m_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), (void*)vertices, GL_STATIC_DRAW);

	GLint loc_pos_uv = glGetAttribLocation(m_shaderLights->m_p, "in_pos_uv");
	GLint loc_corner = glGetAttribLocation(m_shaderLights->m_p, "in_corner");

	glEnableVertexAttribArray(loc_pos_uv);
	glVertexAttribPointer(loc_pos_uv, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

	glEnableVertexAttribArray(loc_corner);
	glVertexAttribPointer(loc_corner, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) sizeof(KCL::Vector4D));
	
	static const KCL::uint16 billboardIndices[] = { 0, 1, 2, 1, 2, 3 };
	glGenBuffers(1, &m_index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(KCL::uint16) * 6, (void*)billboardIndices, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &m_EmissiveVao);
    glBindVertexArray(m_EmissiveVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);

    loc_pos_uv = glGetAttribLocation(m_shaderAmbEmit->m_p, "in_pos_uv");
    glEnableVertexAttribArray(loc_pos_uv);
    glVertexAttribPointer(loc_pos_uv, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
   
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	return init_status;
}

KCL::uint32 ALUTest2::LoadTexture(const char * filename)
{
	KCL::Image image;
	if (!image.load(filename))
	{
		INFO("Error in ALUTest2::LoadTexture: Can not load image: %s", filename);
		return 0;
	}
	image.flipY();

	KCL::uint32 texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, G_BUFFER_WIDTH, G_BUFFER_HEIGHT);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, G_BUFFER_WIDTH, G_BUFFER_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, image.getData());

	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

bool ALUTest2::render()
{
    // First pass: Render the ambient/emissive pixels
	OpenGLStateManager::DisableAllCapabilites();
    OpenGLStateManager::GlUseProgram(m_shaderAmbEmit->m_p);
    
    OpenGLStateManager::GlBlendFunc(GL_ONE, GL_ZERO);
    OpenGLStateManager::GlEnable(GL_BLEND);

	OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_color_texture);
	glBindSampler(0, m_sampler);

	OpenGLStateManager::GlActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normal_texture);
	glBindSampler(1, m_sampler);

	OpenGLStateManager::GlActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_depth_texture);
	glBindSampler(2, m_sampler);

	OpenGLStateManager::Commit();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(m_EmissiveVao);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    
    // Second pass: Render and blend the lights
    OpenGLStateManager::GlUseProgram(m_shaderLights->m_p);
    
    OpenGLStateManager::GlBlendFunc(GL_ONE, GL_ONE);
    OpenGLStateManager::GlEnable(GL_BLEND);   
	OpenGLStateManager::Commit();

	glBindVertexArray(m_LightsVao);
    
    for (KCL::uint32 i = 0; i < m_animated_pos_array.size(); i += NUM_LIGHTS_PER_LOOP)
    {
        KCL::uint32 num_lights = m_animated_pos_array.size()-i;
        if( num_lights > NUM_LIGHTS_PER_LOOP )
            num_lights = NUM_LIGHTS_PER_LOOP;
        glUniform4fv(m_uniform_light_pos_array, num_lights, m_animated_pos_array[i].v);
        glUniform3fv(m_uniform_light_color_array, num_lights, m_light_color_array[i].v);

	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }

	glBindVertexArray(0);
	glBindSampler(0, 0);
	glBindSampler(1, 0);
	glBindSampler(2, 0);
	GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);		

	return true;
}

void ALUTest2::FreeResources()
{
	GLBShader2::DeleteShaders();
	m_shaderLights = 0;
    m_shaderAmbEmit = 0;
	
	glDeleteTextures(1, &m_color_texture);
	glDeleteTextures(1, &m_normal_texture);
	glDeleteTextures(1, &m_depth_texture);
	m_color_texture = 0;
	m_normal_texture = 0;
	m_depth_texture = 0;

	glDeleteSamplers(1, &m_sampler);	
	m_sampler = 0;
	
	glDeleteVertexArrays(1, &m_LightsVao);
    glDeleteVertexArrays(1, &m_EmissiveVao);
	glDeleteBuffers(1, &m_vertex_buffer);
	glDeleteBuffers(1, &m_index_buffer);	
	m_LightsVao = 0;
    m_EmissiveVao = 0;
	m_vertex_buffer = 0;
	m_index_buffer = 0;
	
	ALUTest2_Base::FreeResources();
}

#if 0
void ALUTest2::SaveScene()
{
	FILE * file = fopen("alu2_scene.txt", "wt");
	fprintf(file, "view_pos %f %f %f\n", m_camera.view_pos.x, m_camera.view_pos.y, m_camera.view_pos.z);
	fprintf(file, "depth_params %f %f %f %f\n", m_camera.depth_parameters.x, m_camera.depth_parameters.y, m_camera.depth_parameters.z, m_camera.depth_parameters.w);
	fprintf(file, "corners0 %f %f %f %f\n", m_camera.corners[0].x, m_camera.corners[0].y, m_camera.corners[0].z, m_camera.corners[0].w);
	fprintf(file, "corners1 %f %f %f %f\n", m_camera.corners[1].x, m_camera.corners[1].y, m_camera.corners[1].z, m_camera.corners[1].w);
	fprintf(file, "corners2 %f %f %f %f\n", m_camera.corners[2].x, m_camera.corners[2].y, m_camera.corners[2].z, m_camera.corners[2].w);
	fprintf(file, "corners3 %f %f %f %f\n", m_camera.corners[3].x, m_camera.corners[3].y, m_camera.corners[3].z, m_camera.corners[3].w);
	fprintf(file, "robot_pos %f %f %f\n", -8.471331f, 0.0f, 96.0f);
	fprintf(file, "front_lights %f %f %f\n", -4.734f, 0.5f, 118.976f);
	fprintf(file, "color_count %d\n", m_light_color_array.size());
	for (KCL::uint32 i = 0; i < m_light_color_array.size(); i++)
	{
		fprintf(file, "%f %f %f\n", m_light_color_array[i].x, m_light_color_array[i].y, m_light_color_array[i].z);
	}
	fprintf(file, "light_count %d\n", m_light_pos_array.size());
	for (KCL::uint32 i = 0; i < m_light_pos_array.size(); i++)
	{
		fprintf(file, "%f %f %f %f\n", m_light_pos_array[i].x, m_light_pos_array[i].y, m_light_pos_array[i].z,  m_light_pos_array[i].w);
	}
	
	fclose(file);
}
#endif