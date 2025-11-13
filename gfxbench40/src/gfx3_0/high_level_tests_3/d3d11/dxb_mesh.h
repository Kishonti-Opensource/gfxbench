/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DXB_MESH_H
#define DXB_MESH_H

#define USE_VBO

#include <kcl_base.h>
#include <kcl_material.h>
#include <kcl_node.h>
#include <kcl_aabb.h>

#include <krl_mesh.h>

#include <string>
#include <vector>
#include <set>

#include "d3d11/DX.h"
#include "d3d11/dxb_buffer.h"

class Shader;

namespace DXB
{
    struct InstanceData
	{
		DirectX::XMMATRIX Instance_MV;
        DirectX::XMMATRIX Instance_Inv_MV;
    };

	class Mesh3 : public KRL::Mesh3
	{
		friend class KCL::Mesh3;

    public:
		Mesh3(const char *name) : KRL::Mesh3(name), m_instanceData(NULL)
		{
			m_index_counts[0] = 0;
			m_index_counts[1] = 0;
		}

        void UpdateInstanceVBO(float* floatData, int instanceVBOelemCount)
        {
            if(!m_instanceData)
            {
                m_instanceData = dynamic_cast<DXB::DXBVertexBuffer*>(KCL::VertexBuffer::factory->CreateBuffer<InstanceData>(m_instanceCount, true, false));
                m_instanceData->commit();
            }

            m_instanceData->updateData(floatData, instanceVBOelemCount);
        }

        void BindInstanceVBO()
        {
            assert(m_instanceData);
            m_instanceData->bind(1);
        }

		void InitVertexAttribs();
		void BindLayout27( Shader *shader); //TODO: atvinni mesh3ba, materiallal/vs-el indexelve (override material miatt)
        void BindLayout27_Skinned( Shader *shader);
        void BindLayout30( Shader *shader);
        void BindLayout30_Skinned( Shader *shader);
        void BindLayout30_Instanced( Shader *shader);

	protected:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout27;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout27_skinned;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout30;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout30_skinned;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout30_instanced;

		void InitVertexAttribsVBO_ArrayOfStructures();
		void InitVertexAttribsVBO_PositionStreamFirst_ArrayOfStructures();

        ~Mesh3() { delete m_instanceData; }

        DXB::DXBVertexBuffer* m_instanceData;
	};

	class DXBMesh3Factory : public KCL::Mesh3Factory
	{
	public:
		virtual KCL::Mesh3 *Create(const char* name)
		{
			return new Mesh3(name);
		};
	};
}

#endif
