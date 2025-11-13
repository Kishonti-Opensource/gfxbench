/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gl_driver.h"
#include "kcl_io.h"
#include "opengl/glb_opengl_state_manager.h"
#include "platform.h"
#include "opengl/glbshader.h"
#include "opengl/misc2_opengl.h"
#include "kcl_math3d.h"

CPUOverheadTest::CPUOverheadTest(const GlobalTestEnvironment* const gte) : CPUOverheadTest_Base(gte),
	m_screenWidth(0),
    m_screenHeight(0),
    m_shader (0)
{
	for (int i = 0; i < BufferCount; i++)
	{
		m_vertexBuffers[i] = 0;
		m_indexBuffers[i] = 0;
	}
}


CPUOverheadTest::~CPUOverheadTest()
{
	FreeResources();
}


KCL::KCL_Status CPUOverheadTest::init()
{
	m_score = 0;

	KCL::AssetFile vertex_file("shaders_40/lowlevel/cpuoverhead.vs");
	KCL::AssetFile fragment_file("shaders_40/lowlevel/cpuoverhead.fs");
	const char *vertexShader = vertex_file.GetBuffer();
	const char *fragmentShader = fragment_file.GetBuffer();

	m_shader = GLB::initProgram(vertexShader, fragmentShader, true, GetSetting().m_force_highp);
	if (!m_shader)
	{
		INFO("Error in LowLevelTest::initShaders. Shader error.");
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	m_uniPosition = glGetUniformLocation(m_shader, "u_position");
	m_uniScale = glGetUniformLocation(m_shader, "u_scale");
	m_uniRoration = glGetUniformLocation(m_shader, "u_rotation");
	m_uniMatrixSize = glGetUniformLocation(m_shader, "u_matrixSize");
	m_uniColor0 = glGetUniformLocation(m_shader, "u_color0");
	m_uniColor1 = glGetUniformLocation(m_shader, "u_color1");
	m_uniColor2 = glGetUniformLocation(m_shader, "u_color2");
	m_uniColor3 = glGetUniformLocation(m_shader, "u_color3");
	m_uniScreenResolution = glGetUniformLocation(m_shader, "u_screenResolution");

	if (glGetError())
	{
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	glGenBuffers(BufferCount, m_vertexBuffers);
	glGenBuffers(BufferCount, m_indexBuffers);

	int seed = 123;
	for (int i = 0; i < BufferCount; i++)
	{
		int vertexCount = i + 3;
		int triangleCount = vertexCount - 2;

		// Generate vertices
		float* vertices = new float[vertexCount * 5];
		float* vertPtr = vertices;
		float theta = 0.0f;
		float thetaStep = 6.283185307179586476925286766559f / vertexCount;
		for (int j = 0; j < vertexCount; j++, theta += thetaStep)
		{
			float cosTheta = cosf(theta);
			float sinTheta = sinf(theta);
			*(vertPtr++) = cosTheta;
			*(vertPtr++) = sinTheta;
			*(vertPtr++) = KCL::Math::randomf(&seed);		// Z value
			*(vertPtr++) = cosTheta * 0.5f + 0.5f;
			*(vertPtr++) = sinTheta * 0.5f + 0.5f;
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[i]);
		glBufferData_chunked(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 5, (void*)vertices, GL_STATIC_DRAW);
		if (glGetError())
		{
			return KCL::KCL_TESTERROR_VBO_ERROR;
		}

		KCL::uint16* indices = new KCL::uint16[triangleCount * 3];
		KCL::uint16* indPtr = indices;
		for (int j = 1; j <= triangleCount; j++)
		{
			*(indPtr++) = 0;
			*(indPtr++) = j;
			*(indPtr++) = j + 1;
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffers[i]);
		glBufferData_chunked(GL_ELEMENT_ARRAY_BUFFER, sizeof(KCL::uint16) * triangleCount * 3, (void*)indices, GL_STATIC_DRAW);
		if (glGetError())
		{
			return KCL::KCL_TESTERROR_VBO_ERROR;
		}

		delete[] vertices;
		delete[] indices;
	}

	return KCL::KCL_TESTERROR_NOERROR;
}


bool CPUOverheadTest::render()
{
	if (m_screenWidth == 0.0f)
	{
		GLint params[4];
		glGetIntegerv(GL_VIEWPORT, params);
		m_screenWidth = params[2];
		m_screenHeight = params[3];
	}

	float rotation = 0.003 * m_time;
	float sint = cosf(m_time * 0.0015707963267948966192313216916398f);
	glClearColor(0.25f, 0.75f - 0.25f * sint, 0.75f + 0.25f * sint, 1.0f);

	GLB::OpenGLStateManager::DisableAllCapabilites();
	GLB::OpenGLStateManager::DisableAllVertexAttribs();
	GLB::OpenGLStateManager::Commit();

	//glClearDepth(0.5 + 0.5 * sint); not available on OpenGL ES
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_shader);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(5);

	int seed = 0;
	const int ColumnCount = 50;
	const int RowCount = 50;
	int instanceIdx = 0; 
	for (int y = 0; y < RowCount; y++)
	{
		for (int x = 0; x < ColumnCount; x++)
		{
			float r0 = KCL::Math::randomf(&seed);
			float r1 = KCL::Math::randomf(&seed);
			float r2 = KCL::Math::randomf(&seed);
			float r3 = KCL::Math::randomf(&seed);
			int stateIdx = KCL::Math::randomf(&seed) * 140;
			instanceIdx++;
			seed += x + y;

			glUniform2f(m_uniPosition, x + 0.5f * (y & 1), y);
			glUniform2f(m_uniMatrixSize, ColumnCount, RowCount);

			glUniform1f(m_uniRoration, rotation * (0.2f + r2) + r0);
			glUniform2f(m_uniScale, r1 + 0.5f, r2 + 0.5f);
			glUniform2f(m_uniScreenResolution, m_screenWidth, m_screenHeight);

			glUniform4f(m_uniColor0, r0, r1, r2, r3);
			glUniform4f(m_uniColor1, r1, r2, r3, r0);
			glUniform4f(m_uniColor2, r2, r3, r0, r1);
			glUniform4f(m_uniColor3, r3, r0, r1, r2);

			glBlendColor(r3, r2, r1, r0);

			// Toggle between different settings
			int depthStateIdx = instanceIdx / 100;
			switch (depthStateIdx % 7)
			{
			case 0:
				glDisable(GL_DEPTH_TEST);
				glDepthFunc(GL_NEVER);
				break;

			case 1:
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LEQUAL);
				break;

			case 2:
				glDisable(GL_DEPTH_TEST);
				glDepthFunc(GL_EQUAL);
				break;

			case 3:
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_GREATER);
				break;

			case 4:
				glDisable(GL_DEPTH_TEST);
				glDepthFunc(GL_NOTEQUAL);
				break;

			case 5:
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_GEQUAL);
				break;

			case 6:
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_ALWAYS);
				break;
			}

			int blendStateIdx = instanceIdx / 10;
			switch (blendStateIdx % 5)
			{
			case 0:
				glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
				glEnable(GL_BLEND);
				break;

			case 1:
				glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
				glEnable(GL_BLEND);
				break;

			case 2:
				glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR);
				glEnable(GL_BLEND);
				break;

			case 3:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_BLEND);
				break;

			case 4:
				glBlendFunc(GL_ZERO, GL_SRC_COLOR);
				glEnable(GL_BLEND);
				break;

			case 5:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				glEnable(GL_BLEND);
				break;
			}

			int bufferIdx = stateIdx % BufferCount;
			int vertexCount = bufferIdx + 3;
			int triangleCount = vertexCount - 2;

			glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[bufferIdx]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffers[bufferIdx]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_stride, 0);
			glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, m_stride, (void*) (3 * sizeof(float)));

			glDrawElements(GL_TRIANGLES, triangleCount * 3, GL_UNSIGNED_SHORT, 0);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}


void CPUOverheadTest::FreeResources()
{
	if (m_shader)
	{
		glDeleteProgram(m_shader);
		m_shader = 0;
	}

	for (int i = 0; i < BufferCount; i++)
	{
		if(m_vertexBuffers[i])
		{
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDeleteBuffers(1, &m_vertexBuffers[i]);
			m_vertexBuffers[i] = 0;
		}

		if (m_indexBuffers[i])
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glDeleteBuffers(1, &m_indexBuffers[i]);
			m_indexBuffers[i] = 0;
		}
	}
}

