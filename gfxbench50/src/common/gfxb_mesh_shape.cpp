/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_mesh_shape.h"
#include "ngl.h"

using namespace GFXB;

bool Mesh3::UploadMesh(int flags)
{
	MeshLayout layout;
	layout.has_position = !m_vertex_attribs3[0].empty();
	layout.has_normal = !m_vertex_attribs3[1].empty();
	layout.has_tangent = !m_vertex_attribs3[2].empty();
	layout.has_color = !m_vertex_attribs3[3].empty();
	layout.has_texcoord0 = !m_vertex_attribs2[0].empty();
	layout.has_texcoord1 = !m_vertex_attribs2[1].empty();
	layout.has_texcoord2 = !m_vertex_attribs2[2].empty();
	layout.has_texcoord3 = !m_vertex_attribs2[3].empty();
	layout.has_bones = !m_vertex_attribs4[0].empty();

	// do not upload unused parameters
	layout.has_color = false;
	layout.has_texcoord1 = false;
	layout.has_texcoord2 = false;
	layout.has_texcoord3 = false;

	// Upload the vertex buffer
	if (UploadVertexBuffer(layout, m_vbid) == false)
	{
		INFO("Error: Mesh3::UploadMesh() - Can not commit vertex buffer to NGL: %s", m_name.c_str());
		return false;
	}

	if (flags & FLAG_CREATE_SHADOW_BUFFER)
	{
		// Upload the shadow vertex buffer
		MeshLayout shadow_layout;
		shadow_layout.has_position = !m_vertex_attribs3[0].empty();
		shadow_layout.has_texcoord0 = !m_vertex_attribs2[0].empty() && (flags & FLAG_SHADOW_UV0);
		shadow_layout.has_bones = !m_vertex_attribs4[0].empty();

		if (UploadVertexBuffer(shadow_layout, m_shadow_vbid) == false)
		{
			INFO("Error: Mesh3::UploadMesh() - Can not commit shadow vertex buffer to NGL: %s", m_name.c_str());
			return false;
		}
	}
	else
	{
		m_shadow_vbid = m_vbid;
	}

	// Upload the index buffer
	if (UploadIndexBuffer() == false)
	{
		INFO("Error: Mesh3::UploadMesh() - Can not commit index buffer to : %s", m_name.c_str());
		return false;
	}

	// On Editor we keep the vertex attributes for debugging and raycasting
#ifndef EDITOR
	KCL::FreeContainerData(m_vertex_indices[0]);
	KCL::FreeContainerData(m_vertex_indices[1]);

	for (int i = 0; i < 4; i++)
	{
		KCL::FreeContainerData(m_vertex_attribs2[i]);
		KCL::FreeContainerData(m_vertex_attribs3[i]);
	}

	KCL::FreeContainerData(m_vertex_attribs4[0]);
#endif

	return true;
}

/*
1. Collect the vertex attributes from KCL::Mesh3 to one continuous float array depending on layout
2. Build the input semantics
3. Create the vertex and index buffers
NOTE: Mesh without position is not supported yet
TODO: upload normal, tangent, color as 888
*/
bool Mesh3::UploadVertexBuffer(const MeshLayout &layout, KCL::uint32 &buffer)
{
	if (layout.has_position == false)
	{
		INFO("Error: Mesh3::UploadVertexBuffer() - Mesh without position vertex attributes is not yet supported: %s", m_name.c_str());
		assert(0);
		return false;
	}

	KCL::uint32 vertex_count = (KCL::uint32)m_vertex_attribs3[0].size();
	std::vector<float> vertices;
	vertices.reserve(vertex_count * 11); // Most common case: pos(3), uv0(2), normal(3), tangent(3)

	// Iterate over the vertex attributes and merge them to one continuous float array
	for (KCL::uint32 i = 0; i < vertex_count; i++)
	{
		// Position
		if (layout.has_position)
		{
			vertices.push_back(m_vertex_attribs3[0][i].x);
			vertices.push_back(m_vertex_attribs3[0][i].y);
			vertices.push_back(m_vertex_attribs3[0][i].z);
		}

		// Texcoord0
		if (layout.has_texcoord0)
		{
			vertices.push_back(m_vertex_attribs2[0][i].x);
			vertices.push_back(m_vertex_attribs2[0][i].y);
		}

		// Texcoord1
		if (layout.has_texcoord1)
		{
			vertices.push_back(m_vertex_attribs2[1][i].x);
			vertices.push_back(m_vertex_attribs2[1][i].y);
		}

		// Texcoord2
		if (layout.has_texcoord2)
		{
			vertices.push_back(m_vertex_attribs2[2][i].x);
			vertices.push_back(m_vertex_attribs2[2][i].y);
		}

		// Texcoord3
		if (layout.has_texcoord3)
		{
			vertices.push_back(m_vertex_attribs2[3][i].x);
			vertices.push_back(m_vertex_attribs2[3][i].y);
		}

		// Normal
		if (layout.has_normal)
		{
			vertices.push_back(m_vertex_attribs3[1][i].x);
			vertices.push_back(m_vertex_attribs3[1][i].y);
			vertices.push_back(m_vertex_attribs3[1][i].z);
		}

		// Tangent
		if (layout.has_tangent)
		{
			vertices.push_back(m_vertex_attribs3[2][i].x);
			vertices.push_back(m_vertex_attribs3[2][i].y);
			vertices.push_back(m_vertex_attribs3[2][i].z);
		}

		// Color
		if (layout.has_color)
		{
			vertices.push_back(m_vertex_attribs3[3][i].x);
			vertices.push_back(m_vertex_attribs3[3][i].y);
			vertices.push_back(m_vertex_attribs3[3][i].z);
		}

		// Skeletal meshes
		if (layout.has_bones)
		{
			// Bone index
			vertices.push_back(m_vertex_matrix_indices[i * 4]);
			vertices.push_back(m_vertex_matrix_indices[i * 4 + 1]);
			vertices.push_back(m_vertex_matrix_indices[i * 4 + 2]);
			vertices.push_back(m_vertex_matrix_indices[i * 4 + 3]);

			// Bone weight
			vertices.push_back(m_vertex_attribs4[0][i].x);
			vertices.push_back(m_vertex_attribs4[0][i].y);
			vertices.push_back(m_vertex_attribs4[0][i].z);
			vertices.push_back(m_vertex_attribs4[0][i].w);
		}
	}

	// Build the input semantics
	{
		KCL::uint32 attrib_offset = 0;
		NGL_vertex_descriptor vl;
		NGL_vertex_attrib vla;

		if (layout.has_position)
		{
			vla.m_semantic = "in_position";
			vla.m_format = NGL_R32_G32_B32_FLOAT;
			vla.m_offset = attrib_offset;
			vl.m_attribs.push_back(vla);

			attrib_offset += sizeof(KCL::Vector3D);
		}

		if (layout.has_texcoord0)
		{
			vla.m_semantic = "in_texcoord0_";
			vla.m_format = NGL_R32_G32_FLOAT;
			vla.m_offset = attrib_offset;
			vl.m_attribs.push_back(vla);

			attrib_offset += sizeof(KCL::Vector2D);
		}

		if (layout.has_texcoord1)
		{
			vla.m_semantic = "in_texcoord1_";
			vla.m_format = NGL_R32_G32_FLOAT;
			vla.m_offset = attrib_offset;
			vl.m_attribs.push_back(vla);

			attrib_offset += sizeof(KCL::Vector2D);
		}

		if (layout.has_texcoord2)
		{
			vla.m_semantic = "in_texcoord2_";
			vla.m_format = NGL_R32_G32_FLOAT;
			vla.m_offset = attrib_offset;
			vl.m_attribs.push_back(vla);

			attrib_offset += sizeof(KCL::Vector2D);
		}

		if (layout.has_texcoord3)
		{
			vla.m_semantic = "in_texcoord3_";
			vla.m_format = NGL_R32_G32_FLOAT;
			vla.m_offset = attrib_offset;
			vl.m_attribs.push_back(vla);

			attrib_offset += sizeof(KCL::Vector2D);
		}

		if (layout.has_normal)
		{
			vla.m_semantic = "in_normal";
			vla.m_format = NGL_R32_G32_B32_FLOAT;  // TODO: 888
			vla.m_offset = attrib_offset;
			vl.m_attribs.push_back(vla);

			attrib_offset += sizeof(KCL::Vector3D);
		}

		if (layout.has_tangent)
		{
			vla.m_semantic = "in_tangent";
			vla.m_format = NGL_R32_G32_B32_FLOAT;  // TODO: 888
			vla.m_offset = attrib_offset;
			vl.m_attribs.push_back(vla);

			attrib_offset += sizeof(KCL::Vector3D);
		}

		if (layout.has_color)
		{
			vla.m_semantic = "in_color";
			vla.m_format = NGL_R32_G32_B32_FLOAT; // TODO: 888
			vla.m_offset = attrib_offset;
			vl.m_attribs.push_back(vla);

			attrib_offset += sizeof(KCL::Vector3D);
		}

		if (layout.has_bones)
		{
			vla.m_semantic = "in_bone_index";
			vla.m_format = NGL_R32_G32_B32_A32_FLOAT;
			vla.m_offset = attrib_offset;
			vl.m_attribs.push_back(vla);
			attrib_offset += sizeof(KCL::Vector4D);

			vla.m_semantic = "in_bone_weight";
			vla.m_format = NGL_R32_G32_B32_A32_FLOAT;
			vla.m_offset = attrib_offset;
			vl.m_attribs.push_back(vla);
			attrib_offset += sizeof(KCL::Vector4D);
		}

		vl.m_stride = attrib_offset;

		// Create the vertex buffer
		return nglGenVertexBuffer(buffer, vl, vertex_count, &vertices[0]);
	}
}


bool Mesh3::UploadIndexBuffer()
{
	if (m_vertex_indices[0].empty())
	{
		INFO("Error: Mesh3::UploadIndexBuffer() - Mesh does not have indices: %s", m_name.c_str());
		return false;
	}

	bool result = nglGenIndexBuffer(m_ibid, NGL_R16_UINT, static_cast<uint32_t>(m_vertex_indices[0].size()), &m_vertex_indices[0][0]);
	if (result)
	{
		m_index_counts[0] = (KCL::uint32)m_vertex_indices[0].size();
	}
	return result;
}
