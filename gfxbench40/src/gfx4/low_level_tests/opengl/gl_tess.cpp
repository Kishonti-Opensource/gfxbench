/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gl_tess.h"
#include "opengl/glb_shader2.h"
#include "kcl_io.h"
#include "kcl_camera2.h"
#include "platform.h"
#include "opengl/ext.h"
#include "opengl/glb_discard_functions.h"
#include "opengl/glb_opengl_state_manager.h"

#define ITERATIONS 4


TessTest::TessTest(const GlobalTestEnvironment * const gte) : TessBase(gte), m_vbo( 0), m_ebo( 0), m_vao( 0)
{
}


TessTest::~TessTest()
{
	FreeResources();
}


bool TessTest::animate(const int time)
{
	if (WasKeyPressed('R'))
	{
		INFO("Reload...");
		GLB::GLBShader2::InvalidateShaderCache();
		LoadShaders();
	}

	return TessBase::animate(time);
}


KCL::KCL_Status TessTest::init ()
{
	KCL::KCL_Status s = TessBase::init();

	GLB::GLBShader2::InitShaders((KCL::SceneVersion)KCL::SV_TESS, true);
	KCL::KCL_Status error = LoadShaders();
	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error;
	}

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(KCL::Vector3D) * m_vertices.size(), m_vertices[0].v, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(KCL::uint16) * m_indices.size(), &m_indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays( 1, &m_vao);

	glBindVertexArray( m_vao);
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_ebo);

	glEnableVertexAttribArray( 0);
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray( 0);
	glBindBuffer( GL_ARRAY_BUFFER, 0);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);

	glClearColor(0, 0, 0, 1);

	return s;
}


KCL::KCL_Status TessTest::LoadShaders()
{
	GLB::GLBShaderBuilder sb;
	KCL::KCL_Status error;

	// Compile first shader with no geometry shader
	sb.AddDefine("USE_TESSELLATION");
	if( GLB::g_extension->hasExtension( GLB::GLBEXT_primitive_bounding_box))
	{
		sb.AddDefine("USE_PBB_EXT");
	}

	m_shader = sb.ShaderFile( "bezier_tess.shader").Build(error);

	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error;
	}

	GLB::OpenGLStateManager::GlUseProgram( m_shader->m_p);
	GLint iterations_pos = glGetUniformLocation(m_shader->m_p, "itercount");
	glUniform1f(iterations_pos, float(ITERATIONS));
	GLB::OpenGLStateManager::GlUseProgram( 0);

	// Compile foreground shader for geometry shading
	sb.AddDefine("USE_GEOMSHADER");
	sb.AddDefine("USE_TESSELLATION");
	if( GLB::g_extension->hasExtension( GLB::GLBEXT_primitive_bounding_box))
	{
		sb.AddDefine("USE_PBB_EXT");
	}

	m_shader_geom = sb.ShaderFile( "bezier_tess.shader").Build(error);

	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error;
	}

	GLB::OpenGLStateManager::GlUseProgram( m_shader_geom->m_p);
	iterations_pos = glGetUniformLocation(m_shader_geom->m_p, "itercount");
	glUniform1f(iterations_pos, float(ITERATIONS));
	GLB::OpenGLStateManager::GlUseProgram( 0);

	return KCL::KCL_TESTERROR_NOERROR;
}


bool TessTest::render ()
{
	glDepthRangef(0.5f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLB::OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
	GLB::OpenGLStateManager::GlEnable(GL_CULL_FACE);

	glPatchParameteriProc(GL_PATCH_VERTICES, 16);
	GLB::OpenGLStateManager::GlUseProgram( m_shader->m_p);
	GLB::OpenGLStateManager::Commit();

	if (m_shader->m_uniform_locations[GLB::uniforms::time] > -1)
	{
		glUniform1f(m_shader->m_uniform_locations[GLB::uniforms::time], m_time / 1000.0);
	}
	if (m_shader->m_uniform_locations[GLB::uniforms::frustum_planes] > -1)
	{
		glUniform4fv(m_shader->m_uniform_locations[GLB::uniforms::frustum_planes], 6, m_camera->GetCullPlane(0).v);
	}
	if(m_shader->m_uniform_locations[GLB::uniforms::view_pos] > -1)
	{
		glUniform3fv( m_shader->m_uniform_locations[GLB::uniforms::view_pos], 1, m_camera->GetEye().v);
	}

	glBindVertexArray( m_vao);

	//render close mesh first
	m_mesh1.mv = m_mesh1.model * m_camera->GetView();
	m_mesh1.mvp = m_mesh1.model * m_camera->GetViewProjection();

	m_mesh2.mv = m_mesh2.model * m_camera->GetView();
	m_mesh2.mvp = m_mesh2.model * m_camera->GetViewProjection();

	m_mesh3.mv = m_mesh3.model * m_camera->GetView();
	m_mesh3.mvp = m_mesh3.model * m_camera->GetViewProjection();

	const KCL::uint32 vp_count_x = 4;
	const KCL::uint32 vp_count_y = 4;

	const KCL::uint32 vp_size_x = getViewportWidth() / vp_count_x;
	const KCL::uint32 vp_size_y = getViewportHeight() / vp_count_y;

	float tess_vp_scale = sqrtf(float(vp_size_x) / 1920.0 * float(vp_size_y) / 1080.0);
	glUniform4fv( m_shader->m_uniform_locations[GLB::uniforms::cam_near_far_pid_vpscale], 1, KCL::Vector4D(m_camera->GetNear(), m_camera->GetFar(), 0.0, tess_vp_scale).v);	
	glUniform2f(m_shader->m_uniform_locations[GLB::uniforms::view_port_size], float(vp_size_x), float(vp_size_y));

	for (KCL::uint32 x = 0; x < vp_count_x; x++)
	{
		for (KCL::uint32 y = 0; y < vp_count_y; y++)
		{
			glViewport(vp_size_x * x, vp_size_y * y, vp_size_x, vp_size_y);
			RenderMesh(m_mesh1, m_shader);
			RenderMesh(m_mesh2, m_shader);
			RenderMesh(m_mesh3, m_shader);
		}
	}

	// Change to geometry shader for foreground
	GLB::OpenGLStateManager::GlUseProgram( m_shader_geom->m_p);
	GLB::OpenGLStateManager::Commit();

	glDepthRangef(0.0f, 0.5f);
	glViewport(0, 0, getViewportWidth(), getViewportHeight());

	tess_vp_scale = sqrtf(float(getViewportWidth()) / 1920.0 * float(getViewportHeight()) / 1080.0);
	glUniform4fv( m_shader_geom->m_uniform_locations[GLB::uniforms::cam_near_far_pid_vpscale], 1, KCL::Vector4D(m_camera->GetNear(), m_camera->GetFar(), 0.0, tess_vp_scale).v);
	glUniform2f(m_shader_geom->m_uniform_locations[GLB::uniforms::view_port_size], float(getViewportWidth()), float(getViewportHeight()));

	if (m_shader_geom->m_uniform_locations[GLB::uniforms::time] > -1)
	{
		glUniform1f(m_shader_geom->m_uniform_locations[GLB::uniforms::time], m_time / 1000.0);
	}
	if (m_shader_geom->m_uniform_locations[GLB::uniforms::frustum_planes] > -1)
	{
		glUniform4fv(m_shader_geom->m_uniform_locations[GLB::uniforms::frustum_planes], 6, m_camera->GetCullPlane(0).v);
	}
	if(m_shader_geom->m_uniform_locations[GLB::uniforms::view_pos] > -1)
	{
		glUniform3fv( m_shader_geom->m_uniform_locations[GLB::uniforms::view_pos], 1, m_camera->GetEye().v);
	}

	RenderMesh(m_mesh1, m_shader_geom);
	RenderMesh(m_mesh3, m_shader_geom);

	glBindVertexArray( 0);

	return true;
}


void TessTest::RenderMesh(const Mesh &mesh, const GLB::GLBShader2 *shader)
{
	if(shader->m_uniform_locations[GLB::uniforms::mvp] > -1)
	{
		glUniformMatrix4fv( shader->m_uniform_locations[GLB::uniforms::mvp], 1, 0, mesh.mvp.v);
	}
	if(shader->m_uniform_locations[GLB::uniforms::mv] > -1)
	{
		glUniformMatrix4fv( shader->m_uniform_locations[GLB::uniforms::mv], 1, 0, mesh.mv.v);
	}
	if(shader->m_uniform_locations[GLB::uniforms::model] > -1)
	{
		glUniformMatrix4fv( shader->m_uniform_locations[GLB::uniforms::model], 1, 0, mesh.model.v);
	}
	glDrawElements( GL_PATCHES, m_indices.size(), GL_UNSIGNED_SHORT, 0);
}


void TessTest::FreeResources()
{
	GLB::GLBShader2::DeleteShaders();

	if( m_vbo)
	{
		glDeleteBuffers( 1, &m_vbo);
	}
	if( m_ebo)
	{
		glDeleteBuffers( 1, &m_ebo);
	}
	if( m_vao)
	{
		glDeleteVertexArrays( 1, &m_vao);
	}

}
