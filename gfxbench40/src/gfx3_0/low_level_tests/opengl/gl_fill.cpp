/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gl_fill.h"
#include "kcl_io.h"
#include "opengl/glb_opengl_state_manager.h"
#include "opengl/glbshader.h"
#include "platform.h"
#include "opengl/glb_texture.h"
#include "opengl/misc2_opengl.h"

CompressedFillTest::CompressedFillTest(const GlobalTestEnvironment* const gte) : CompressedFillTest_Base(gte),
    m_aspectRatio(0.0f),
    m_vertexBuffer(0),
    m_indexBuffer(0),
    m_shader (0)
{
}


CompressedFillTest::~CompressedFillTest()
{
	FreeResources();
}


KCL::Texture* CompressedFillTest::CreateTexture(KCL::Image* img)
{
	return new GLB::GLBTextureES2(img);	
}


KCL::KCL_Status CompressedFillTest::init()
{
	KCL::KCL_Status error = CompressedFillTest_Base::init() ;

	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error ;
	}

	KCL::AssetFile vertex_file("shaders_40/lowlevel/compressedfill.vs");
	KCL::AssetFile fragment_file("shaders_40/lowlevel/compressedfill.fs");
	const char *vertexShader = vertex_file.GetBuffer();
	const char *fragmentShader = fragment_file.GetBuffer();

	m_shader = GLB::initProgram(vertexShader, fragmentShader, true, GetSetting().m_force_highp);
	if (!m_shader)
	{
		INFO("Error in LowLevelTest::initShaders. Shader error.");
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}


	m_rotationLocation = glGetUniformLocation(m_shader, "u_rotation");
	m_uniAspectRatio = glGetUniformLocation(m_shader, "u_aspectRatio");
	m_sampler0Location = glGetUniformLocation(m_shader, "texture0");
	m_sampler1Location = glGetUniformLocation(m_shader, "texture1");

	if (glGetError())
	{
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	static const float screenBillboard[] =
	{
		-1, -1, 0.5f, 0, 1,
		-1,  1, 0.5f, 0, 0,
		1, -1, 0.5f, 1, 1,
		1,  1, 0.5f, 1, 0
	};

	glGenBuffers(1, &m_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glBufferData_chunked(GL_ARRAY_BUFFER, sizeof(float) * 20, (void*)screenBillboard, GL_STATIC_DRAW);
	if (glGetError())
	{
		return KCL::KCL_TESTERROR_VBO_ERROR;
	}

	static const KCL::uint16 billboardIndices[] = { 0, 1, 2, 1, 2, 3 };
	glGenBuffers(1, &m_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
	glBufferData_chunked(GL_ELEMENT_ARRAY_BUFFER, sizeof(KCL::uint16) * 6, (void*)billboardIndices, GL_STATIC_DRAW);
	if (glGetError())
	{
		return KCL::KCL_TESTERROR_VBO_ERROR;
	}

	return KCL::KCL_TESTERROR_NOERROR;
}


bool CompressedFillTest::render()
{
	GLB::OpenGLStateManager::DisableAllCapabilites();
	GLB::OpenGLStateManager::DisableAllVertexAttribs();
	GLB::OpenGLStateManager::GlUseProgram(m_shader);
	GLB::OpenGLStateManager::GlEnable(GL_BLEND);
	GLB::OpenGLStateManager::GlBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
	GLB::OpenGLStateManager::GlEnableVertexAttribArray(0);
	GLB::OpenGLStateManager::GlEnableVertexAttribArray(5);
	GLB::OpenGLStateManager::Commit();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	if (m_aspectRatio == 0.0f)
	{
		GLint params[4];
		glGetIntegerv(GL_VIEWPORT, params);
		m_aspectRatio = (float)(params[2]) / params[3];
	}


	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_stride, 0);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, m_stride, (void*) (3 * sizeof(float)));

	m_colorTexture->bind(0);
	m_lightTexture->bind(1);

	glUniform1i(m_sampler0Location , 0);
	glUniform1i(m_sampler1Location, 1);
	glUniform1f(m_uniAspectRatio, m_aspectRatio);

	const float exposureTime = 150;
	for (int i = 0; i < m_displayedElementCount; i++)
	{
		glBlendColor(1, 1, 1, 1.0f / (1 + i));
		float rotation = (m_time + exposureTime * i / m_displayedElementCount) * 0.0003;
		glUniform1f(m_rotationLocation, rotation);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	}

	const unsigned int texelsPerScreen =  m_settings->m_viewport_width * m_settings->m_viewport_height * 4;
	m_transferredTexels += texelsPerScreen * m_displayedElementCount;

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}


void CompressedFillTest::FreeResources()
{
	if (m_shader)
	{
		glDeleteProgram(m_shader);
		m_shader = 0;
	}

	if(m_vertexBuffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &m_vertexBuffer);
		m_vertexBuffer = 0;
	}

	if (m_indexBuffer)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &m_indexBuffer);
		m_indexBuffer = 0;
	}

	CompressedFillTest_Base::FreeResources() ;
}

