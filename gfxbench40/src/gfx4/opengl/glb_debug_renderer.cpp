/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_debug_renderer.h"
#include "opengl/glb_opengl_state_manager.h"
#include "platform.h"
#include <kcl_camera2.h>
#include <kcl_mesh.h>

using namespace GLB;

DebugRenderer::DebugRenderer()
{
    KCL::KCL_Status error;
    GLBShaderBuilder sb;
    m_shader = sb.ShaderFile("debug.shader").Build(error);    
    m_pos_attrib = glGetAttribLocation(m_shader->m_p, "in_position");
    m_color_attrib = glGetAttribLocation(m_shader->m_p, "in_color");

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vao);

    glEnableVertexAttribArray(m_pos_attrib);
    glEnableVertexAttribArray(m_color_attrib);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

DebugRenderer::~DebugRenderer()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
}

void DebugRenderer::SetTransform(const KCL::Matrix4x4 &matrix)
{
    m_transform = matrix;
}

void DebugRenderer::ClearTransform()
{
    m_transform.identity();
}

void DebugRenderer::DrawLine(const KCL::Vector3D &v1, const KCL::Vector3D &v2, const KCL::Vector4D &color)
{
    AddVertex(v1, color);
    AddVertex(v2, color);
}

void DebugRenderer::DrawTriangle(const KCL::Vector3D &v1, const KCL::Vector3D &v2, const KCL::Vector3D &v3, const KCL::Vector4D &color)
{
    AddVertex(v1, color);
    AddVertex(v2, color);

    AddVertex(v2, color);
    AddVertex(v3, color);

    AddVertex(v3, color);
    AddVertex(v1, color);
}

void DebugRenderer::DrawQuad(const KCL::Vector3D &v1, const KCL::Vector3D &v2, const KCL::Vector3D &v3, const KCL::Vector3D &v4, const KCL::Vector4D &color)
{
    AddVertex(v1, color);
    AddVertex(v2, color);

    AddVertex(v2, color);
    AddVertex(v3, color);

    AddVertex(v3, color);
    AddVertex(v4, color);

    AddVertex(v4, color);
    AddVertex(v1, color);
}

void DebugRenderer::DrawFrusum(const KCL::Vector3D v[8], const KCL::Vector4D &color)
{
    DrawQuad(v[0], v[1], v[2], v[3], color);
    DrawQuad(v[4], v[5], v[6], v[7], color);              
    DrawLine(v[0], v[4], color);
    DrawLine(v[1], v[5], color);
    DrawLine(v[2], v[6], color);
    DrawLine(v[3], v[7], color);           
}

void DebugRenderer::DrawFrusum(const KCL::Camera2 *camera, const KCL::Vector4D &color, const KCL::Matrix4x4 &model_matrix)
{
    KCL::Matrix4x4 matrix;
    KCL::Matrix4x4::Invert4x4(camera->GetViewProjection(), matrix);

    matrix = matrix * model_matrix;

    KCL::Vector3D points[8];
    KCL::Vector3D projected_frustum_points[8] =
	{
		KCL::Vector3D(-1.0f,-1.0f,-1.0f), KCL::Vector3D(-1.0f,1.0f,-1.0f), KCL::Vector3D(1.0f,1.0f,-1.0f), KCL::Vector3D(1.0f,-1.0f,-1.0f),
		KCL::Vector3D(-1.0f,-1.0f, 1.0f), KCL::Vector3D(-1.0f,1.0f, 1.0f), KCL::Vector3D(1.0f,1.0f, 1.0f), KCL::Vector3D(1.0f,-1.0f, 1.0f),
	};
	
	// Frustum in view space
	for(int i = 0; i < 8; i++)
	{            
        points[i] = KCL::Vector3D(matrix * KCL::Vector4D(projected_frustum_points[i]));
	}
    DrawFrusum(points, color);
}

void DebugRenderer::DrawAABB(const KCL::Vector3D &min, const KCL::Vector3D &max, const KCL::Vector4D &color)
{
    KCL::Vector3D v[8];
    v[0] = KCL::Vector3D( min.x, min.y, min.z);
	v[1] = KCL::Vector3D( max.x, min.y, min.z);
	v[2] = KCL::Vector3D( max.x, max.y, min.z);
	v[3] = KCL::Vector3D( min.x, max.y, min.z);

	v[4] = KCL::Vector3D( min.x, min.y, max.z);
	v[5] = KCL::Vector3D( max.x, min.y, max.z);
	v[6] = KCL::Vector3D( max.x, max.y, max.z);
	v[7] = KCL::Vector3D( min.x, max.y, max.z);
    DrawFrusum(v, color);
}

void DebugRenderer::DrawSphere(const KCL::Vector3D &center, float radius, const KCL::Vector4D &color)
{
    std::vector<KCL::Vector3D> sphere_vertices;
	std::vector<KCL::Vector2D> sphere_tcs;
	std::vector<KCL::uint16> sphere_indices;

	KCL::Mesh3::CreateSphere(sphere_vertices, sphere_tcs, sphere_indices, 10, 10);

    for (KCL::uint32 i = 0; i < sphere_indices.size(); i = i + 3)
    {
        DrawTriangle(center + sphere_vertices[sphere_indices[i]] * radius, center + sphere_vertices[sphere_indices[i + 1]] * radius, center + sphere_vertices[sphere_indices[i]] * radius, color);
    }
}

void DebugRenderer::DrawCone(const KCL::Vector3D &center, float radius, const KCL::Vector4D &color)
{
    std::vector<KCL::Vector3D> cone_vertices;
    std::vector<KCL::Vector2D> cone_tcs;
    std::vector<KCL::uint16> cone_indices;

    KCL::Mesh3::CreateCone(cone_vertices, cone_tcs, cone_indices, 32);
    auto offset = KCL::Vector3D(0.0f, 0.0f, radius);

    for (KCL::uint32 i = 0; i < cone_indices.size(); i = i + 3)
    {
        DrawTriangle(
            center + cone_vertices[cone_indices[i]] * radius - offset,
            center + cone_vertices[cone_indices[i + 1]] * radius - offset,
            center + cone_vertices[cone_indices[i + 2]] * radius - offset,
            color);
    }
}

void DebugRenderer::AddVertex(const KCL::Vector3D &pos, const KCL::Vector4D &color)
{
    m_vertices.push_back(DebugVertex(KCL::Vector3D(m_transform * KCL::Vector4D(pos, 1.0f)), color));
}

void DebugRenderer::Render(const KCL::Camera2 *camera)
{
    Render(camera->GetViewProjection());
}

void DebugRenderer::Render(const KCL::Matrix4x4 &viewProjectionMatrix)
{
    if (m_vertices.empty())
    {
        return;
    }

    OpenGLStateManager::GlEnable(GL_BLEND);
    OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
    OpenGLStateManager::GlDepthFunc(GL_LESS);	//so lines can be depth tested as long as we have a depth-map to read (prior to DoF)
    OpenGLStateManager::GlDisable(GL_CULL_FACE);

    OpenGLStateManager::GlUseProgram(m_shader->m_p);
    glUniformMatrix4fv(m_shader->m_uniform_locations[GLB::uniforms::vp], 1, GL_FALSE, viewProjectionMatrix.v);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(DebugVertex), &m_vertices[0], GL_DYNAMIC_DRAW);

    glVertexAttribPointer(m_pos_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), 0);
    glVertexAttribPointer(m_color_attrib, 4, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (GLvoid*)sizeof(KCL::Vector3D));

    OpenGLStateManager::Commit();
    glDrawArrays(GL_LINES, 0, m_vertices.size());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    OpenGLStateManager::GlDisable(GL_BLEND);
    OpenGLStateManager::GlUseProgram(0);

    m_vertices.clear();
}