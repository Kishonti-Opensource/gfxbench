/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gl_blending.h"
#include "kcl_io.h"
#include "platform.h"
#include "opengl/glb_opengl_state_manager.h"
#include "opengl/glbshader.h"
#include "opengl/glb_texture.h"
#include "opengl/misc2_opengl.h"


UITest::UITest(const GlobalTestEnvironment* const gte)
	: UITest_Base(gte),
	m_scaleX(0),
	m_scaleY(0),
	m_vertexBuffer(0),
    m_indexBuffer(0),
    m_shader (0)
{
}


UITest::~UITest()
{
	FreeResources();
}


KCL::KCL_Status UITest::init()
{
	KCL::AssetFile vertex_file("shaders_40/lowlevel/ui.vs");
	KCL::AssetFile fragment_file("shaders_40/lowlevel/ui.fs");
	const char *vertexShader = vertex_file.GetBuffer();
	const char *fragmentShader = fragment_file.GetBuffer();

	m_shader = GLB::initProgram(vertexShader, fragmentShader, true, GetSetting().m_force_highp);
	if (!m_shader)
	{
		INFO("Error in LowLevelTest::initShaders. Shader error.");
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	m_scaleLocation = glGetUniformLocation(m_shader, "u_scale");
	m_offsetLocation = glGetUniformLocation(m_shader, "u_offset");
	m_samplerLocation = glGetUniformLocation(m_shader, "tex2D");

	if (glGetError())
	{
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	static const float screenBillboard[] =
	{
		-1, -1, 0.5f, 0, 0,
		-1,  1, 0.5f, 0, 1,
		1, -1, 0.5f, 1, 0,
		1,  1, 0.5f, 1, 1
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


KCL::Texture* UITest::createTexture(KCL::Image *image)
{
	return new GLB::GLBTextureES2(image) ;
}


bool UITest::render()
{
	GLB::OpenGLStateManager::DisableAllCapabilites();
	GLB::OpenGLStateManager::DisableAllVertexAttribs();
	GLB::OpenGLStateManager::GlUseProgram(m_shader);
	GLB::OpenGLStateManager::GlBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
	GLB::OpenGLStateManager::GlEnable(GL_BLEND);
	GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
	GLB::OpenGLStateManager::GlBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLB::OpenGLStateManager::GlEnableVertexAttribArray(0);
	GLB::OpenGLStateManager::GlEnableVertexAttribArray(5);
	GLB::OpenGLStateManager::Commit();

	glClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (m_scaleX == 0)
	{
		GLint params[4];
		glGetIntegerv(GL_VIEWPORT, params);
		m_scaleX = 1.0f / params[2];
		m_scaleY = 1.0f / params[3];
		m_aspectRatio = (float)(params[2]) / params[3];

		createItems(params[2], params[3]);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_stride, 0);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, m_stride, (void*) (3 * sizeof(float)));

	for (int i = 0; i < m_displayedElementCount; i++)
	{
		TestUIElement* e = m_uiElements[i % m_uiElements.size()] ;
		TestUIElement* element = dynamic_cast<TestUIElement*>( e );
		if (!element->m_texture)
		{
			element->LoadTexture();
		}

		float param = m_time * 0.0001 * (5 + (i & 11));
		float offsetX = 0.2f * m_scaleX * element->m_width * cos(param);
		float offsetY = 0.2f * m_scaleY * element->m_height * sin(param);

		element->m_texture->bind(0);
		glUniform2f(m_scaleLocation, m_scaleX * element->m_width, m_scaleY * element->m_height);
		glUniform2f(m_offsetLocation, offsetX + element->m_positionX, offsetY + element->m_positionY);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		if (glGetError())
		{
			while (glGetError());
			break;
		}
		else
		{
			m_transferredBytes += (element->m_width * element->m_height) << 2;
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}


void UITest::FreeResources()
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
}
