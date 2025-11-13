/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gl_driver2.h"
#include "kcl_io.h"
#include "opengl/glb_opengl_state_manager.h"
#include "platform.h"
#include "opengl/glb_shader2.h"
#include "opengl/misc2_opengl.h"
#include "kcl_math3d.h"
#include "opengl/fbo.h"

DriverOverheadTest2::DriverOverheadTest2(const GlobalTestEnvironment* const gte) : TestBase(gte),
    m_shader (0),
	m_global_vao(0),
	m_ibuf_sqr(0),
	m_vbuf_sqr(0),
	m_ibuf_tri(0),
	m_vbuf_tri(0),
	m_ibuf_dot(0),
	m_vbuf_dot(0),
	m_glowfbo(0),
	m_sampler(0),
	m_normal_tex(0),
	m_burnt_tex(0),
	m_burnt2_tex(0)
{
}


DriverOverheadTest2::~DriverOverheadTest2()
{
	FreeResources();
}


bool DriverOverheadTest2::animate(const int time)
{
	SetAnimationTime(time);

	int score = m_frames;
	if (score > m_score)
	{
		m_score = score;
	}

	return time < m_settings->m_play_time;
}

bool GenerateBuffer(GLuint* bufferid, GLenum buftype, size_t datasize, void* data)
{
	glGenBuffers(1, bufferid);
	glBindBuffer(buftype, *bufferid);
	glBufferData_chunked(buftype, datasize, data, GL_STATIC_DRAW);
	if (glGetError()) return false;
	return true;
}

GLB::GLBTexture* CreateTexture(KCL::uint8 red, KCL::uint8 green, KCL::uint8 blue)
{
	KCL::Image *img;
	img = new KCL::Image(2, 2, KCL::Image_RGBA8888);
	KCL::uint8* i = (KCL::uint8*)img->getData();
	for (KCL::uint8* i = (KCL::uint8*)img->getData(); i < (KCL::uint8*)img->getData() + img->getDataSize();)
	{
		*(i++) = red;
		*(i++) = green;
		*(i++) = blue;
		*(i++) = 255;
	}

	GLB::GLBTextureES3 *tex=new GLB::GLBTextureES3(img, true);
	tex->setMagFilter(KCL::TextureFilter_Nearest);
	tex->setMinFilter(KCL::TextureFilter_Nearest);
	tex->setMipFilter(KCL::TextureFilter_NotApplicable);
	tex->commit();
	return tex;
}

KCL::KCL_Status DriverOverheadTest2::init()
{
	while (glGetError() != GL_NO_ERROR);

	glGenVertexArrays(1, &m_global_vao);
	glBindVertexArray(m_global_vao);

	glGenSamplers(1, &m_sampler);
	glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	GLB::GLBShader2::InitShaders(KCL::SV_30, GetSetting().m_force_highp);

    if (m_window_height <= m_window_width || GetScreenMode() != 0 ||  GetSetting().m_virtual_resolution)
	{
		m_transformmat[0] = 1; m_transformmat[1] = 0;
		m_transformmat[2] = 0; m_transformmat[3] = 1;
	}
	else {
		m_transformmat[0] = 0; m_transformmat[1] = -1;
		m_transformmat[2] = 1; m_transformmat[3] = 0;
	}

	m_score = 0;

	GLB::GLBShaderBuilder sb;
	KCL::KCL_Status err;

	GLB::GLBShader2* shader;
	
	shader = sb.AddShaderDir("shaders_40/lowlevel2/").ShaderFile("driver2_solid.shader").Build(err);

	if (err!=KCL::KCL_TESTERROR_NOERROR)
	{
		INFO("Error in LowLevelTest::initShaders. Shader error (1).");
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}
	m_shader = shader->m_p;

	shader = sb.AddShaderDir("shaders_40/lowlevel2/").ShaderFile("driver2_quad.shader").Build(err);

	if (err != KCL::KCL_TESTERROR_NOERROR)
	{
		INFO("Error in LowLevelTest::initShaders. Shader error (2).");
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}
	m_renderglowshader = shader->m_p;

	m_uniPosition = glGetUniformLocation(m_shader, "u_position");
	m_uniColor = glGetUniformLocation(m_shader, "u_color");
	m_uniGridSize = glGetUniformLocation(m_shader, "u_gridsize");
	m_uniMargin = glGetUniformLocation(m_shader, "u_margin");
	m_uniGlobalMargin = glGetUniformLocation(m_shader, "u_globmargin");
	m_uniTransMat = glGetUniformLocation(m_shader, "u_transform_matrix");

	if (glGetError())
	{
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	float vertices_sqr[] = {
		-1.0f, -1.0f, 0.5f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.5f, 0.0f, 0.0f,
		-1.0f, 1.0f, 0.5f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.5f, 0.0f, 0.0f
	};
	KCL::uint16 indices_sqr[] = { 0, 1, 2, 3, 2, 1 };

	float vertices_tri[] = {
		-1.0f, -1.0f, 0.5f, 0.0f,
		1.0f, 0.0f, 0.5f, 0.0f,
		- 1.0f, 1.0f, 0.5f, 0.0f
	};
	KCL::uint16 indices_tri[] = { 0, 1, 2 };

	float vertices_dot[] = {
		0.0f, 0.0f, 0.5f,
		0.0f, 1.0f, 0.5f,
		0.75f, 0.75f, 0.5f,
		1.0f, 0.0f, 0.5f,
		0.75f, -0.75f, 0.5f,
		0.0f, -1.0f, 0.5f,
		-0.75f, -0.75f, 0.5f,
		-1.0f, 0.0f, 0.5f,
		-0.75f, 0.75f, 0.5f
	};
	KCL::uint16 indices_dot[] = { 0, 2, 1, 0, 3, 2, 0, 4, 3, 0, 5, 4, 0, 6, 5, 0, 7, 6, 0, 8, 7, 0, 1, 8 };

	if (!GenerateBuffer(&m_vbuf_sqr, GL_ARRAY_BUFFER, sizeof(vertices_sqr), vertices_sqr)) return KCL::KCL_TESTERROR_VBO_ERROR;
	if (!GenerateBuffer(&m_ibuf_sqr, GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_sqr), indices_sqr)) return KCL::KCL_TESTERROR_VBO_ERROR;
	if (!GenerateBuffer(&m_vbuf_tri, GL_ARRAY_BUFFER, sizeof(vertices_tri), vertices_tri)) return KCL::KCL_TESTERROR_VBO_ERROR;
	if (!GenerateBuffer(&m_ibuf_tri, GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_tri), indices_tri)) return KCL::KCL_TESTERROR_VBO_ERROR;
	if (!GenerateBuffer(&m_vbuf_dot, GL_ARRAY_BUFFER, sizeof(vertices_dot), vertices_dot)) return KCL::KCL_TESTERROR_VBO_ERROR;
	if (!GenerateBuffer(&m_ibuf_dot, GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_dot), indices_dot)) return KCL::KCL_TESTERROR_VBO_ERROR;

	m_glowfbo= new GLB::FBO(64, 64, 0, GLB::RGB888_Linear, GLB::DEPTH_None, "");

	m_normal_tex = CreateTexture(127, 127, 127);
	m_burnt_tex = CreateTexture(215, 195, 127);
	m_burnt2_tex = CreateTexture(50, 25, 90);

	glBindVertexArray(0);

	return KCL::KCL_TESTERROR_NOERROR;
}

inline float frac(float val)
{
	return val - floor(val);
}

void DriverOverheadTest2::RenderChannels(int RowCount, int ColumnCount, float* channels, float backgroundIntensity)
{
	glUniform2f(m_uniGridSize, 1, 1);
	glUniform2f(m_uniMargin, 0, 0);
	glUniform2f(m_uniGlobalMargin, 0, 0);
	glUniform3f(m_uniColor, backgroundIntensity, backgroundIntensity, backgroundIntensity);
	glUniform2f(m_uniPosition, 0, 0);
	glUniformMatrix2fv(m_uniTransMat, 1, 0, m_transformmat);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	m_normal_tex->bind(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbuf_sqr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibuf_sqr);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	for (int x = 0; x < ColumnCount; ++x)
	{
		for (int y = 0; y < RowCount; ++y)
		{
			int e = x + y*ColumnCount;
			if (channels[y] > x / (float)ColumnCount)
			{
				if (e % 153 == 0 || e % 255 == 95)
				{
					m_burnt_tex->bind(0);
				}
				else if (e % 46 == 0 || e % 163 == 95)
				{
					m_burnt2_tex->bind(0);
				}
				else
				{
					m_normal_tex->bind(0);
				}

			}
			else
			{
				m_normal_tex->bind(0);
			}
			glUniform2f(m_uniGridSize, ColumnCount, RowCount + (RowCount - 1) / 2);
			glUniform2f(m_uniMargin, 0.2f, 0.4f);
			glUniform2f(m_uniGlobalMargin, 0.05f, 0.1f);
			if (x < ColumnCount*0.9)
			{
				if (channels[y] > x / (float)ColumnCount)
				{
					glUniform3f(m_uniColor, 0.2f, 0.8f, 0.2f);
				}
				else {
					glUniform3f(m_uniColor, 0.1f, 0.2f, 0.1f);
				}
			}
			else {
				if (channels[y] > x / (float)ColumnCount)
				{
					glUniform3f(m_uniColor, 0.8f, 0.2f, 0.2f);
				}
				else {
					glUniform3f(m_uniColor, 0.2f, 0.1f, 0.1f);
				}
			}
			glUniform2f(m_uniPosition, x, y + y / 2);

			if (y%16<8)
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_NOTEQUAL);
				glEnable(GL_BLEND);
				glDisable(GL_SCISSOR_TEST);
				glScissor(0, 0, 0, 0);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBindBuffer(GL_ARRAY_BUFFER, m_vbuf_sqr);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibuf_sqr);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
			}
			else if (y % 16<12) {
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);
				glEnable(GL_BLEND);
				glEnable(GL_SCISSOR_TEST);
				glScissor(0, 0, GetSetting().m_viewport_width, GetSetting().m_viewport_height);
				glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
				glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR);
				glBindBuffer(GL_ARRAY_BUFFER, m_vbuf_tri);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibuf_tri);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
				glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
			}
			else {
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LEQUAL);
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE,GL_ZERO);
				glDisable(GL_SCISSOR_TEST);
				glScissor(0, 0, 0, 0);
				glUniform2f(m_uniMargin, 0.2f, 0.7f);
				glBindBuffer(GL_ARRAY_BUFFER, m_vbuf_dot);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibuf_dot);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, 0);
			}
		}
	}
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void DriverOverheadTest2::RenderGlow(int RowCount, int ColumnCount, float* channels)
{
	glUniformMatrix2fv(m_uniTransMat, 1, 0, m_transformmat);
	m_normal_tex->bind(0);
	for (int x = 0; x < ColumnCount; ++x)
	{
		for (int y = 0; y < RowCount; ++y)
		{
			if (channels[y] < x / (float)ColumnCount) continue;
			glUniform2f(m_uniGridSize, ColumnCount, RowCount + (RowCount - 1) / 2);
			glUniform2f(m_uniMargin, 0.0f, 0.0f);
			glUniform2f(m_uniGlobalMargin, 0.05f, 0.1f);
			if (x < ColumnCount*0.9)
			{
				glUniform3f(m_uniColor, 0.2f, 0.8f, 0.2f);
			}
			else {
				glUniform3f(m_uniColor, 0.8f, 0.2f, 0.2f);
			}
			glUniform2f(m_uniPosition, x, y + y / 2);

			glBindBuffer(GL_ARRAY_BUFFER, m_vbuf_sqr);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibuf_sqr);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool DriverOverheadTest2::render()
{
	glBindVertexArray(m_global_vao);
	const int ColumnCount = 200;
	const int RowCount = 32;

	float time = GetAnimationTime() / 1000.0f;

	float beat = fabs(0.5 - (time * 2 - floor(time * 2)));

	float channels[RowCount];

	channels[0] = channels[31] = (0.95 - frac(time * 2)*0.1 - frac(time * 4)*0.1 )*0.9;
	channels[1] = channels[30] = 0.95 - frac(time * 2)*0.1 - frac(time * 4)*0.1;

	channels[2] = channels[29] = 0.86 + frac(time * 46)*0.1;
	channels[3] = channels[28] = 0.85 + frac(time * 50)*0.1;

	channels[4] = channels[27] = 0.9 - frac(time*3)*0.2;
	channels[5] = channels[26] = 0.9 - frac(time * 4)*0.2;

	channels[6] = channels[25] = 0.9 + 0.05*sin(time*6) - frac(time * 20)*0.1;
	channels[7] = channels[24] = 0.9 + 0.05*sin(time*6-1) - frac(time * 21)*0.1;

	channels[8] = channels[23] = 0.5 - frac(time * 4)*0.1;
	channels[9] = channels[22] = 0.5 - frac(time * 4-0.5)*0.1;

	channels[10] = channels[21] = 0.85 - frac(time * 6-0.4)*0.1;
	channels[11] = channels[20] = 0.95 - frac(time * 6)*0.1;

	channels[12] = channels[19] = 0.55 - frac(time * 7-0.4)*0.1;
	channels[13] = channels[18] = 0.65 - frac(time * 7)*0.1;

	channels[14] = channels[17] = 0.25 - frac(time * 9)*0.1;
	channels[15] = channels[16] = 0.15 - frac(time * 9-0.4)*0.1;

	GLB::OpenGLStateManager::DisableAllCapabilites();
	GLB::OpenGLStateManager::DisableAllVertexAttribs();
	GLB::OpenGLStateManager::Commit();

	glUseProgram(m_shader);

	glEnableVertexAttribArray(0);

	// rendering glow to fbo
	GLB::FBO::bind(m_glowfbo);
	glViewport(0, 0, m_glowfbo->getWidth(), m_glowfbo->getHeight());
	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RenderGlow(RowCount, ColumnCount, channels);

	// rendering main
	GLB::FBO::bind(0);
	glViewport(0, 0, GetSetting().m_viewport_width, GetSetting().m_viewport_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RenderChannels(RowCount, ColumnCount, channels, beat / 5);

	// add glow
	glEnable(GL_BLEND);
	glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
	glBlendColor(0.0f, 0.0f, 0.0f, 0.5f);
	glUseProgram(m_renderglowshader);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbuf_sqr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibuf_sqr);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);	
	glBindTexture(GL_TEXTURE_2D, m_glowfbo->getTextureName());
	glBindSampler(0, m_sampler);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);


	glBindVertexArray(0);
	glBindSampler(0, 0);

	return true;
}


void DriverOverheadTest2::FreeResources()
{
	GLB::GLBShader2::DeleteShaders();

	glDeleteVertexArrays(1, &m_global_vao);
	m_global_vao = 0;
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_vbuf_sqr);
	m_vbuf_sqr = 0;
	glDeleteBuffers(1, &m_ibuf_sqr);
	m_ibuf_sqr = 0;
	glDeleteBuffers(1, &m_vbuf_tri);
	m_vbuf_tri = 0;
	glDeleteBuffers(1, &m_ibuf_tri);
	m_ibuf_tri = 0;
	glDeleteBuffers(1, &m_vbuf_dot);
	m_vbuf_dot = 0;
	glDeleteBuffers(1, &m_ibuf_dot);
	m_ibuf_dot = 0;
	delete m_glowfbo;
	m_glowfbo = 0;
	delete m_normal_tex;
	delete m_burnt_tex;
	delete m_burnt2_tex;

	glDeleteSamplers(1, &m_sampler);
	m_sampler = 0;
}

