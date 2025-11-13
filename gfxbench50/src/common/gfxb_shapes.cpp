/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_shapes.h"
#include "gfxb_mesh_shape.h"
#include "ngl.h"

using namespace GFXB;

Shapes::Shapes()
{
	m_fullscreen_vbid = 0;
	m_fullscreen_ibid = 0;
	m_sphere_vbid = 0;
	m_sphere_ibid = 0;
	m_cone_vbid = 0;
	m_cone_ibid = 0;
	m_cube_vbid = 0;
	m_cube_ibid = 0;
	m_line_vbid = 0;
	m_line_ibid = 0;
}


Shapes::~Shapes()
{
}


void Shapes::Init()
{
	//fulscreen quad
	{
		_basic_vertex vertices[4];

		float s = 1.0;
		vertices[0].m_pos = KCL::Vector3D(-s, -s, 0);
		vertices[1].m_pos = KCL::Vector3D(s, -s, 0);
		vertices[2].m_pos = KCL::Vector3D(s, s, 0);
		vertices[3].m_pos = KCL::Vector3D(-s, s, 0);

		vertices[0].m_tc = KCL::Vector2D(0, 0);
		vertices[1].m_tc = KCL::Vector2D(1, 0);
		vertices[2].m_tc = KCL::Vector2D(1, 1);
		vertices[3].m_tc = KCL::Vector2D(0, 1);

//TODO: check the NDC + fullscreen quad mesh
#if 0
		if (nglGetInteger(NGL_DX_NDC) == 0)
		{
			vertices[0].m_tc = KCL::Vector2D(0, 0);
			vertices[1].m_tc = KCL::Vector2D(1, 0);
			vertices[2].m_tc = KCL::Vector2D(1, 1);
			vertices[3].m_tc = KCL::Vector2D(0, 1);
		}
		else
		{
			vertices[0].m_tc = KCL::Vector2D(0, 1);
			vertices[1].m_tc = KCL::Vector2D(1, 1);
			vertices[2].m_tc = KCL::Vector2D(1, 0);
			vertices[3].m_tc = KCL::Vector2D(0, 0);
		}
#endif

		NGL_vertex_descriptor vl;
		NGL_vertex_attrib vla;

		vla.m_semantic = "in_position";
		vla.m_format = NGL_R32_G32_FLOAT;
		vla.m_offset = offsetof(_basic_vertex, m_pos);
		vl.m_attribs.push_back(vla);

		vla.m_semantic = "in_texcoord0_";
		vla.m_format = NGL_R32_G32_FLOAT;
		vla.m_offset = offsetof(_basic_vertex, m_tc);
		vl.m_attribs.push_back(vla);

		vl.m_stride = sizeof(_basic_vertex);

		m_fullscreen_vbid = 0;
		nglGenVertexBuffer(m_fullscreen_vbid, vl, 4, &vertices[0]);

		uint16_t indices[6] =
		{
			0, 1, 2, 0, 2, 3
		};

		m_fullscreen_ibid = 0;
		nglGenIndexBuffer(m_fullscreen_ibid, NGL_R16_UINT, 6, indices);
	}

	//sphere
	{
		std::vector<KCL::Vector3D> sphere_position;
		std::vector<KCL::Vector3D> sphere_normals;
		std::vector<KCL::Vector2D> sphere_tcs;
		std::vector<KCL::uint16> sphere_indices;

		KCL::Mesh3::CreateSphere(sphere_position, sphere_tcs, sphere_indices, 30, 15);

		sphere_normals.resize(sphere_position.size());
		for (uint32_t i = 0; i < sphere_position.size(); ++i)
		{
			KCL::Vector3D normal = sphere_position[i];
			normal.normalize();
			sphere_normals[i] = normal;
		}

		KCL::Mesh3 sphere("");
		sphere.m_vertex_attribs3[0] = sphere_position;
		sphere.m_vertex_attribs3[1] = sphere_normals;
		sphere.m_vertex_attribs2[0] = sphere_tcs;
		sphere.m_vertex_indices[0] = sphere_indices;
		sphere.CalculateTangents();
		std::vector<KCL::Vector3D>& sphere_tangent = sphere.m_vertex_attribs3[2];

		std::vector<_basic_vertex_with_normal_tangent> sphere_vertices;
		sphere_vertices.resize(sphere_position.size());
		for (uint32_t i = 0; i < sphere_position.size(); ++i)
		{
			_basic_vertex_with_normal_tangent &vertex = sphere_vertices[i];
			vertex.m_pos = sphere_position[i];
			vertex.m_tc = sphere_tcs[i];
			vertex.m_n = sphere_normals[i];
			vertex.m_t = sphere_tangent[i];
		}


		NGL_vertex_descriptor vl;
		NGL_vertex_attrib vla;

		vla.m_semantic = "in_position";
		vla.m_format = NGL_R32_G32_B32_FLOAT;
		vla.m_offset = offsetof(_basic_vertex_with_normal_tangent, m_pos);
		vl.m_attribs.push_back(vla);

		vla.m_semantic = "in_texcoord0_";
		vla.m_format = NGL_R32_G32_FLOAT;
		vla.m_offset = offsetof(_basic_vertex_with_normal_tangent, m_tc);
		vl.m_attribs.push_back(vla);

		vla.m_semantic = "in_normal";
		vla.m_format = NGL_R32_G32_B32_FLOAT;
		vla.m_offset = offsetof(_basic_vertex_with_normal_tangent, m_n);
		vl.m_attribs.push_back(vla);

		vla.m_semantic = "in_tangent";
		vla.m_format = NGL_R32_G32_B32_FLOAT;
		vla.m_offset = offsetof(_basic_vertex_with_normal_tangent, m_t);
		vl.m_attribs.push_back(vla);

		vl.m_stride = sizeof(_basic_vertex_with_normal_tangent);

		m_sphere_vbid = 0;
		nglGenVertexBuffer(m_sphere_vbid, vl, static_cast<uint32_t>(sphere_vertices.size()), &sphere_vertices[0]);
		m_sphere_ibid = 0;
		nglGenIndexBuffer(m_sphere_ibid, NGL_R16_UINT, static_cast<uint32_t>(sphere_indices.size()), &sphere_indices[0]);
	}

	//cone
	{
		std::vector<KCL::Vector3D> cone_vertices;
		std::vector<KCL::Vector2D> cone_tcs;
		std::vector<KCL::uint16> cone_indices;

		KCL::Mesh3::CreateCone(cone_vertices, cone_tcs, cone_indices, 15, 1);

		NGL_vertex_descriptor vl;
		NGL_vertex_attrib vla;

		vla.m_semantic = "in_position";
		vla.m_format = NGL_R32_G32_B32_FLOAT;
		vla.m_offset = 0;
		vl.m_attribs.push_back(vla);

		vl.m_stride = sizeof(KCL::Vector3D);

		m_cone_vbid = 0;
		nglGenVertexBuffer(m_cone_vbid, vl, static_cast<uint32_t>(cone_vertices.size()), &cone_vertices[0]);
		m_cone_ibid = 0;
		nglGenIndexBuffer(m_cone_ibid, NGL_R16_UINT, static_cast<uint32_t>(cone_indices.size()), &cone_indices[0]);
	}

	//cylinder
	{
		std::vector<KCL::Vector3D> cylinder_vertices;
		std::vector<KCL::Vector2D> cylinder_tcs;
		std::vector<KCL::uint16> cylinder_indices;

		KCL::Mesh3::CreateCylinder(cylinder_vertices, cylinder_tcs, cylinder_indices, 15, 1);

		NGL_vertex_descriptor vl;
		NGL_vertex_attrib vla;

		vla.m_semantic = "in_position";
		vla.m_format = NGL_R32_G32_B32_FLOAT;
		vla.m_offset = 0;
		vl.m_attribs.push_back(vla);

		vl.m_stride = sizeof(KCL::Vector3D);

		m_cylinder_vbid = 0;
		nglGenVertexBuffer(m_cylinder_vbid, vl, static_cast<uint32_t>(cylinder_vertices.size()), &cylinder_vertices[0]);
		m_cylinder_ibid = 0;
		nglGenIndexBuffer(m_cylinder_ibid, NGL_R16_UINT, static_cast<uint32_t>(cylinder_indices.size()), &cylinder_indices[0]);

	}

	// unit cube
	{
		{
			std::vector<KCL::uint16> indices;

			for (int i = 0; i < 6; i++)
			{
				indices.push_back(i * 4 + 0);
				indices.push_back(i * 4 + 2);
				indices.push_back(i * 4 + 1);
				indices.push_back(i * 4 + 0);
				indices.push_back(i * 4 + 3);
				indices.push_back(i * 4 + 2);
			}

			m_cube_ibid = 0;
			nglGenIndexBuffer(m_cube_ibid, NGL_R16_UINT, (uint32_t)indices.size(), indices.data());
		}

		{
			const float one = 1.0f;

			_basic_vertex vertices[24];
			vertices[0].m_pos.set(one, one, -one);		vertices[0].m_tc.set(1, 1);
			vertices[1].m_pos.set(one, -one, -one);		vertices[1].m_tc.set(0, 1);
			vertices[2].m_pos.set(one, -one, one);		vertices[2].m_tc.set(0, 0);
			vertices[3].m_pos.set(one, one, one);		vertices[3].m_tc.set(1, 0);

			vertices[4].m_pos.set(one, -one, one);		vertices[4].m_tc.set(1, 1);
			vertices[5].m_pos.set(-one, -one, one);		vertices[5].m_tc.set(0, 1);
			vertices[6].m_pos.set(-one, one, one);		vertices[6].m_tc.set(0, 0);
			vertices[7].m_pos.set(one, one, one);		vertices[7].m_tc.set(1, 0);

			vertices[8].m_pos.set(one, -one, -one);		vertices[8].m_tc.set(1, 1);
			vertices[9].m_pos.set(-one, -one, -one);	vertices[9].m_tc.set(0, 1);
			vertices[10].m_pos.set(-one, -one, one);	vertices[10].m_tc.set(0, 0);
			vertices[11].m_pos.set(one, -one, one);		vertices[11].m_tc.set(1, 0);

			vertices[12].m_pos.set(-one, -one, -one);	vertices[12].m_tc.set(1, 1);
			vertices[13].m_pos.set(-one, one, -one);	vertices[13].m_tc.set(0, 1);
			vertices[14].m_pos.set(-one, one, one);		vertices[14].m_tc.set(0, 0);
			vertices[15].m_pos.set(-one, -one, one);	vertices[15].m_tc.set(1, 0);

			vertices[16].m_pos.set(-one, -one, -one);	vertices[16].m_tc.set(0, 0);
			vertices[17].m_pos.set(one, -one, -one);	vertices[17].m_tc.set(1, 0);
			vertices[18].m_pos.set(one, one, -one);		vertices[18].m_tc.set(1, 1);
			vertices[19].m_pos.set(-one, one, -one);	vertices[19].m_tc.set(0, 1);

			vertices[20].m_pos.set(-one, one, -one);	vertices[20].m_tc.set(1, 1);
			vertices[21].m_pos.set(one, one, -one);		vertices[21].m_tc.set(0, 1);
			vertices[22].m_pos.set(one, one, one);		vertices[22].m_tc.set(0, 0);
			vertices[23].m_pos.set(-one, one, one);		vertices[23].m_tc.set(1, 0);

			NGL_vertex_descriptor vl;
			NGL_vertex_attrib vla;

			vla.m_semantic = "in_position";
			vla.m_format = NGL_R32_G32_B32_FLOAT;
			vla.m_offset = offsetof(_basic_vertex, m_pos);
			vl.m_attribs.push_back(vla);

			vla.m_semantic = "in_texcoord0_";
			vla.m_format = NGL_R32_G32_FLOAT;
			vla.m_offset = offsetof(_basic_vertex, m_tc);
			vl.m_attribs.push_back(vla);

			vl.m_stride = sizeof(_basic_vertex);
			m_cube_vbid = 0;
			nglGenVertexBuffer(m_cube_vbid, vl, 24, vertices);
		}
	}

	// line
	{
		{
			KCL::uint16 indices[2] = {0, 1};
			nglGenIndexBuffer(m_line_ibid, NGL_R16_UINT, 2, indices);
		}

		{
			KCL::Vector3D vertices[2] =
			{
				KCL::Vector3D(0.0f, 0.0f, 0.0f),
				KCL::Vector3D(1.0f, 0.0f, 0.0f)
			};

			NGL_vertex_descriptor vl;
			NGL_vertex_attrib vla;

			vla.m_semantic = "in_position";
			vla.m_format = NGL_R32_G32_B32_FLOAT;
			vla.m_offset = 0;
			vl.m_attribs.push_back(vla);
			vl.m_stride = sizeof(KCL::Vector3D);

			nglGenVertexBuffer(m_line_vbid, vl, 2, vertices);
		}
	}
}

const KCL::Vector3D Shapes::m_cube_vertices[8] =
{
	KCL::Vector3D(-1.0f, -1.0f, -1.0f),
	KCL::Vector3D(+1.0f, -1.0f, -1.0f),
	KCL::Vector3D(-1.0f, +1.0f, -1.0f),
	KCL::Vector3D(+1.0f, +1.0f, -1.0f),

	KCL::Vector3D(-1.0f, -1.0f, +1.0f),
	KCL::Vector3D(+1.0f, -1.0f, +1.0f),
	KCL::Vector3D(-1.0f, +1.0f, +1.0f),
	KCL::Vector3D(+1.0f, +1.0f, +1.0f)
};
