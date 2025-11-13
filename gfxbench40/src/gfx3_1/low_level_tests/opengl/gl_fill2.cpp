/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gl_fill2.h"
#include "platform.h"
#include "opengl/glb_opengl_state_manager.h"
#include "opengl/glb_texture.h"
#include "opengl/glb_shader2.h"
#include "opengl/fbo.h"

#include <limits.h> // UINT_MAX

CompressedFillTest2::CompressedFillTest2(const GlobalTestEnvironment* const gte) : CompressedFillTest2_Base(gte)
{
	m_vao_portrait = 0;
    m_vao_landscape = 0;
	m_vertex_buffer_portait = 0;
    m_vertex_buffer_landscape = 0;
	m_index_buffer = 0;

	m_fullscreen_background = 0;
	m_fullscreen_mask = 0;
	m_cube_texture = NULL;
	m_depth_texture = 0;
	m_near_sampler = 0;

	m_shader = NULL;
	m_shader_cube = NULL;
	m_shader_resize = NULL;

	m_offsets_location = 0;
	m_rotation_location = 0;
	m_rotation_location_cube = 0;

	m_viewport_width = 0;
	m_viewport_height = 0;
}

CompressedFillTest2::~CompressedFillTest2()
{
	FreeResources();
}


void CompressedFillTest2::renderApiFinish()
{
	glFinish();
}


KCL::KCL_Status CompressedFillTest2::init()
{
	m_viewport_width = getViewportWidth();
	m_viewport_height = getViewportHeight();

    bool landscape = m_window_height <= m_window_width || (GetSetting().GetScreenMode() != 0) || GetSetting().m_virtual_resolution;
		
	// Load the shaders
	GLB::GLBShader2::InitShaders(KCL::SV_30, GetSetting().m_force_highp);

	KCL::KCL_Status error = KCL::KCL_TESTERROR_NOERROR;
	GLB::GLBShaderBuilder sb;

	m_shader = sb
		.AddShaderDir("shaders_40/lowlevel2/")
		.ShaderFile("compressedfill2.shader")
		.Build(error);
	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error;
	}

	m_shader_cube = sb
		.AddShaderDir("shaders_40/lowlevel2/")
		.AddDefine("CUBE_DEPTH_PASS")
		.ShaderFile("compressedfill2.shader")
		.Build(error);
	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error;
	}

	m_shader_resize = sb
		.AddShaderDir("shaders_40/lowlevel2/")
		.AddDefine("RESIZE_PASS")
		.ShaderFile("compressedfill2.shader")
		.Build(error);
	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error;
	}

	// Setup the shaders
	float aspectRatio;
    if (landscape)
    {
        aspectRatio = m_viewport_width / m_viewport_height;
    }
    else
    {
         aspectRatio = m_viewport_height / m_viewport_width;
    }
	GLB::OpenGLStateManager::GlUseProgram(m_shader->m_p);
	glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
	glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::texture_unit1], 1);
	glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::texture_unit2], 2);
	glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::texture_unit3], 3);
	glUniform1f(glGetUniformLocation(m_shader->m_p, "u_aspectRatio"), aspectRatio);
	m_rotation_location = glGetUniformLocation(m_shader->m_p, "u_rotation");
	m_offsets_location = glGetUniformLocation(m_shader->m_p, "u_offsets");
    glBindAttribLocation(m_shader->m_p, 0, "in_vertex");
	
	GLB::OpenGLStateManager::GlUseProgram(m_shader_cube->m_p);
	glUniform1i(m_shader_cube->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
	glUniform1i(m_shader_cube->m_uniform_locations[GLB::uniforms::texture_unit2], 2);
	glUniform1i(m_shader_cube->m_uniform_locations[GLB::uniforms::texture_unit3], 3);
	glUniform1i(m_shader_cube->m_uniform_locations[GLB::uniforms::envmap0], 1); // envmap0 - cube map
	glUniform1f(glGetUniformLocation(m_shader_cube->m_p, "u_aspectRatio"), aspectRatio);
	m_rotation_location_cube = glGetUniformLocation(m_shader_cube->m_p, "u_rotation");
    glBindAttribLocation(m_shader_cube->m_p, 0, "in_vertex");

	GLB::OpenGLStateManager::GlUseProgram(m_shader_resize->m_p);
	glUniform1i(m_shader_resize->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
    glBindAttribLocation(m_shader_resize->m_p, 0, "in_vertex");

	// Load the images	
	std::string imageFolder;
	if (GLB::g_extension->isES())
	{
		imageFolder = "fill2/images_ETC2";	
	}
	else
	{
		imageFolder = "fill2/images_DXT5";	
	}
		
	KCL::Image image_2048;
	KCL::Image image_1024;
	KCL::Image image_128;
	KCL::Image image_uncompressed_1920_background;
	KCL::Image image_uncompressed_1920_mask;
	KCL::Image image_cube;
		
	if (!image_2048.load((imageFolder + std::string("/fill_tex_2048.png")).c_str()))
	{
		INFO("Error in CompressedFillTest31::init. Can not load image: %s", image_2048.getName().c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
	if (!image_1024.load((imageFolder + std::string("/fill_tex_1024.png")).c_str()))
	{
		INFO("Error in CompressedFillTest31::init. Can not load image: %s", image_1024.getName().c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
	if (!image_128.load((imageFolder + std::string("/fill_tex_128.png")).c_str()))
	{
		INFO("Error in CompressedFillTest31::init. Can not load image: %s", image_128.getName().c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
	if (!image_uncompressed_1920_background.load("fill2/fill_texture_bg.png"))
	{
		INFO("Error in CompressedFillTest31::init. Can not load image: %s", image_uncompressed_1920_background.getName().c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
    image_uncompressed_1920_background.flipY();
	if (!image_uncompressed_1920_mask.load("fill2/fill_texture_alpha.png"))
	{
		INFO("Error in CompressedFillTest31::init. Can not load image: %s", image_uncompressed_1920_mask.getName().c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
    image_uncompressed_1920_mask.flipY();
	if (!image_cube.loadCube((imageFolder + std::string("/envmap000")).c_str()))
	{
		INFO("Error in CompressedFillTest31::init. Can not load cube images: %s", image_cube.getName().c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
	
	m_render_passes[0].textures[0] = CreateTexture(&image_1024);
	m_render_passes[0].textures[1] = CreateTexture(&image_128);
	m_render_passes[0].textures[2] = CreateTexture(&image_1024);
	m_render_passes[0].textures[3] = CreateTexture(&image_2048);	

	m_render_passes[1].textures[0] = CreateTexture(&image_128);
	m_render_passes[1].textures[1] = CreateTexture(&image_128);
	m_render_passes[1].textures[2] = CreateTexture(&image_1024);
	m_render_passes[1].textures[3] = CreateTexture(&image_2048);	

	m_render_passes[2].textures[0] = CreateTexture(&image_1024);
	m_render_passes[2].textures[1] = CreateTexture(&image_128);
	m_render_passes[2].textures[2] = CreateTexture(&image_1024);
	m_render_passes[2].textures[3] = CreateTexture(&image_2048);	
	
	m_render_passes[3].textures[0] = CreateTexture(&image_2048);
	m_render_passes[3].textures[1] = CreateTexture(&image_128);
	m_render_passes[3].textures[2] = CreateTexture(&image_1024);
	m_render_passes[3].textures[3] = CreateTexture(&image_2048);

    if (landscape)
    {
        m_render_passes[0].offsets = KCL::Vector4D(-0.5f, -0.5f, 0.0f, 0.0f);
        m_render_passes[1].offsets = KCL::Vector4D(0.5f, -0.5f, 0.5f, 0.0f);
        m_render_passes[2].offsets = KCL::Vector4D(-0.5f, 0.5f, 0.0f, 0.5f);
        m_render_passes[3].offsets = KCL::Vector4D(0.5f, 0.5f, 0.5f, 0.5f);
    }
    else
    {
        m_render_passes[0].offsets = KCL::Vector4D(0.5f, -0.5f, 0.0f, 0.0f);
        m_render_passes[1].offsets = KCL::Vector4D(0.5f, 0.5f, 0.5f, 0.0f);
        m_render_passes[2].offsets = KCL::Vector4D(-0.5f, -0.5f, 0.0f, 0.5f);
        m_render_passes[3].offsets = KCL::Vector4D(-0.5f, 0.5f, 0.5f, 0.5f);
    }
		
	// Create near sampler for "depth" and "G buffer" sampling
	glGenSamplers(1, &m_near_sampler);
	glSamplerParameteri(m_near_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(m_near_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(m_near_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(m_near_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
		
    static const int stride = 4 * sizeof(float); // distance between the beginning of vertices
    KCL::Vector4D screenBillboard[4];
    screenBillboard[0] = KCL::Vector4D(-0.5f, -0.5f, 0, 0);
    screenBillboard[1] = KCL::Vector4D(-0.5f,  0.5f, 0, 1);
    screenBillboard[2] = KCL::Vector4D(0.5f, -0.5f, 1, 0);
    screenBillboard[3] = KCL::Vector4D(0.5f,  0.5f, 1, 1);    
    
	// Create the landscape VAO
    glGenVertexArrays(1, &m_vao_landscape);
	glBindVertexArray(m_vao_landscape);

	// Upload the vertex data
    glGenBuffers(1, &m_vertex_buffer_landscape);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_landscape);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, (void*)screenBillboard[0].v, GL_STATIC_DRAW);
    
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, 0);

    static const KCL::uint16 billboardIndices[] = { 0, 1, 2, 1, 2, 3 };
	glGenBuffers(1, &m_index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(KCL::uint16) * 6, (void*)billboardIndices, GL_STATIC_DRAW);
        
	// Create the portrait VAO
    screenBillboard[0] = KCL::Vector4D(0.5f, -0.5f, 0, 0); 
    screenBillboard[1] = KCL::Vector4D(-0.5f, -0.5f, 0, 1);
    screenBillboard[2] = KCL::Vector4D(0.5f, 0.5f, 1, 0);
    screenBillboard[3] = KCL::Vector4D(-0.5f,  0.5f, 1, 1);     

    glGenVertexArrays(1, &m_vao_portrait);
	glBindVertexArray(m_vao_portrait);

	// Upload the vertex data
    glGenBuffers(1, &m_vertex_buffer_portait);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_portait);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, (void*)screenBillboard[0].v, GL_STATIC_DRAW);
    
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
        
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
    if (landscape)
    {
        m_vao = m_vao_landscape;
    }
    else
    {
        m_vao = m_vao_portrait;
    }    

	// Create the textures	
	GLB::GLBTextureES3 * fullhd_background_texture = CreateTexture(&image_uncompressed_1920_background);	
	GLB::GLBTextureES3 * fullhd_mask_texture = CreateTexture(&image_uncompressed_1920_mask);	
	CreateFullScreenTextures(fullhd_background_texture->textureObject(), fullhd_mask_texture->textureObject());
	CreateDepthTexture();
	m_cube_texture = CreateTexture(&image_cube);	
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	return KCL::KCL_TESTERROR_NOERROR;
}

GLB::GLBTextureES3 * CompressedFillTest2::CreateTexture(KCL::Image * image)
{
	GLB::GLBTextureES3 * texture = new GLB::GLBTextureES3(image, false);
	texture->setMinFilter(KCL::TextureFilter_Linear);
	texture->setMagFilter(KCL::TextureFilter_Linear);
	texture->setMipFilter(KCL::TextureFilter_Linear);
	texture->setWrapS(KCL::TextureWrap_Repeat);
	texture->setWrapT(KCL::TextureWrap_Repeat);
	texture->commit();
	m_textures.push_back(texture);
	return texture;
}

void CompressedFillTest2::CreateFullScreenTextures(KCL::uint32 src_texture1, KCL::uint32 src_texture2)
{
	GLint saved_viewport[4];
	glGetIntegerv(GL_VIEWPORT, saved_viewport);

    KCL::uint32 width = m_viewport_width;
    KCL::uint32 height = m_viewport_height;
    if (m_viewport_width < m_viewport_height)
    {
        width = m_viewport_height;
        height = m_viewport_width;
    }

	glGenTextures(1, &m_fullscreen_background);
	glBindTexture(GL_TEXTURE_2D, m_fullscreen_background);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

	glGenTextures(1, &m_fullscreen_mask);
	glBindTexture(GL_TEXTURE_2D, m_fullscreen_mask);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

	GLuint fbo = 0;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	const GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	glViewport(0, 0, width, height);

	GLB::OpenGLStateManager::GlUseProgram(m_shader_resize->m_p);
	glUseProgram(m_shader_resize->m_p);
	glBindVertexArray(m_vao_landscape);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, src_texture1);
	glBindSampler(0, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fullscreen_background, 0);	
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, src_texture2);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fullscreen_mask, 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	GLB::OpenGLStateManager::GlUseProgram(0);	
	GLB::FBO::bind(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);	
	glDeleteFramebuffers(1, &fbo);

	glViewport(saved_viewport[0], saved_viewport[1], saved_viewport[2], saved_viewport[3]);
}

void CompressedFillTest2::CreateDepthTexture()
{	
	const KCL::uint32 pixel_count = m_viewport_width * m_viewport_height;

	std::vector<GLuint> depth_data;
	depth_data.resize(pixel_count);

	KCL::int32 seed = 42;
	for (KCL::uint32 i = 0; i < pixel_count; i++)
	{
		depth_data[i] = GLuint(UINT_MAX * KCL::Math::randomf(&seed));
	}
	glGenTextures(1, &m_depth_texture);
	glBindTexture(GL_TEXTURE_2D, m_depth_texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, m_viewport_width, m_viewport_height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_viewport_width, m_viewport_height, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, depth_data.data());
	glBindTexture(GL_TEXTURE_2D, 0);	
}

bool CompressedFillTest2::render()
{	
	GLB::OpenGLStateManager::DisableAllCapabilites();
	GLB::OpenGLStateManager::GlEnable(GL_BLEND);
	GLB::OpenGLStateManager::Commit();

	glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);	
	
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(m_vao);

	const float exposure_time = 800.0f;	
	float rotation = 0.0f;
	
	KCL::uint32 pass_index = 0;
	GLB::OpenGLStateManager::GlUseProgram(m_shader->m_p);
	for (int i = 0; i < m_displayed_element_count; i++)
	{	
		rotation = (m_time + exposure_time * i / float(m_displayed_element_count)) * 0.0003f;
		glBlendColor(1, 1, 1, 1.0f / (1 + i));

		for (KCL::uint32 j = 0; j < m_render_pass_count; j++)
		{						
			pass_index = j % m_render_pass_count;
			m_render_passes[pass_index].textures[0]->bind(0);
			m_render_passes[pass_index].textures[1]->bind(1);
			m_render_passes[pass_index].textures[2]->bind(2);
			m_render_passes[pass_index].textures[3]->bind(3);		

			glUniform1f(m_rotation_location, rotation);

			glUniform4fv(m_offsets_location, 1, m_render_passes[pass_index].offsets.v);
			
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);	
		}	
	}
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

	GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fullscreen_background);
	glBindSampler(0, m_near_sampler);

	m_cube_texture->bind(1);

	GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_depth_texture);
	glBindSampler(2, m_near_sampler);

	GLB::OpenGLStateManager::Commit();
	GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D,  m_fullscreen_mask);
	glBindSampler(3, m_near_sampler);

	GLB::OpenGLStateManager::GlUseProgram(m_shader_cube->m_p);

	rotation = (m_time + exposure_time) * 0.0003f;
	glUniform1f(m_rotation_location_cube, rotation);
		
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	GLB::OpenGLStateManager::GlUseProgram(0);
	glBindSampler(0, 0);
	glBindSampler(1, 0);
	glBindSampler(2, 0);
	glBindSampler(3, 0);
	GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);		

	glBindVertexArray(0);

	const KCL::uint32 pass_textels = m_viewport_width * m_viewport_height * (4 + 4 * m_displayed_element_count);
	m_transferred_texels += pass_textels;

	GLB::OpenGLStateManager::DisableAllCapabilites();
	GLB::OpenGLStateManager::GlDisable(GL_BLEND);
	GLB::OpenGLStateManager::Commit();

	return true;
}


void CompressedFillTest2::FreeResources()
{
	GLB::GLBShader2::DeleteShaders();
	m_shader = 0;
	m_shader_cube = 0;
	m_shader_resize = 0;

	glDeleteVertexArrays(1, &m_vao_landscape);
    glDeleteVertexArrays(1, &m_vao_portrait);
    glDeleteBuffers(1, &m_vertex_buffer_landscape);	
    glDeleteBuffers(1, &m_vertex_buffer_portait);	
	glDeleteBuffers(1, &m_index_buffer);	
	m_vao_landscape = 0;
    m_vao_portrait = 0;
	m_vertex_buffer_landscape = 0;
    m_vertex_buffer_portait = 0;
	m_index_buffer = 0;

	glDeleteTextures(1, &m_fullscreen_background);
	glDeleteTextures(1, &m_fullscreen_mask);
	glDeleteTextures(1, &m_depth_texture);
	m_fullscreen_background = 0;
	m_fullscreen_mask = 0;
	m_depth_texture = 0;
	m_cube_texture = 0;

	glDeleteSamplers(1, &m_near_sampler);
	m_near_sampler = 0;

	for (KCL::uint32 i = 0; i < m_textures.size(); i++)
	{
		delete m_textures[i];
	}
	m_textures.clear();

	for (KCL::uint32 i = 0; i < m_render_pass_count; i++)
	{
		m_render_passes[i].Clear();
	}
}

