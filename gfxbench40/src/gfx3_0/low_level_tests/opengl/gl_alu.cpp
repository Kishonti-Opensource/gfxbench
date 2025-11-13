/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gl_alu.h"
#include "kcl_io.h"
#include "platform.h"
#include "opengl/glbshader.h"
#include "opengl/glb_opengl_state_manager.h"
#include "kcl_math3d.h"
#include "opengl/misc2_opengl.h"


ALUTest::ALUTest(const GlobalTestEnvironment* const gte) : ALUTest_Base(gte),
	m_vertexBuffer(0),
    m_indexBuffer(0),
    m_shader (0),
    m_aspectRatio(0, 0),
    m_isRotated(false),
	m_vectorized(false)
{
}


ALUTest::~ALUTest()
{
	FreeResources();
}

KCL::KCL_Status ALUTest::init()
{
	KCL::AssetFile vertex_file("shaders_40/lowlevel/alu.vs");

	std::string fsFileName;
	if (m_vectorized) {
		fsFileName = "shaders_40/lowlevel/alu_vectorized.fs";
	} else {
		fsFileName = "shaders_40/lowlevel/alu.fs";
	}

	KCL::AssetFile fragment_file(fsFileName);

	if (!vertex_file.Opened())
	{
		INFO("missing file: %s", fsFileName.c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
	
	if (!fragment_file.Opened())
	{
		INFO("missing file: %s", fsFileName.c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}

	const char *vertexShader = vertex_file.GetBuffer();
	const char *fragmentShader = fragment_file.GetBuffer();

	m_shader = GLB::initProgram(vertexShader, fragmentShader, true, GetSetting().m_force_highp);

	if (!m_shader)
	{
		INFO("ERROR: Error in ALUTest::initShaders. Shader error.");
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	m_uniTimeLocation = glGetUniformLocation(m_shader, "u_time");
	m_uniAspectRatio = glGetUniformLocation(m_shader, "u_aspectRatio");
	m_uniLightDirLocation = glGetUniformLocation(m_shader, "u_lightDir");
	m_uniPositionLocation = glGetUniformLocation(m_shader, "u_eyePosition");
	m_uniOrientationLocation = glGetUniformLocation(m_shader, "u_orientation");

	if (glGetError())
	{
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	static const float screenBillboard[] =
	{
		-1, -1, 0, 0, 0,
		-1,  1, 0, 0, 1,
		 1, -1, 0, 1, 0,
		 1,  1, 0, 1, 1
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


bool ALUTest::render()
{
	GLB::OpenGLStateManager::DisableAllCapabilites();
	GLB::OpenGLStateManager::DisableAllVertexAttribs();
	GLB::OpenGLStateManager::GlUseProgram(m_shader);
	GLB::OpenGLStateManager::GlEnableVertexAttribArray(0);
	GLB::OpenGLStateManager::GlEnableVertexAttribArray(5);
	GLB::OpenGLStateManager::GlBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
	GLB::OpenGLStateManager::GlDisable(GL_BLEND);
	glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
	GLB::OpenGLStateManager::Commit();

	if (m_aspectRatio.x == 0.0f)
	{
		GLint params[4];
		glGetIntegerv(GL_VIEWPORT, params);

		if( params[2] >= params[3])
		{
			m_aspectRatio.x = (float)params[2] / params[3];
			m_aspectRatio.y = 1.0f;
		}
		else
		{
			m_aspectRatio.x = 1.0f;
			m_aspectRatio.y = (float)params[3] / params[2];
            m_isRotated = true;
		}
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_stride, 0);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, m_stride, (void*) (3 * sizeof(float)));

	glUniform2fv(m_uniAspectRatio, 1, m_aspectRatio.v);
	float time = (m_time%1000)/1000.0f;
	glUniform1f(m_uniTimeLocation, time);

	float lightTime = m_time * 0.0003;
	KCL::Vector3D lightDir(sin(lightTime), 0.5 + 0.5 * cos(lightTime), 0.5 - 0.5 * cos(lightTime));
	lightDir.normalize();
	glUniform3f(m_uniLightDirLocation, lightDir.x, lightDir.y, lightDir.z);

	float flyTime = m_time * 0.0005;
	//glUniform3f(m_uniPositionLocation, 10 * sin(flyTime), sin(flyTime * 0.5) * 2 + 2.1, -flyTime * 10);
	glUniform3f(m_uniPositionLocation, 10 * sin(flyTime), sin(flyTime * 0.5) * 2 + 2.1, 10 * cos(flyTime));

	KCL::Matrix4x4 yaw, pitch, roll, orientation;
	KCL::Matrix4x4::RotateY(yaw, -cos(flyTime) * 28);
	KCL::Matrix4x4::RotateX(pitch, -cos(flyTime * 0.5) * 28);
	KCL::Matrix4x4::RotateZ(roll, - sin(flyTime) * 28);

	orientation = pitch * roll * yaw;

	if( m_isRotated)
	{
		KCL::Matrix4x4 t;
		KCL::Matrix4x4::RotateZ( t, -90);
		orientation = orientation * t;
	}

	glUniformMatrix4fv(m_uniOrientationLocation, 1, false, orientation);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}


void ALUTest::FreeResources()
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
