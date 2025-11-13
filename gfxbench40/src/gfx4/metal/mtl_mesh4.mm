/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#include "mtl_globals.h"
#include "mtl_mesh4.h"
#include "mtl_factories.h"

#include "platform.h"
#include "vbopool.h"
#include "misc2.h"

#include "mtl_types.h"
#import "graphics/metalgraphicscontext.h"

using namespace MetalRender;

MTLVertexDescriptor * GFXB4::Mesh3::s_vertex_descriptor;

GFXB4::Mesh3::Mesh3(const char *name) :
	KRL::Mesh3(name),
	m_vertex_buffer(nil),
	m_index_buffer(),
	m_device(MetalRender::GetContext()->getDevice())
{
	m_index_counts[0] = 0;
	m_index_counts[1] = 0;
}

GFXB4::Mesh3::~Mesh3()
{
	releaseObj(m_vertex_buffer);

	for (KCL::uint32 i = 0; i < NUM_INDEX_BUFFERS; ++i)
	{
		releaseObj(m_index_buffer[i]);
	}
}

MTLVertexDescriptor * GFXB4::Mesh3::GetVertexDescriptor()
{
	if (s_vertex_descriptor == NULL)
	{
		s_vertex_descriptor = [[MTLVertexDescriptor alloc] init];

		// POSITION
		s_vertex_descriptor.attributes[0].format = MTLVertexFormatFloat3;
		s_vertex_descriptor.attributes[0].bufferIndex = 0;
		s_vertex_descriptor.attributes[0].offset = 0;

		// NORMAL
		s_vertex_descriptor.attributes[1].format = MTLVertexFormatUChar3Normalized;
		s_vertex_descriptor.attributes[1].bufferIndex = 0;
		s_vertex_descriptor.attributes[1].offset = 12;

		// TANGENT
		s_vertex_descriptor.attributes[2].format = MTLVertexFormatUChar3Normalized;
		s_vertex_descriptor.attributes[2].bufferIndex = 0;
		s_vertex_descriptor.attributes[2].offset = 16;

		// TEXCOORD0
		s_vertex_descriptor.attributes[3].format = MTLVertexFormatFloat2;
		s_vertex_descriptor.attributes[3].bufferIndex = 0;
		s_vertex_descriptor.attributes[3].offset = 20;

		// TEXCOORD1
		s_vertex_descriptor.attributes[4].format = MTLVertexFormatFloat2;
		s_vertex_descriptor.attributes[4].bufferIndex = 0;
		s_vertex_descriptor.attributes[4].offset = 28;

		// 34 + 2 bytes to align
		s_vertex_descriptor.layouts[0].stride = 36;
		s_vertex_descriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
	}

	return s_vertex_descriptor;
}

void GFXB4::Mesh3::InitVertexAttribs()
{
	CalculateTangents();

	size_t vertex_stride = 36;
	size_t num_verts =  m_vertex_attribs3[0].size();
	size_t buffer_size = num_verts * vertex_stride;

	m_vertex_buffer = [m_device newBufferWithLength:buffer_size options:STORAGE_MODE_MANAGED_OR_SHARED];

	const KCL::Vector2D default_vector_2d = KCL::Vector2D(0.0f, 0.0f);
	const KCL::int8 * default_vector_2d_ptr = (KCL::int8*)(&default_vector_2d);

	const KCL::Vector3D default_vector_3d = KCL::Vector3D(0.0f, 0.0f, 0.0f);
	const KCL::int8 * default_vector_3d_ptr = (KCL::int8*)(&default_vector_3d);

	KCL::int8 * data = (KCL::int8*)[m_vertex_buffer contents];

	for (KCL::uint32 i = 0; i < num_verts; ++i)
	{
		KCL::int8 * vert_dst = data + i * vertex_stride;

		const KCL::int8 * src_ptrs[ATTRIBUTE_COUNT];

		src_ptrs[POSITION]           = !m_vertex_attribs3[0].empty() ? (KCL::int8 *)(&m_vertex_attribs3[0][i]) : nullptr;
		src_ptrs[NORMAL]             = !m_vertex_attribs3[1].empty() ? (KCL::int8 *)(&m_vertex_attribs3[1][i]) : default_vector_3d_ptr;
		src_ptrs[TANGENT]            = !m_vertex_attribs3[2].empty() ? (KCL::int8 *)(&m_vertex_attribs3[2][i]) : default_vector_3d_ptr;
		src_ptrs[TEXTURE_COORD0]     = !m_vertex_attribs2[0].empty() ? (KCL::int8 *)(&m_vertex_attribs2[0][i]) : default_vector_2d_ptr;
		src_ptrs[TEXTURE_COORD1]     = !m_vertex_attribs2[1].empty() ? (KCL::int8 *)(&m_vertex_attribs2[1][i]) : default_vector_2d_ptr;

		KCL::int8 * attr_dst = vert_dst;

		for (KCL::uint32 j = 0; j < ATTRIBUTE_COUNT; ++j)
		{
			size_t attr_size = 0;

			switch (j)
			{
				case POSITION:
					attr_size = sizeof(float) * 3;
					memcpy(attr_dst, src_ptrs[j], attr_size);
					break;

				case NORMAL:
				case TANGENT:
				{
					attr_size = sizeof(KCL::int8) * 3;
					KCL::Vector3D * v = (KCL::Vector3D *)(src_ptrs[j]);

					attr_dst[0] = (KCL::uint8)(((v->x + 1.0f) / 2.0f) * 255.0f);
					attr_dst[1] = (KCL::uint8)(((v->y + 1.0f) / 2.0f) * 255.0f);
					attr_dst[2] = (KCL::uint8)(((v->z + 1.0f) / 2.0f) * 255.0f);
					break;
				}

				case TEXTURE_COORD0:
				case TEXTURE_COORD1:
					attr_size = sizeof(float) * 2;
					memcpy(attr_dst, src_ptrs[j], attr_size);
					break;
			}
            
            attr_size = (attr_size + 3) & ~3;

			attr_dst += attr_size;
		}
	}

#if !TARGET_OS_EMBEDDED
	[m_vertex_buffer didModifyRange: NSMakeRange(0, buffer_size)];
#endif

	DeleteVertexAttribs();

	for (KCL::uint32 i = 0; i < NUM_INDEX_BUFFERS; i++)
	{
		assert(!m_ebo[i].m_buffer);

		size_t buffer_size = m_vertex_indices[i].size() * sizeof(KCL::uint16);

		m_index_buffer[i] = [m_device newBufferWithBytes:(const void*)(&m_vertex_indices[i][0]) length:buffer_size options:STORAGE_MODE_MANAGED_OR_SHARED];

		m_index_counts[i] = static_cast<uint32_t>(m_vertex_indices[i].size());
		m_vertex_indices[i].clear();
	}
}


KCL::Mesh3 * GFXB4::Mesh3Factory::Create(const char *name)
{
	return new GFXB4::Mesh3(name);
}
