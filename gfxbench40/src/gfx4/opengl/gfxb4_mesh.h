/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB4_MESH_H
#define GFXB4_MESH_H

#include "glb_mesh.h"

namespace GFXB4
{
	class Mesh3 : public GLB::Mesh3
	{
		friend class KCL::Mesh3;
		friend class Mesh3Factory;
	public:
		KCL::uint32 m_vao_4;
		virtual void InitVertexAttribs(bool reinit = false);
		virtual void InitVAO4();

	protected:

		Mesh3(const char *name) : GLB::Mesh3(name), m_vao_4(0) {}

		virtual ~Mesh3() 
		{
			if (m_vao_4 != 0)
			{
				glDeleteVertexArrays(1, &m_vao_4);
			}
		}
	};

	class Mesh3Factory : public KCL::Mesh3Factory
	{
	public:
		virtual KCL::Mesh3 *Create(const char *name);
	};
}

#endif
