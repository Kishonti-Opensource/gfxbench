/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dxb_mesh.h"

#include "platform.h"
#include "d3d11/vbopool.h"
#include "misc2.h"
#include "d3d11/shader.h"

using namespace DXB;

#ifndef EPSILON
#define EPSILON 0.0001f
#endif


void Mesh3::InitVertexAttribs()
{
#ifdef USE_VBO
	InitVertexAttribsVBO_ArrayOfStructures();
	//InitVertexAttribsVBO_PositionStreamFirst_ArrayOfStructures();



	DeleteVertexAttribs();
#else
	for( uint32 i=0; i<m_submeshes.size(); i++)
	{
		Mesh *sm = m_submeshes[i];

		CalculateTangents( sm);

		m_vertex_attribs[0].m_size = 3;
		//m_vertex_attribs[0].m_type = GL_FLOAT;
		m_vertex_attribs[0].m_stride = 0;
		m_vertex_attribs[0].m_data = m_vertex_attribs3[0][0].v;

		if( m_part)
		{
			m_vertex_attribs[1].m_size = 4;
			//m_vertex_attribs[1].m_type = GL_FLOAT;
			m_vertex_attribs[1].m_stride = 0;
			m_vertex_attribs[1].m_data = m_vertex_attribs4[0][0].v;

			m_vertex_attribs[2].m_size = 4;
			//m_vertex_attribs[2].m_type = GL_UNSIGNED_BYTE;
			m_vertex_attribs[2].m_stride = 0;
			m_vertex_attribs[2].m_data = &m_part->m_matrixIndices[0];
		}


		m_vertex_attribs[3].m_size = 3;
		//m_vertex_attribs[3].m_type = GL_FLOAT;
		m_vertex_attribs[3].m_stride = 0;
		m_vertex_attribs[3].m_data = m_vertex_attribs3[1][0].v;

		m_vertex_attribs[4].m_size = 3;
		//m_vertex_attribs[4].m_type = GL_FLOAT;
		m_vertex_attribs[4].m_stride = 0;
		m_vertex_attribs[4].m_data = m_vertex_attribs3[2][0].v;

		m_vertex_attribs[5].m_size = 3;
		//m_vertex_attribs[5].m_type = GL_FLOAT;
		m_vertex_attribs[5].m_stride = 0;
		m_vertex_attribs[5].m_data = m_vertex_attribs3[3][0].v;

		m_vertex_attribs[6].m_size = 2;
		//m_vertex_attribs[6].m_type = GL_FLOAT;
		m_vertex_attribs[6].m_stride = 0;
		m_vertex_attribs[6].m_data = m_vertex_attribs2[0][0].v;

		m_vertex_attribs[7].m_size = 2;
		//m_vertex_attribs[7].m_type = GL_FLOAT;
		m_vertex_attribs[7].m_stride = 0;
		m_vertex_attribs[7].m_data = m_vertex_attribs2[1][0].v;

		m_vertex_attribs[8].m_size = 2;
		//m_vertex_attribs[8].m_type = GL_FLOAT;
		m_vertex_attribs[8].m_stride = 0;
		m_vertex_attribs[8].m_data = m_vertex_attribs2[2][0].v;

		//NOTE: bumpmap texcoord = colormap texcoord
		m_vertex_attribs[9].m_size = 2;
		//m_vertex_attribs[9].m_type = GL_FLOAT;
		m_vertex_attribs[9].m_stride = 0;
		m_vertex_attribs[9].m_data = m_vertex_attribs2[0][0].v;

	}
#endif
}

void Mesh3::InitVertexAttribsVBO_ArrayOfStructures()
{
	if( m_vbo)
	{
		return;
	}

	CalculateTangents();

	size_t vtxStride = 0;

	for(int i=0; i<2; ++i)
	{
		if(m_vertex_attribs2[i].size())
		{
			vtxStride += 2 * sizeof(float);
		}
	}

	if(m_vertex_attribs3[0].size())
	{
		vtxStride += 3 * sizeof(float);
	}

	if(m_vertex_attribs3[1].size()) //normal
	{
		vtxStride += 4 * sizeof(KCL::uint8);
	}
	if(m_vertex_attribs3[2].size()) //tangent
	{
		vtxStride += 4 * sizeof(KCL::uint8);
	}
	if(m_vertex_attribs3[3].size())
	{
		vtxStride += 3 * sizeof(float);
	}


	if(m_vertex_attribs4[0].size())
	{
		vtxStride += 4 * sizeof(float);
	}

	size_t sz_attrib2_0 = m_vertex_attribs2[0].size() * 2 * sizeof(float); //texcoord0
	size_t sz_attrib2_1 = m_vertex_attribs2[1].size() * 2 * sizeof(float); //texcoord1
	size_t sz_attrib3_0 = m_vertex_attribs3[0].size() * 3 * sizeof(float); //position

	size_t sz_attrib3_1 = m_vertex_attribs3[1].size() * 4 * sizeof(KCL::uint8); //normal
	size_t sz_attrib3_2 = m_vertex_attribs3[2].size() * 4 * sizeof(KCL::uint8); //tangent

	size_t sz_attrib3_3 = m_vertex_attribs3[3].size() * 3 * sizeof(float); //color	
	size_t sz_attrib4_0 = m_vertex_attribs4[0].size() * 4 * sizeof(float); //bone_weight

	size_t sz =
		sz_attrib2_0 +
		sz_attrib2_1 +
		sz_attrib3_0 +
		sz_attrib3_1 +
		sz_attrib3_2 +
		sz_attrib3_3 +
		sz_attrib4_0;

	sz_attrib2_0 = m_vertex_attribs2[0].size() ? 2 * sizeof(float) : 0;
	sz_attrib2_1 = m_vertex_attribs2[1].size() ? 2 * sizeof(float) : 0;
	sz_attrib3_0 = m_vertex_attribs3[0].size() ? 3 * sizeof(float) : 0;

	sz_attrib3_1 = m_vertex_attribs3[1].size() ? 4 * sizeof(KCL::uint8) : 0;
	sz_attrib3_2 = m_vertex_attribs3[2].size() ? 4 * sizeof(KCL::uint8) : 0;

	sz_attrib3_3 = m_vertex_attribs3[3].size() ? 3 * sizeof(float) : 0;
	sz_attrib4_0 = m_vertex_attribs4[0].size() ? 4 * sizeof(float) : 0;

	const size_t offset_attrib2_0 = 0;                               // texcoord0
	const size_t offset_attrib2_1 = offset_attrib2_0 + sz_attrib2_0; // texcoord1
	const size_t offset_attrib3_0 = offset_attrib2_1 + sz_attrib2_1; // position
	const size_t offset_attrib3_1 = offset_attrib3_0 + sz_attrib3_0; // normal
	const size_t offset_attrib3_2 = offset_attrib3_1 + sz_attrib3_1; // tangent
	const size_t offset_attrib3_3 = offset_attrib3_2 + sz_attrib3_2; // color
	const size_t offset_attrib4_0 = offset_attrib3_3 + sz_attrib3_3; // bone_weight
	const size_t offset_partition = offset_attrib4_0 + sz_attrib4_0; // matrix indices


	if( m_nodes.size())
	{
		sz += m_vertex_matrix_indices.size() * sizeof(KCL::uint8);
		vtxStride += 4 * sizeof(KCL::uint8);
	}

	//size_t offset = 0;

	m_vertex_attribs[0].m_size = 3;
	//m_vertex_attribs[0].m_type = GL_FLOAT;
	m_vertex_attribs[0].m_stride = vtxStride;
	m_vertex_attribs[0].m_data = (const void*)offset_attrib3_0; //m_vertex_attribs3[0][0].v;

	if( m_nodes.size())
	{
		m_vertex_attribs[1].m_size = 4;
		//m_vertex_attribs[1].m_type = GL_FLOAT;
		m_vertex_attribs[1].m_stride = vtxStride;
		m_vertex_attribs[1].m_data = (const void*)offset_attrib4_0; //m_vertex_attribs4[0][0].v;

		m_vertex_attribs[2].m_size = 4;
		//m_vertex_attribs[2].m_type = GL_UNSIGNED_INT;
		m_vertex_attribs[2].m_stride = vtxStride;
		//m_vertex_attribs[2].m_data = (const void*)(offset_partition + offset); //&m_part->m_matrixIndices[0];
		m_vertex_attribs[2].m_data = (const void*)(offset_partition ); //&m_part->m_matrixIndices[0];

		//offset += 4 * sizeof(uint8);
	}


	//NORMAL
	m_vertex_attribs[3].m_size = 4;
	//m_vertex_attribs[3].m_type = GL_UNSIGNED_BYTE;
	m_vertex_attribs[3].m_stride = vtxStride;
	m_vertex_attribs[3].m_normalized = true;
	m_vertex_attribs[3].m_data = (const void*)offset_attrib3_1; //m_vertex_attribs3[1][0].v;

	//TANGENT
	m_vertex_attribs[4].m_size = 4;
	//m_vertex_attribs[4].m_type = GL_UNSIGNED_BYTE;
	m_vertex_attribs[4].m_stride = vtxStride;
	m_vertex_attribs[4].m_normalized = true;
	m_vertex_attribs[4].m_data = (const void*)offset_attrib3_2; //m_vertex_attribs3[2][0].v;

	m_vertex_attribs[5].m_size = 3;
	//m_vertex_attribs[5].m_type = GL_FLOAT;
	m_vertex_attribs[5].m_stride = vtxStride;
	m_vertex_attribs[5].m_data = (const void*)offset_attrib3_3; //m_vertex_attribs3[3][0].v;

	m_vertex_attribs[6].m_size = 2;
	//m_vertex_attribs[6].m_type = GL_FLOAT;
	m_vertex_attribs[6].m_stride = vtxStride;
	m_vertex_attribs[6].m_data = (const void*)offset_attrib2_0; //m_vertex_attribs2[0][0].v;

	m_vertex_attribs[7].m_size = 2;
	//m_vertex_attribs[7].m_type = GL_FLOAT;
	m_vertex_attribs[7].m_stride = vtxStride;
	m_vertex_attribs[7].m_data = (const void*)offset_attrib2_1; //m_vertex_attribs2[1][0].v;

	//NOTE: bumpmap texcoord = colormap texcoord
	m_vertex_attribs[9].m_size = 2;
	//m_vertex_attribs[9].m_type = GL_FLOAT;
	m_vertex_attribs[9].m_stride = vtxStride;
	m_vertex_attribs[9].m_data = (const void*)offset_attrib2_0; //m_vertex_attribs2[0][0].v;



	KCL::int8* data = (KCL::int8*)malloc(sz);
	if (!data) throw std::exception("Out of memory");
	KCL::int8* it = data;


	KCL::int8* itSource = 0;
	for(size_t i=0; i<m_vertex_attribs3[0].size(); ++i)
	{
		for(size_t j=0; j<2; ++j)
		{
			if(m_vertex_attribs2[j].size())
			{
				itSource = (KCL::int8*)(&m_vertex_attribs2[j][0]);
				itSource += i * 2 * sizeof(float);
				memcpy(it, itSource, 2 * sizeof(float));
				it += 2 * sizeof(float);
			}
		}

		if(m_vertex_attribs3[0].size())
		{
			itSource = (KCL::int8*)(&m_vertex_attribs3[0][0]);
			itSource += i * 3 * sizeof(float);
			memcpy(it, itSource, 3 * sizeof(float));
			it += 3 * sizeof(float);
		}

		if(m_vertex_attribs3[1].size()) //normal
		{
			//itSource = (KCL::int8*)(&m_vertex_attribs3[1][0]);
			//itSource += i * 3 * sizeof(float);
			//memcpy(it, itSource, 3 * sizeof(float));
			//it += 3 * sizeof(float);

			KCL::uint8 byteVec[4];
			byteVec[0] = (KCL::uint8)((m_vertex_attribs3[1][i].x + 1.0f) / 2.0f * 255.0f);
			byteVec[1] = (KCL::uint8)((m_vertex_attribs3[1][i].y + 1.0f) / 2.0f * 255.0f);
			byteVec[2] = (KCL::uint8)((m_vertex_attribs3[1][i].z + 1.0f) / 2.0f * 255.0f);
			memcpy(it, byteVec, 4 * sizeof(KCL::uint8));
			it += 4 * sizeof(KCL::uint8);

		}

		if(m_vertex_attribs3[2].size()) //tangent
		{
			//itSource = (KCL::int8*)(&m_vertex_attribs3[2][0]);
			//itSource += i * 3 * sizeof(float);
			//memcpy(it, itSource, 3 * sizeof(float));
			//it += 3 * sizeof(float);

			KCL::uint8 byteVec[4];
			byteVec[0] = (KCL::uint8)((m_vertex_attribs3[2][i].x + 1.0f) / 2.0f * 255.0f);
			byteVec[1] = (KCL::uint8)((m_vertex_attribs3[2][i].y + 1.0f) / 2.0f * 255.0f);
			byteVec[2] = (KCL::uint8)((m_vertex_attribs3[2][i].z + 1.0f) / 2.0f * 255.0f);
			memcpy(it, byteVec, 4 * sizeof(KCL::uint8));
			it += 4 * sizeof(KCL::uint8);
		}

		if(m_vertex_attribs3[3].size())
		{
			itSource = (KCL::int8*)(&m_vertex_attribs3[3][0]);
			itSource += i * 3 * sizeof(float);
			memcpy(it, itSource, 3 * sizeof(float));
			it += 3 * sizeof(float);
		}

		if(m_vertex_attribs4[0].size())
		{
			itSource = (KCL::int8*)(&m_vertex_attribs4[0][0]);
			itSource += i * 4 * sizeof(float);
			memcpy(it, itSource, 4 * sizeof(float));
			it += 4 * sizeof(float);
		}

		if(m_nodes.size())
		{
			itSource = (KCL::int8*)(&m_vertex_matrix_indices[0]);
			itSource += i * 4 * sizeof(KCL::uint8);
			memcpy(it, itSource, 4 * sizeof(KCL::uint8));
			it += 4 * sizeof(KCL::uint8);
		}
	}


	//glGenBuffers(1, &m_vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	//glBufferData(GL_ARRAY_BUFFER, sz, (const void*)data, GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	KCL::uint32 offsetInVboPool;
	VboPool::Instance()->AddData(sz, (const void*)data, m_vbo, offsetInVboPool, vtxStride);

	free(data);

	for(size_t k=0; k<16; ++k)
	{
		size_t offset = (size_t)m_vertex_attribs[k].m_data;
		offset += offsetInVboPool;
		m_vertex_attribs[k].m_data = (const void*)offset;
	}

	KCL::uint32 offsetInIndexBufferPool;
	for (KCL::uint32 i = 0; i<2; i++)
	{
		IndexBufferPool::Instance()->AddData(m_vertex_indices[i].size() * sizeof(KCL::uint16), (const void*)(&m_vertex_indices[i][0]), m_ebo[i].m_buffer, offsetInIndexBufferPool);
		m_ebo[i].m_offset = (const void*)offsetInIndexBufferPool;

		m_index_counts[i] = m_vertex_indices[i].size();
		m_vertex_indices[i].clear();
	}
}

void Mesh3::BindLayout27( Shader *shader)
{
	if (! m_inputLayout27.Get())
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> vertexDesc;

		D3D11_INPUT_ELEMENT_DESC texcoord0  = { "TEX_COORD",   0, DXGI_FORMAT_R32G32_FLOAT,	      0, (size_t)m_vertex_attribs[6].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC texcoord1  = { "TEX_COORD",   1, DXGI_FORMAT_R32G32_FLOAT,	      0, (size_t)m_vertex_attribs[7].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC position   = { "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (size_t)m_vertex_attribs[0].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC normal     = { "NORMAL",      0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, (size_t)m_vertex_attribs[3].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC tangent    = { "TANGENT",     0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, (size_t)m_vertex_attribs[4].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };

		vertexDesc.push_back( texcoord0  );
		vertexDesc.push_back( texcoord1  );
		vertexDesc.push_back( position   );
		vertexDesc.push_back( normal     );
		vertexDesc.push_back( tangent    );

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateInputLayout(
			&vertexDesc[0],
			vertexDesc.size(),
			&shader->m_vs.m_bytes[0],
			shader->m_vs.m_bytes.size(),
			&m_inputLayout27
			)
			);
	}

	DX::getContext()->IASetInputLayout(m_inputLayout27.Get());
}

void Mesh3::BindLayout27_Skinned( Shader *shader)
{
	if (! m_inputLayout27_skinned.Get())
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> vertexDesc;

		D3D11_INPUT_ELEMENT_DESC texcoord0  = { "TEX_COORD",   0, DXGI_FORMAT_R32G32_FLOAT,	      0, (size_t)m_vertex_attribs[6].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC texcoord1  = { "TEX_COORD",   1, DXGI_FORMAT_R32G32_FLOAT,	      0, (size_t)m_vertex_attribs[7].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC position   = { "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (size_t)m_vertex_attribs[0].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC normal     = { "NORMAL",      0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, (size_t)m_vertex_attribs[3].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC tangent    = { "TANGENT",     0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, (size_t)m_vertex_attribs[4].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC boneindex  = { "BONE_INDEX",  0, DXGI_FORMAT_R8G8B8A8_UINT,      0, (size_t)m_vertex_attribs[2].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC boneweight = { "BONE_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (size_t)m_vertex_attribs[1].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };

		vertexDesc.push_back( texcoord0  );
		vertexDesc.push_back( texcoord1  );
		vertexDesc.push_back( position   );
		vertexDesc.push_back( normal     );
		vertexDesc.push_back( tangent    );
		vertexDesc.push_back( boneindex  );
		vertexDesc.push_back( boneweight );

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateInputLayout(
			&vertexDesc[0],
			vertexDesc.size(),
			&shader->m_vs.m_bytes[0],
			shader->m_vs.m_bytes.size(),
			&m_inputLayout27_skinned
			)
			);
	}

	DX::getContext()->IASetInputLayout(m_inputLayout27_skinned.Get());
}

void Mesh3::BindLayout30( Shader* shader)
{
    if (! m_inputLayout30.Get())
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> vertexDesc;

		D3D11_INPUT_ELEMENT_DESC texcoord0  = { "TEX_COORD",   0, DXGI_FORMAT_R32G32_FLOAT,	      0, (size_t)m_vertex_attribs[6].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC position   = { "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (size_t)m_vertex_attribs[0].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC normal     = { "NORMAL",      0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, (size_t)m_vertex_attribs[3].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC tangent    = { "TANGENT",     0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, (size_t)m_vertex_attribs[4].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };

		vertexDesc.push_back( texcoord0  );
		vertexDesc.push_back( position   );
		vertexDesc.push_back( normal     );
		vertexDesc.push_back( tangent    );

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateInputLayout(
			&vertexDesc[0],
			vertexDesc.size(),
			&shader->m_vs.m_bytes[0],
			shader->m_vs.m_bytes.size(),
			&m_inputLayout30
			)
			);
	}

	DX::getContext()->IASetInputLayout(m_inputLayout30.Get());
}

void Mesh3::BindLayout30_Skinned( Shader* shader)
{
    if (! m_inputLayout30_skinned.Get())
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> vertexDesc;

		D3D11_INPUT_ELEMENT_DESC texcoord0  = { "TEX_COORD",   0, DXGI_FORMAT_R32G32_FLOAT,	      0, (size_t)m_vertex_attribs[6].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC position   = { "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (size_t)m_vertex_attribs[0].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC normal     = { "NORMAL",      0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, (size_t)m_vertex_attribs[3].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC tangent    = { "TANGENT",     0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, (size_t)m_vertex_attribs[4].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC boneindex  = { "BONE_INDEX",  0, DXGI_FORMAT_R8G8B8A8_UINT,      0, (size_t)m_vertex_attribs[2].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC boneweight = { "BONE_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (size_t)m_vertex_attribs[1].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };

		vertexDesc.push_back( texcoord0  );
		vertexDesc.push_back( position   );
		vertexDesc.push_back( normal     );
		vertexDesc.push_back( tangent    );
		vertexDesc.push_back( boneindex  );
		vertexDesc.push_back( boneweight );

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateInputLayout(
			&vertexDesc[0],
			vertexDesc.size(),
			&shader->m_vs.m_bytes[0],
			shader->m_vs.m_bytes.size(),
			&m_inputLayout30_skinned
			)
			);
	}

	DX::getContext()->IASetInputLayout(m_inputLayout30_skinned.Get());
}

void Mesh3::BindLayout30_Instanced( Shader* shader)
{
    if (! m_inputLayout30_instanced.Get())
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> vertexDesc;

		D3D11_INPUT_ELEMENT_DESC texcoord0  = { "TEX_COORD",   0, DXGI_FORMAT_R32G32_FLOAT,	      0, (size_t)m_vertex_attribs[6].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC position   = { "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (size_t)m_vertex_attribs[0].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC normal     = { "NORMAL",      0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, (size_t)m_vertex_attribs[3].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC tangent    = { "TANGENT",     0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, (size_t)m_vertex_attribs[4].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC boneindex  = { "BONE_INDEX",  0, DXGI_FORMAT_R8G8B8A8_UINT,      0, (size_t)m_vertex_attribs[2].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };
		D3D11_INPUT_ELEMENT_DESC boneweight = { "BONE_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (size_t)m_vertex_attribs[1].m_data , D3D11_INPUT_PER_VERTEX_DATA, 0 };

        D3D11_INPUT_ELEMENT_DESC imv0  = { "INSTANCE_MV",   0, DXGI_FORMAT_R32G32B32A32_FLOAT,          1, 0 , D3D11_INPUT_PER_INSTANCE_DATA, 1 };
        D3D11_INPUT_ELEMENT_DESC imv1  = { "INSTANCE_MV",   1, DXGI_FORMAT_R32G32B32A32_FLOAT,          1, 16 , D3D11_INPUT_PER_INSTANCE_DATA, 1 };
        D3D11_INPUT_ELEMENT_DESC imv2  = { "INSTANCE_MV",   2, DXGI_FORMAT_R32G32B32A32_FLOAT,          1, 32 , D3D11_INPUT_PER_INSTANCE_DATA, 1 };
        D3D11_INPUT_ELEMENT_DESC imv3  = { "INSTANCE_MV",   3, DXGI_FORMAT_R32G32B32A32_FLOAT,          1, 48 , D3D11_INPUT_PER_INSTANCE_DATA, 1 };

        D3D11_INPUT_ELEMENT_DESC i_imv0  = { "INSTANCE_INV_MV",   0, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, 64 , D3D11_INPUT_PER_INSTANCE_DATA, 1 };
        D3D11_INPUT_ELEMENT_DESC i_imv1  = { "INSTANCE_INV_MV",   1, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, 80 , D3D11_INPUT_PER_INSTANCE_DATA, 1 };
        D3D11_INPUT_ELEMENT_DESC i_imv2  = { "INSTANCE_INV_MV",   2, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, 96 , D3D11_INPUT_PER_INSTANCE_DATA, 1 };
        D3D11_INPUT_ELEMENT_DESC i_imv3  = { "INSTANCE_INV_MV",   3, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, 112 , D3D11_INPUT_PER_INSTANCE_DATA, 1 };

		vertexDesc.push_back( texcoord0  );
		vertexDesc.push_back( position   );
		vertexDesc.push_back( normal     );
		vertexDesc.push_back( tangent    );
		vertexDesc.push_back( boneindex  );
		vertexDesc.push_back( boneweight );

        vertexDesc.push_back( imv0 );    //instance mv
        vertexDesc.push_back( imv1 );
        vertexDesc.push_back( imv2 );
        vertexDesc.push_back( imv3 );

        vertexDesc.push_back( i_imv0 );    //instance inv mv
        vertexDesc.push_back( i_imv1 );
        vertexDesc.push_back( i_imv2 );
        vertexDesc.push_back( i_imv3 );


		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateInputLayout(
			&vertexDesc[0],
			vertexDesc.size(),
			&shader->m_vs.m_bytes[0],
			shader->m_vs.m_bytes.size(),
			&m_inputLayout30_instanced
			)
			);
	}

	DX::getContext()->IASetInputLayout(m_inputLayout30_instanced.Get());
}

