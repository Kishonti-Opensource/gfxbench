/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_MESH_H
#define MTL_MESH_H

#include <krl_mesh.h>
#include <kcl_base.h>
#include <kcl_mesh.h>
#include <kcl_material.h>
#include <kcl_node.h>
#include <kcl_aabb.h>
#include <kcl_scene_version.h>

#include <string>
#include <vector>
#include <set>

#include "platform.h"
#include <Metal/Metal.h>

#include <string>
#include <vector>
#include <set>

#include "platform.h"
#include "mtl_globals.h"



namespace GFXB4
{
	class Mesh3 : public KRL::Mesh3
	{
		friend class Factory;

		static const KCL::uint32 NUM_INDEX_BUFFERS = 2;

		enum
		{
			POSITION,
			NORMAL,
			TANGENT,
			TEXTURE_COORD0,
			TEXTURE_COORD1,
			ATTRIBUTE_COUNT
		};

	public:
		Mesh3(const char *name);
		virtual ~Mesh3();

		void InitVertexAttribs();

		static MTLVertexDescriptor * GetVertexDescriptor();
		const id<MTLBuffer> GetVertexBuffer() const { return m_vertex_buffer; }
		const id<MTLBuffer> GetIndexBuffer(KCL::uint32 lod) const { return m_index_buffer[lod]; }

	private:
		static MTLVertexDescriptor * s_vertex_descriptor;
		id <MTLBuffer> m_vertex_buffer;
		id <MTLBuffer> m_index_buffer[NUM_INDEX_BUFFERS];
		id <MTLDevice> m_device;
	};

	class Mesh3Factory : public KCL::Mesh3Factory
	{
	public:
		virtual KCL::Mesh3 * Create(const char *name);
	};
}

#endif // MTL_MESH_H
