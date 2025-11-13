/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_MESH_H
#define GLB_MESH_H

#define USE_VBO

#include <krl_mesh.h>
#include <kcl_base.h>
#include <kcl_mesh.h>
#include <kcl_material.h>
#include <kcl_node.h>
#include <kcl_aabb.h>


#include <string>
#include <vector>
#include <set>

#include "platform.h"

namespace GLB
{
	class Mesh3 : public KRL::Mesh3
	{
		friend class KCL::Mesh3;
		friend class KCL::Mesh;
		friend class Mesh3Factory;
	public:
		virtual void InitVertexAttribs(bool reinit = false);

        KCL::uint32 m_instanceVBO;

        void UpdateInstanceVBO(float* floatData, int byteCount) 
        { 
            if(!m_instanceVBO)
            {
                glGenBuffers(1, &m_instanceVBO);
            }
             
            glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
	        glBufferData(GL_ARRAY_BUFFER, byteCount, floatData, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        void BindInstanceVBO()
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
        }

	protected:

		void InitVertexAttribsVBO_ArrayOfStructures(bool reinit);
		void InitVertexAttribsVBO_PositionStreamFirst_ArrayOfStructures();

		Mesh3(const char *name) : KRL::Mesh3(name), m_instanceVBO(0)
		{
			m_index_counts[0] = 0;
			m_index_counts[1] = 0;
		}

        virtual ~Mesh3()
        {
            glDeleteBuffers(1, &m_instanceVBO);
        }
	};
	
	class Mesh3Factory : public KCL::Mesh3Factory
	{
	public:
		virtual KCL::Mesh3 *Create(const char *name);
	};
		
}

#endif
