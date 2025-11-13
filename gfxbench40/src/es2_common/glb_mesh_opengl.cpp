/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_mesh.h"
#include "platform.h"

#include "opengl/shader.h"
#include "opengl/vbopool.h"
#include "misc2.h"

#ifdef WIN32
#pragma warning(disable: 4267) // conversion from 'size_t' to 'KCL::uint32', possible loss of data
#endif

using namespace KCL;
using namespace GLB;

#ifndef EPSILON
#define EPSILON 0.0001f
#endif


void GLB::Mesh3::InitVertexAttribs(bool reinit)
{
#ifdef USE_VBO
	InitVertexAttribsVBO_ArrayOfStructures(reinit);

#ifndef HAVE_GUI_FOLDER
	DeleteVertexAttribs();
#endif

	//InitVertexAttribsVBO_PositionStreamFirst_ArrayOfStructures();
#else
	for( KCL::uint32 i=0; i<m_submeshes.size(); i++)
	{
		Mesh *sm = m_submeshes[i];

		CalculateTangents( sm);

		m_vertex_attribs[0].m_size = 3;
		m_vertex_attribs[0].m_type = GL_FLOAT;
		m_vertex_attribs[0].m_stride = 0;
		m_vertex_attribs[0].m_data = m_vertex_attribs3[0][0].v;

		if( m_part)
		{
			m_vertex_attribs[1].m_size = 4;
			m_vertex_attribs[1].m_type = GL_FLOAT;
			m_vertex_attribs[1].m_stride = 0;
			m_vertex_attribs[1].m_data = m_vertex_attribs4[0][0].v;

			m_vertex_attribs[2].m_size = 4;
			m_vertex_attribs[2].m_type = GL_UNSIGNED_BYTE;
			m_vertex_attribs[2].m_stride = 0;
			m_vertex_attribs[2].m_data = &m_part->m_matrixIndices[0];
		}


		m_vertex_attribs[3].m_size = 3;
		m_vertex_attribs[3].m_type = GL_FLOAT;
		m_vertex_attribs[3].m_stride = 0;
		m_vertex_attribs[3].m_data = m_vertex_attribs3[1][0].v;

		m_vertex_attribs[4].m_size = 3;
		m_vertex_attribs[4].m_type = GL_FLOAT;
		m_vertex_attribs[4].m_stride = 0;
		m_vertex_attribs[4].m_data = m_vertex_attribs3[2][0].v;

		m_vertex_attribs[5].m_size = 3;
		m_vertex_attribs[5].m_type = GL_FLOAT;
		m_vertex_attribs[5].m_stride = 0;
		m_vertex_attribs[5].m_data = m_vertex_attribs3[3][0].v;

		m_vertex_attribs[6].m_size = 2;
		m_vertex_attribs[6].m_type = GL_FLOAT;
		m_vertex_attribs[6].m_stride = 0;
		m_vertex_attribs[6].m_data = m_vertex_attribs2[0][0].v;

		m_vertex_attribs[7].m_size = 2;
		m_vertex_attribs[7].m_type = GL_FLOAT;
		m_vertex_attribs[7].m_stride = 0;
		m_vertex_attribs[7].m_data = m_vertex_attribs2[1][0].v;

		m_vertex_attribs[8].m_size = 2;
		m_vertex_attribs[8].m_type = GL_FLOAT;
		m_vertex_attribs[8].m_stride = 0;
		m_vertex_attribs[8].m_data = m_vertex_attribs2[2][0].v;

		//NOTE: bumpmap texcoord = colormap texcoord
		m_vertex_attribs[9].m_size = 2;
		m_vertex_attribs[9].m_type = GL_FLOAT;
		m_vertex_attribs[9].m_stride = 0;
		m_vertex_attribs[9].m_data = m_vertex_attribs2[0][0].v;

	}
#endif
}


void GLB::Mesh3::InitVertexAttribsVBO_ArrayOfStructures(bool reinit)
{
	if( !reinit && m_vbo)
	{
		return;
	}

	CalculateTangents();

	KCL::uint32 vtxStride = 0;

    for (int i = 0; i < 2; ++i)
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
		//vtxStride += 3 * sizeof(float);
#ifdef __glew_h__
		vtxStride += 4 * sizeof(KCL::uint8);
#else
		vtxStride += 4 * sizeof(KCL::int8);
#endif
	}
	if(m_vertex_attribs3[2].size()) //tangent
	{
		//vtxStride += 3 * sizeof(float);
#ifdef __glew_h__
		vtxStride += 4 * sizeof(KCL::uint8);
#else
		vtxStride += 4 * sizeof(KCL::int8);
#endif
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

	//size_t sz_attrib3_1 = m_vertex_attribs3[1].size() * 3 * sizeof(float); //normal
#ifdef __glew_h__
	size_t sz_attrib3_1 = m_vertex_attribs3[1].size() * 4 * sizeof(KCL::uint8); //normal
#else
	size_t sz_attrib3_1 = m_vertex_attribs3[1].size() * 4 * sizeof(KCL::int8); //normal
#endif

	//size_t sz_attrib3_2 = m_vertex_attribs3[2].size() * 3 * sizeof(float); //tangent
#ifdef __glew_h__
	size_t sz_attrib3_2 = m_vertex_attribs3[2].size() * 4 * sizeof(KCL::uint8); //tangent
#else
	size_t sz_attrib3_2 = m_vertex_attribs3[2].size() * 4 * sizeof(KCL::int8); //tangent
#endif

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

	//sz_attrib3_1 = m_vertex_attribs3[1].size() ? 3 * sizeof(float) : 0;
#ifdef __glew_h__
	sz_attrib3_1 = m_vertex_attribs3[1].size() ? 4 * sizeof(KCL::uint8) : 0;
#else
	sz_attrib3_1 = m_vertex_attribs3[1].size() ? 4 * sizeof(KCL::int8) : 0;
#endif

	//sz_attrib3_2 = m_vertex_attribs3[2].size() ? 3 * sizeof(float) : 0;
#ifdef __glew_h__
	sz_attrib3_2 = m_vertex_attribs3[2].size() ? 4 * sizeof(KCL::uint8) : 0;
#else
	sz_attrib3_2 = m_vertex_attribs3[2].size() ? 4 * sizeof(KCL::int8) : 0;
#endif

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
	m_vertex_attribs[0].m_type = GL_FLOAT;
	m_vertex_attribs[0].m_stride = vtxStride;
	m_vertex_attribs[0].m_data = (const void*)offset_attrib3_0; //m_vertex_attribs3[0][0].v;

	if( m_nodes.size())
	{
		m_vertex_attribs[1].m_size = 4;
		m_vertex_attribs[1].m_type = GL_FLOAT;
		m_vertex_attribs[1].m_stride = vtxStride;
		m_vertex_attribs[1].m_data = (const void*)offset_attrib4_0; //m_vertex_attribs4[0][0].v;

		m_vertex_attribs[2].m_size = 4;
		m_vertex_attribs[2].m_type = GL_UNSIGNED_BYTE;
		m_vertex_attribs[2].m_stride = vtxStride;
		//m_vertex_attribs[2].m_data = (const void*)(offset_partition + offset); //&m_part->m_matrixIndices[0];
		m_vertex_attribs[2].m_data = (const void*)(offset_partition ); //&m_part->m_matrixIndices[0];

		//offset += 4 * sizeof(KCL::uint8);
	}


	//NORMAL
	m_vertex_attribs[3].m_size = 3;
	//m_vertex_attribs[3].m_type = GL_FLOAT;
#ifdef __glew_h__
	m_vertex_attribs[3].m_type = GL_UNSIGNED_BYTE;
#else
	m_vertex_attribs[3].m_type = GL_BYTE;
#endif
	m_vertex_attribs[3].m_stride = vtxStride;
	m_vertex_attribs[3].m_normalized = true;
	m_vertex_attribs[3].m_data = (const void*)offset_attrib3_1; //m_vertex_attribs3[1][0].v;

	//TANGENT
	m_vertex_attribs[4].m_size = 3;
	//m_vertex_attribs[4].m_type = GL_FLOAT;
#ifdef __glew_h__
	m_vertex_attribs[4].m_type = GL_UNSIGNED_BYTE;
#else
	m_vertex_attribs[4].m_type = GL_BYTE;
#endif
	m_vertex_attribs[4].m_stride = vtxStride;
	m_vertex_attribs[4].m_normalized = true;
	m_vertex_attribs[4].m_data = (const void*)offset_attrib3_2; //m_vertex_attribs3[2][0].v;

	m_vertex_attribs[5].m_size = 3;
	m_vertex_attribs[5].m_type = GL_FLOAT;
	m_vertex_attribs[5].m_stride = vtxStride;
	m_vertex_attribs[5].m_data = (const void*)offset_attrib3_3; //m_vertex_attribs3[3][0].v;

	m_vertex_attribs[6].m_size = 2;
	m_vertex_attribs[6].m_type = GL_FLOAT;
	m_vertex_attribs[6].m_stride = vtxStride;
	m_vertex_attribs[6].m_data = (const void*)offset_attrib2_0; //m_vertex_attribs2[0][0].v;

	m_vertex_attribs[7].m_size = 2;
	m_vertex_attribs[7].m_type = GL_FLOAT;
	m_vertex_attribs[7].m_stride = vtxStride;
	m_vertex_attribs[7].m_data = (const void*)offset_attrib2_1; //m_vertex_attribs2[1][0].v;

	//NOTE: bumpmap texcoord = colormap texcoord
	m_vertex_attribs[9].m_size = 2;
	m_vertex_attribs[9].m_type = GL_FLOAT;
	m_vertex_attribs[9].m_stride = vtxStride;
	m_vertex_attribs[9].m_data = (const void*)offset_attrib2_0; //m_vertex_attribs2[0][0].v;


	sz += vtxStride;

	KCL::int8* data = new KCL::int8[sz];
	KCL::int8* it = data;

	memset(data, 0, sz);

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

		if(m_vertex_attribs3[1].size()) //normal --> byte
		{
			//itSource = (KCL::int8*)(m_vertex_attribs3[1].m_data);
			//itSource += i * 3 * sizeof(float);
			//memcpy(it, itSource, 3 * sizeof(float));
			//it += 3 * sizeof(float);

#ifdef __glew_h__
			KCL::uint8 byteVec[4];
			byteVec[0] = (KCL::uint8)(( (m_vertex_attribs3[1][i].x + 1.0f) / 2.0f  ) * 255.0f);
			byteVec[1] = (KCL::uint8)(( (m_vertex_attribs3[1][i].y + 1.0f) / 2.0f  ) * 255.0f);
			byteVec[2] = (KCL::uint8)(( (m_vertex_attribs3[1][i].z + 1.0f) / 2.0f  ) * 255.0f);
			memcpy(it, byteVec, 4 * sizeof(KCL::uint8));
			it += 4 * sizeof(KCL::uint8);
#else
			KCL::int8 byteVec[4];
			byteVec[0] = (KCL::int8)(m_vertex_attribs3[1][i].x * 127.0f);
			byteVec[1] = (KCL::int8)(m_vertex_attribs3[1][i].y * 127.0f);
			byteVec[2] = (KCL::int8)(m_vertex_attribs3[1][i].z * 127.0f);
			memcpy(it, byteVec, 4 * sizeof(KCL::int8));
			it += 4 * sizeof(KCL::int8);
#endif
		}

		if(m_vertex_attribs3[2].size()) //tangent --> byte
		{
			//itSource = (KCL::int8*)(m_vertex_attribs3[2].m_data);
			//itSource += i * 3 * sizeof(float);
			//memcpy(it, itSource, 3 * sizeof(float));
			//it += 3 * sizeof(float);

#ifdef __glew_h__
			KCL::uint8 byteVec[4];
			byteVec[0] = (KCL::uint8)(( (m_vertex_attribs3[2][i].x + 1.0f) / 2.0f  ) * 255.0f);
			byteVec[1] = (KCL::uint8)(( (m_vertex_attribs3[2][i].y + 1.0f) / 2.0f  ) * 255.0f);
			byteVec[2] = (KCL::uint8)(( (m_vertex_attribs3[2][i].z + 1.0f) / 2.0f  ) * 255.0f);
			memcpy(it, byteVec, 4 * sizeof(KCL::uint8));
			it += 4 * sizeof(KCL::uint8);
#else
			KCL::int8 byteVec[4];
			byteVec[0] = (KCL::int8)(m_vertex_attribs3[2][i].x * 127.0f);
			byteVec[1] = (KCL::int8)(m_vertex_attribs3[2][i].y * 127.0f);
			byteVec[2] = (KCL::int8)(m_vertex_attribs3[2][i].z * 127.0f);
			memcpy(it, byteVec, 4 * sizeof(KCL::int8));
			it += 4 * sizeof(KCL::int8);
#endif
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

	delete [] data;

	for(size_t k=0; k<16; ++k)
	{
		size_t offset = (size_t)m_vertex_attribs[k].m_data;
		offset += offsetInVboPool;
		m_vertex_attribs[k].m_data = (const void*)offset;
	}

	size_t offsetInIndexBufferPool;
	for( KCL::uint32 i=0; i<2; i++)
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


void GLB::Mesh3::InitVertexAttribsVBO_PositionStreamFirst_ArrayOfStructures()
{
	if( m_vbo)
	{
		return;
	}

	CalculateTangents();

	size_t vboSz = 0; // the full size of the vbo in bytes
	
// 1st stream stride:
	size_t vtxStride_1st_stream = 0; // position, bone-weight, matrix-index
	size_t sz_attrib3_0 = 0; //position
	size_t sz_attrib4_0 = 0; //bone_weight
	//matrix indices attrib size not needed
	
	if(m_vertex_attribs3[0].size()) // position
	{
		sz_attrib3_0 = 3 * sizeof(float);
		vtxStride_1st_stream += sz_attrib3_0;
		vboSz += sz_attrib3_0 * m_vertex_attribs3[0].size();
	}
	if(m_vertex_attribs4[0].size()) //bone weight
	{
		sz_attrib4_0 = 4 * sizeof(float);
		vtxStride_1st_stream += sz_attrib4_0;
		vboSz += sz_attrib4_0 * m_vertex_attribs4[0].size();
	}
	if( m_nodes.size())
	{
		vtxStride_1st_stream += 4 * sizeof(KCL::uint8);
		vboSz += m_vertex_matrix_indices.size() * sizeof(KCL::uint8);
	}
	const size_t sizeOf_1st_stream = vboSz;


	
// 2nd stream stride:
	size_t vtxStride_2nd_stream = 0; // texcoord0, texcoord1, normal, tangent, color
	size_t sz_attrib2_0 = 0; //texcoord0
	size_t sz_attrib2_1 = 0; //texcoord1
	size_t sz_attrib3_1 = 0; //normal
	size_t sz_attrib3_2 = 0; //tangent
	size_t sz_attrib3_3 = 0; //color
	
	{
		size_t* sz_attrib2_alias[] = {&sz_attrib2_0, &sz_attrib2_1};
		for(int i=0; i<2; ++i) //texcoord
		{
			if(m_vertex_attribs2[i].size())
			{
				*(sz_attrib2_alias[i]) = 2 * sizeof(float);
				vtxStride_2nd_stream += *(sz_attrib2_alias[i]);
				vboSz += *(sz_attrib2_alias[i]) * m_vertex_attribs2[i].size();
			}
		}
	}

	if(m_vertex_attribs3[1].size()) //normal
	{
#ifdef __glew_h__
		sz_attrib3_1 = 4 * sizeof(KCL::uint8);
#else
		sz_attrib3_1 = 4 * sizeof(KCL::int8);
#endif
		vtxStride_2nd_stream += sz_attrib3_1;
		vboSz += sz_attrib3_1 * m_vertex_attribs3[1].size();
	}

	if(m_vertex_attribs3[2].size()) //tangent
	{
#ifdef __glew_h__
		sz_attrib3_2 = 4 * sizeof(KCL::uint8);
#else
		sz_attrib3_2 = 4 * sizeof(KCL::int8);
#endif
		vtxStride_2nd_stream += sz_attrib3_2;
		vboSz += sz_attrib3_2 * m_vertex_attribs3[2].size();
	}

	if(m_vertex_attribs3[3].size()) //color
	{
		sz_attrib3_3 = 3 * sizeof(float);
		vtxStride_2nd_stream += sz_attrib3_3;
		vboSz += sz_attrib3_3 * m_vertex_attribs3[3].size();
	}
// 2nd stream stride - END


	const size_t offset_attrib3_0 = 0;                                 // position
	const size_t offset_attrib4_0 = sz_attrib3_0 ;                     // bone_weight
	const size_t offset_partition = offset_attrib4_0  + sz_attrib4_0;  // matrix indices

	const size_t offset_attrib2_0 = sizeOf_1st_stream + 0 ;            // texcoord0
	const size_t offset_attrib2_1 = offset_attrib2_0  + sz_attrib2_0 ; // texcoord1
	const size_t offset_attrib3_1 = offset_attrib2_1  + sz_attrib2_1 ; // normal
	const size_t offset_attrib3_2 = offset_attrib3_1  + sz_attrib3_1 ; // tangent
	const size_t offset_attrib3_3 = offset_attrib3_2  + sz_attrib3_2 ; // color


	//size_t offset = 0;

	m_vertex_attribs[0].m_size = 3;
	m_vertex_attribs[0].m_type = GL_FLOAT;
	m_vertex_attribs[0].m_stride = vtxStride_1st_stream;
	m_vertex_attribs[0].m_data = (const void*)offset_attrib3_0; //position

	if( m_nodes.size())
	{
		m_vertex_attribs[1].m_size = 4;
		m_vertex_attribs[1].m_type = GL_FLOAT;
		m_vertex_attribs[1].m_stride = vtxStride_1st_stream;
		m_vertex_attribs[1].m_data = (const void*)offset_attrib4_0; //bone_weight

		m_vertex_attribs[2].m_size = 4;
		m_vertex_attribs[2].m_type = GL_UNSIGNED_BYTE;
		m_vertex_attribs[2].m_stride = vtxStride_1st_stream;
		m_vertex_attribs[2].m_data = (const void*)(offset_partition ); //matrix indices
	}


	//NORMAL
	m_vertex_attribs[3].m_size = 3;
#ifdef __glew_h__
	m_vertex_attribs[3].m_type = GL_UNSIGNED_BYTE;
#else
	m_vertex_attribs[3].m_type = GL_BYTE;
#endif
	m_vertex_attribs[3].m_stride = vtxStride_2nd_stream;
	m_vertex_attribs[3].m_normalized = true;
	m_vertex_attribs[3].m_data = (const void*)offset_attrib3_1; //normal

	//TANGENT
	m_vertex_attribs[4].m_size = 3;
#ifdef __glew_h__
	m_vertex_attribs[4].m_type = GL_UNSIGNED_BYTE;
#else
	m_vertex_attribs[4].m_type = GL_BYTE;
#endif
	m_vertex_attribs[4].m_stride = vtxStride_2nd_stream;
	m_vertex_attribs[4].m_normalized = true;
	m_vertex_attribs[4].m_data = (const void*)offset_attrib3_2; //tangent

	m_vertex_attribs[5].m_size = 3;
	m_vertex_attribs[5].m_type = GL_FLOAT;
	m_vertex_attribs[5].m_stride = vtxStride_2nd_stream;
	m_vertex_attribs[5].m_data = (const void*)offset_attrib3_3; //color

	m_vertex_attribs[6].m_size = 2;
	m_vertex_attribs[6].m_type = GL_FLOAT;
	m_vertex_attribs[6].m_stride = vtxStride_2nd_stream;
	m_vertex_attribs[6].m_data = (const void*)offset_attrib2_0; //texcoord0

	m_vertex_attribs[7].m_size = 2;
	m_vertex_attribs[7].m_type = GL_FLOAT;
	m_vertex_attribs[7].m_stride = vtxStride_2nd_stream;
	m_vertex_attribs[7].m_data = (const void*)offset_attrib2_1; //texcoord1

	//NOTE: bumpmap texcoord = colormap texcoord
	m_vertex_attribs[9].m_size = 2;
	m_vertex_attribs[9].m_type = GL_FLOAT;
	m_vertex_attribs[9].m_stride = vtxStride_2nd_stream;
	m_vertex_attribs[9].m_data = (const void*)offset_attrib2_0; // texcoord


	std::vector<KCL::int8> data;
	data.reserve( vboSz);

	//push 1st stream
	for(size_t i=0; i<m_vertex_attribs3[0].size(); ++i)
	{
		push_back_multiple_bytes(data, m_vertex_attribs3[0][i].v); // position
		
		if(m_vertex_attribs4[0].size()) // bone_weight
		{
			push_back_multiple_bytes(data, m_vertex_attribs4[0][i].v);
		}
		
		if(m_nodes.size()) // matrix indices
		{
			data.push_back( m_vertex_matrix_indices[i*4 + 0] );
			data.push_back( m_vertex_matrix_indices[i*4 + 1] );
			data.push_back( m_vertex_matrix_indices[i*4 + 2] );
			data.push_back( m_vertex_matrix_indices[i*4 + 3] );
		}
	}

	//push 2nd stream
	for(size_t i=0; i<m_vertex_attribs3[0].size(); ++i)
	{
		for(size_t j=0; j<2; ++j) // texcoord0 and texcoord1
		{
			if(m_vertex_attribs2[j].size())
			{
				push_back_multiple_bytes(data, m_vertex_attribs2[j][i].v);
			}
		}

		if(m_vertex_attribs3[1].size()) //normal --> byte
		{

#ifdef __glew_h__
			KCL::uint8 byteVec[4];
			byteVec[0] = (KCL::uint8)(( (m_vertex_attribs3[1][i].x + 1.0f) / 2.0f  ) * 255.0f);
			byteVec[1] = (KCL::uint8)(( (m_vertex_attribs3[1][i].y + 1.0f) / 2.0f  ) * 255.0f);
			byteVec[2] = (KCL::uint8)(( (m_vertex_attribs3[1][i].z + 1.0f) / 2.0f  ) * 255.0f);
			
			push_back_multiple_bytes(data, byteVec);
#else
			KCL::int8 byteVec[4];
			byteVec[0] = (KCL::int8)(m_vertex_attribs3[1][i].x * 127.0f);
			byteVec[1] = (KCL::int8)(m_vertex_attribs3[1][i].y * 127.0f);
			byteVec[2] = (KCL::int8)(m_vertex_attribs3[1][i].z * 127.0f);
			
			push_back_multiple_bytes(data, byteVec);
#endif
		}

		if(m_vertex_attribs3[2].size()) //tangent --> byte
		{

#ifdef __glew_h__
			KCL::uint8 byteVec[4];
			byteVec[0] = (KCL::uint8)(( (m_vertex_attribs3[2][i].x + 1.0f) / 2.0f  ) * 255.0f);
			byteVec[1] = (KCL::uint8)(( (m_vertex_attribs3[2][i].y + 1.0f) / 2.0f  ) * 255.0f);
			byteVec[2] = (KCL::uint8)(( (m_vertex_attribs3[2][i].z + 1.0f) / 2.0f  ) * 255.0f);
			
			push_back_multiple_bytes(data, byteVec);
#else
			KCL::int8 byteVec[4];
			byteVec[0] = (KCL::int8)(m_vertex_attribs3[2][i].x * 127.0f);
			byteVec[1] = (KCL::int8)(m_vertex_attribs3[2][i].y * 127.0f);
			byteVec[2] = (KCL::int8)(m_vertex_attribs3[2][i].z * 127.0f);
			
			push_back_multiple_bytes(data, byteVec);
#endif
		}

		if(m_vertex_attribs3[3].size()) // color
		{
			push_back_multiple_bytes(data, m_vertex_attribs3[3][i].v);			
		}

	}


	size_t offsetInVboPool;
	VboPool::Instance()->AddData(vboSz, (const void*)(&(data.front())) , m_vbo, offsetInVboPool);


	for(size_t k=0; k<16; ++k)
	{
		size_t offset = (size_t)m_vertex_attribs[k].m_data;
		offset += offsetInVboPool;
		m_vertex_attribs[k].m_data = (const void*)offset;
	}

	size_t offsetInIndexBufferPool;
	for( KCL::uint32 i=0; i<2; i++)
	{
		IndexBufferPool::Instance()->AddData(m_vertex_indices[i].size() * sizeof(KCL::uint16), (const void*)(&m_vertex_indices[i][0]), m_ebo[i].m_buffer, offsetInIndexBufferPool);
		m_ebo[i].m_offset = (const void*)offsetInIndexBufferPool;

		m_index_counts[i] = m_vertex_indices[i].size();
		m_vertex_indices[i].clear();
	}
}

