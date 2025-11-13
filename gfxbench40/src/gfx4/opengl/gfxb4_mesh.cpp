/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb4_mesh.h"
#include "opengl/vbopool.h"
#include "opengl/glb_shader2.h"
#include "opengl/ext.h"


#define ATTR_COUNT 14 

#define ATTR_POSITION_LOCATION 0

#define ATTR_NORMAL_LOCATION 1
#define ATTR_TANGENT_LOCATION 2

#define ATTR_TEXCOORD0_LOCATION 3
#define ATTR_TEXCOORD1_LOCATION 4


void GFXB4::Mesh3::InitVAO4()
{
	glGenVertexArrays(1, &m_vao_4);

	glBindVertexArray(m_vao_4);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	int attribs[ATTR_COUNT];

	for (int i = 0; i < ATTR_COUNT; i++)
	{
		attribs[i] = -1;
	}


	attribs[GLB::attribs::in_position] = ATTR_POSITION_LOCATION;
	attribs[GLB::attribs::in_normal] = ATTR_NORMAL_LOCATION;
	attribs[GLB::attribs::in_tangent] = ATTR_TANGENT_LOCATION;
	attribs[GLB::attribs::in_texcoord0] = ATTR_TEXCOORD0_LOCATION;
	attribs[GLB::attribs::in_texcoord1] = ATTR_TEXCOORD1_LOCATION;


	for (unsigned int l = 0; l<ATTR_COUNT; l++)
	{
		if (m_vertex_attribs[l].m_size)
		{
			glEnableVertexAttribArray(attribs[l]);

			glVertexAttribPointer(
				attribs[l],
				m_vertex_attribs[l].m_size,
				m_vertex_attribs[l].m_type,
				m_vertex_attribs[l].m_normalized,
				m_vertex_attribs[l].m_stride,
				m_vertex_attribs[l].m_data
				);
		}
	}


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo[0].m_buffer);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	if (!m_instanceVBO)
	{
		glGenBuffers(1, &m_instanceVBO);
	}
}


void GFXB4::Mesh3::InitVertexAttribs(bool reinit)
{
	if (!reinit && m_vbo)
	{
		return;
	}

	CalculateTangents();

	size_t vtxStride = 0;

	bool is_es = GLB::g_extension->isES();
	size_t sz_float = sizeof(float);
	size_t sz_byte;
	KCL::uint32 byte_type;

	if (!is_es)
	{
		sz_byte = sizeof(KCL::uint8);
		byte_type = GL_UNSIGNED_BYTE;
	}
	else
	{
		sz_byte = sizeof(KCL::int8);
		byte_type = GL_BYTE;
	}

	for (int i = 0; i<2; ++i) //texcoords
	{
		if (m_vertex_attribs2[i].size())
		{
			vtxStride += 2 * sz_float;
		}
	}

	if (m_vertex_attribs3[0].size()) //position
	{
		vtxStride += 3 * sz_float;
	}

	if (m_vertex_attribs3[1].size()) //normal
	{
		vtxStride += 4 * sz_byte;
	}

	if (m_vertex_attribs3[2].size()) //tangent
	{
		vtxStride += 4 * sz_byte;
	}

	if (m_vertex_attribs3[3].size()) //color
	{
		vtxStride += 3 * sz_float;
	}

	if (m_vertex_attribs4[0].size()) //weight
	{
		vtxStride += 4 * sz_float;
	}

	if (vtxStride == 12)
	{
		int i = 3;
	}

	size_t sz_attrib2_0 = m_vertex_attribs2[0].size() * 2 * sz_float; //texcoord0
	size_t sz_attrib2_1 = m_vertex_attribs2[1].size() * 2 * sz_float; //texcoord1
	size_t sz_attrib3_0 = m_vertex_attribs3[0].size() * 3 * sz_float; //position
	size_t sz_attrib3_1 = m_vertex_attribs3[1].size() * 4 * sz_byte; //normal
	size_t sz_attrib3_2 = m_vertex_attribs3[2].size() * 4 * sz_byte; //tangent
	size_t sz_attrib3_3 = m_vertex_attribs3[3].size() * 3 * sz_float; //color
	size_t sz_attrib4_0 = m_vertex_attribs4[0].size() * 4 * sz_float; //bone_weight

	size_t sz =
		sz_attrib2_0 +
		sz_attrib2_1 +
		sz_attrib3_0 +
		sz_attrib3_1 +
		sz_attrib3_2 +
		sz_attrib3_3 +
		sz_attrib4_0;

	sz_attrib2_0 = m_vertex_attribs2[0].size() ? 2 * sz_float : 0;
	sz_attrib2_1 = m_vertex_attribs2[1].size() ? 2 * sz_float : 0;
	sz_attrib3_0 = m_vertex_attribs3[0].size() ? 3 * sz_float : 0;
	sz_attrib3_1 = m_vertex_attribs3[1].size() ? 4 * sz_byte : 0;
	sz_attrib3_2 = m_vertex_attribs3[2].size() ? 4 * sz_byte : 0;
	sz_attrib3_3 = m_vertex_attribs3[3].size() ? 3 * sz_float : 0;
	sz_attrib4_0 = m_vertex_attribs4[0].size() ? 4 * sz_float : 0;

	const size_t offset_attrib2_0 = 0;                               // texcoord0
	const size_t offset_attrib2_1 = offset_attrib2_0 + sz_attrib2_0; // texcoord1
	const size_t offset_attrib3_0 = offset_attrib2_1 + sz_attrib2_1; // position
	const size_t offset_attrib3_1 = offset_attrib3_0 + sz_attrib3_0; // normal
	const size_t offset_attrib3_2 = offset_attrib3_1 + sz_attrib3_1; // tangent
	const size_t offset_attrib3_3 = offset_attrib3_2 + sz_attrib3_2; // color
	const size_t offset_attrib4_0 = offset_attrib3_3 + sz_attrib3_3; // bone_weight
	const size_t offset_partition = offset_attrib4_0 + sz_attrib4_0; // matrix indices


	if (m_nodes.size())
	{
		sz += m_vertex_matrix_indices.size() * sizeof(KCL::uint8);
		vtxStride += 4 * sizeof(KCL::uint8);
	}

	//position
	m_vertex_attribs[0].m_size = 3;
	m_vertex_attribs[0].m_type = GL_FLOAT;
	m_vertex_attribs[0].m_stride = vtxStride;
	m_vertex_attribs[0].m_data = (const void*)offset_attrib3_0; //m_vertex_attribs3[0][0].v;

	if (m_nodes.size())
	{
		//weight
		m_vertex_attribs[1].m_size = 4;
		m_vertex_attribs[1].m_type = GL_FLOAT;
		m_vertex_attribs[1].m_stride = vtxStride;
		m_vertex_attribs[1].m_data = (const void*)offset_attrib4_0; //m_vertex_attribs4[0][0].v;
		
		//matrix indices
		m_vertex_attribs[2].m_size = 4;
		m_vertex_attribs[2].m_type = GL_UNSIGNED_BYTE;
		m_vertex_attribs[2].m_stride = vtxStride;
		m_vertex_attribs[2].m_data = (const void*)(offset_partition);

		//offset += 4 * sz_byte;
	}


	//normal
	if (m_vertex_attribs3[1].size())
	{
		m_vertex_attribs[3].m_size = 4;
		m_vertex_attribs[3].m_type = byte_type;
		m_vertex_attribs[3].m_stride = vtxStride;
		m_vertex_attribs[3].m_normalized = true;
		m_vertex_attribs[3].m_data = (const void*)offset_attrib3_1; //m_vertex_attribs3[1][0].v;
	}

	//tangent
	if (m_vertex_attribs3[2].size())
	{
		m_vertex_attribs[4].m_size = 4;
		m_vertex_attribs[4].m_type = byte_type;
		m_vertex_attribs[4].m_stride = vtxStride;
		m_vertex_attribs[4].m_normalized = true;
		m_vertex_attribs[4].m_data = (const void*)offset_attrib3_2;
	}

	//color
	if (m_vertex_attribs3[3].size())
	{
		m_vertex_attribs[5].m_size = 3;
		m_vertex_attribs[5].m_type = GL_FLOAT;
		m_vertex_attribs[5].m_stride = vtxStride;
		m_vertex_attribs[5].m_data = (const void*)offset_attrib3_3;
	}

	//texcoord0
	if (m_vertex_attribs2[0].size())
	{
		m_vertex_attribs[6].m_size = 2;
		m_vertex_attribs[6].m_type = GL_FLOAT;
		m_vertex_attribs[6].m_stride = vtxStride;
		m_vertex_attribs[6].m_data = (const void*)offset_attrib2_0;
	}

	//texcoord1
	if (m_vertex_attribs2[1].size())
	{
		m_vertex_attribs[7].m_size = 2;
		m_vertex_attribs[7].m_type = GL_FLOAT;
		m_vertex_attribs[7].m_stride = vtxStride;
		m_vertex_attribs[7].m_data = (const void*)offset_attrib2_1;
	}


	KCL::int8* data = new KCL::int8[sz];
	KCL::int8* it = data;

	KCL::int8* itSource = NULL;
	for (size_t i = 0; i<m_vertex_attribs3[0].size(); ++i)
	{
		//texcoords
		for (size_t j = 0; j<2; ++j)
		{
			if (m_vertex_attribs2[j].size())
			{
				itSource = (KCL::int8*)(&m_vertex_attribs2[j][0]);
				itSource += i * 2 * sz_float;
				memcpy(it, itSource, 2 * sz_float);
				it += 2 * sz_float;
			}
		}

		//position
		if (m_vertex_attribs3[0].size())
		{
			itSource = (KCL::int8*)(&m_vertex_attribs3[0][0]);
			itSource += i * 3 * sz_float;
			memcpy(it, itSource, 3 * sz_float);
			it += 3 * sz_float;
		}

		//normal, byte conversion
		if (m_vertex_attribs3[1].size())
		{
			if (!is_es)
			{
				KCL::uint8 byteVec[4];
				byteVec[0] = (KCL::uint8)(((m_vertex_attribs3[1][i].x + 1.0f) / 2.0f) * 255.0f);
				byteVec[1] = (KCL::uint8)(((m_vertex_attribs3[1][i].y + 1.0f) / 2.0f) * 255.0f);
				byteVec[2] = (KCL::uint8)(((m_vertex_attribs3[1][i].z + 1.0f) / 2.0f) * 255.0f);
				memcpy(it, byteVec, 4 * sz_byte);
				it += 4 * sz_byte;
			}
			else
			{
				KCL::int8 byteVec[4];
				byteVec[0] = (KCL::int8)(m_vertex_attribs3[1][i].x * 127.0f);
				byteVec[1] = (KCL::int8)(m_vertex_attribs3[1][i].y * 127.0f);
				byteVec[2] = (KCL::int8)(m_vertex_attribs3[1][i].z * 127.0f);
				memcpy(it, byteVec, 4 * sz_byte);
				it += 4 * sz_byte;
			}
		}

		//tangent, byte conversion
		if (m_vertex_attribs3[2].size())
		{
			if (!is_es)
			{
				KCL::uint8 byteVec[4];
				byteVec[0] = (KCL::uint8)(((m_vertex_attribs3[2][i].x + 1.0f) / 2.0f) * 255.0f);
				byteVec[1] = (KCL::uint8)(((m_vertex_attribs3[2][i].y + 1.0f) / 2.0f) * 255.0f);
				byteVec[2] = (KCL::uint8)(((m_vertex_attribs3[2][i].z + 1.0f) / 2.0f) * 255.0f);
				memcpy(it, byteVec, 4 * sz_byte);
				it += 4 * sz_byte;
			}
			else
			{
				KCL::int8 byteVec[4];
				byteVec[0] = (KCL::int8)(m_vertex_attribs3[2][i].x * 127.0f);
				byteVec[1] = (KCL::int8)(m_vertex_attribs3[2][i].y * 127.0f);
				byteVec[2] = (KCL::int8)(m_vertex_attribs3[2][i].z * 127.0f);
				memcpy(it, byteVec, 4 * sz_byte);
				it += 4 * sz_byte;
			}
		}

		//color
		if (m_vertex_attribs3[3].size())
		{
			itSource = (KCL::int8*)(&m_vertex_attribs3[3][0]);
			itSource += i * 3 * sz_float;
			memcpy(it, itSource, 3 * sz_float);
			it += 3 * sz_float;
		}

		//weight
		if (m_vertex_attribs4[0].size())
		{
			itSource = (KCL::int8*)(&m_vertex_attribs4[0][0]);
			itSource += i * 4 * sz_float;
			memcpy(it, itSource, 4 * sz_float);
			it += 4 * sz_float;
		}

		if (m_nodes.size())
		{
			itSource = (KCL::int8*)(&m_vertex_matrix_indices[0]);
			itSource += i * 4 * sz_byte;
			memcpy(it, itSource, 4 * sz_byte);
			it += 4 * sz_byte;
		}
	}


	size_t offsetInVboPool;
	if (m_vbo && reinit)
	{
		VboPool::Instance()->SubData(sz, data, m_vbo, 0);
		offsetInVboPool = 0;
	}
	else
	{
		VboPool::Instance()->AddData(sz, (const void*)data, m_vbo, offsetInVboPool);
	}

	delete[] data;

	for (size_t k = 0; k<16; ++k)
	{
		if (m_vertex_attribs[k].m_size)
		{
			size_t offset = (size_t)m_vertex_attribs[k].m_data;
			offset += offsetInVboPool;
			m_vertex_attribs[k].m_data = (const void*)offset;
		}
	}

	size_t offsetInIndexBufferPool;
	for (KCL::uint32 i = 0; i<2; i++)
	{
		if (m_ebo[i].m_buffer && reinit)
		{
			IndexBufferPool::Instance()->SubData(m_vertex_indices[i].size() * sizeof(KCL::uint16), (const void*)(&m_vertex_indices[i][0]), m_ebo[i].m_buffer, 0);
			offsetInIndexBufferPool = 0;
		}
		else
		{
			IndexBufferPool::Instance()->AddData(m_vertex_indices[i].size() * sizeof(KCL::uint16), (const void*)(&m_vertex_indices[i][0]), m_ebo[i].m_buffer, offsetInIndexBufferPool);
		}
		m_ebo[i].m_offset = (const void*)offsetInIndexBufferPool;

		m_index_counts[i] = m_vertex_indices[i].size();
#ifndef HAVE_GUI_FOLDER
		m_vertex_indices[i].clear();
#endif
	}
}


KCL::Mesh3 *GFXB4::Mesh3Factory::Create(const char *name)
{
	return new GFXB4::Mesh3(name);
}
