/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "newnotificationmanager_dx.h"
#include "dxb_texture.h"
#include "d3d11/vbopool.h"
#include "fbo3.h"

namespace GLB {

	__declspec(align(16)) struct ConstantBufferScale
	{
		DirectX::XMFLOAT2		scale;
	};

	NewNotificationManager* NewNotificationManager::NewInstance()
	{
		return new NewNotificationManagerDX();
	}

	NewNotificationManagerDX::NewNotificationManagerDX()
	{
		m_constantBuffer = NULL;

		KCL::KCL_Status error;

		std::string prefix = Shader::getShaderPrefix();
		Shader::setShaderPrefix("");
		m_shader = Shader::CreateShader("newnotificationmanager.vs", "offscreenmanager.fs", 0, error);
		
		if (m_shader == NULL)
		{
			return;
		}

		Shader::setShaderPrefix(prefix);
		const float vbodata[8] = {
			-1.0f, //0 vertex
			-1.0f, //0 vertex
			1.0f, //1 vertex
			-1.0f, //1 vertex
			1.0f, //2 vertex
			1.0f, //2 vertex
			-1.0f, //3 vertex
			1.0f, //3 vertex
		};

		const KCL::uint16 idxdata[] = { 0, 1, 2, 0, 2, 3 }; //6

		KCL::uint32 offset;
		VboPool::Instance()->AddData(8 * sizeof(float), (const void*)vbodata, m_vbo, offset, 2 * sizeof(float));
		IndexBufferPool::Instance()->AddData(6 * sizeof(KCL::uint16), (const void*)idxdata, m_ebo, offset);



		std::vector<D3D11_INPUT_ELEMENT_DESC> vertexDesc;

		D3D11_INPUT_ELEMENT_DESC position = { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

		vertexDesc.push_back(position);

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateInputLayout(
			&vertexDesc[0],
			vertexDesc.size(),
			&m_shader->m_vs.m_bytes[0],
			m_shader->m_vs.m_bytes.size(),
			&m_inputlayout
			)
			);

		m_constantBuffer = KCL::ConstantBuffer::factory->CreateBuffer<ConstantBufferScale>();
		m_constantBuffer->commit();
	}

	NewNotificationManagerDX::~NewNotificationManagerDX()
	{
		delete m_constantBuffer;
	}

	void NewNotificationManagerDX::ShowLogo(bool stretch, bool blend)
	{
		if (m_shader == NULL)
		{
			return;
		}

		DX::Clear();
		ConstantBufferScale* cb = (ConstantBufferScale*)m_constantBuffer->map();

		if (cb!=NULL)
		{
			if (stretch)
			{
				cb->scale.x = 1.0;
				cb->scale.y = 1.0;
			}
			else
			{
				cb->scale.x = m_texture->getWidth()*1.0f / DX::getWidth();
				cb->scale.y = m_texture->getHeight()*1.0f / DX::getHeight();
			}

			m_constantBuffer->unmap();
			m_constantBuffer->bind(0);
		}

		FBO::bind(0);

		m_shader->Bind();
		m_texture->bind(0);
		VboPool::Instance()->BindBuffer(m_vbo);
		DX::getContext()->IASetInputLayout(m_inputlayout.Get());
		IndexBufferPool::Instance()->BindBuffer(m_ebo);

		DX::getStateManager()->Push();
		DX::getStateManager()->SetBlendEnabled(blend);
		DX::getStateManager()->SetBlendFunction(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);
		DX::getStateManager()->SetDepthEnabled(false);
		//DX::getStateManager()->SetRasterCullMode(D3D11_CULL_NONE);
		DX::getStateManager()->ApplyStates();
		DX::getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		DX::getContext()->DrawIndexed(
			6,
			0,
			0
			);
		DX::getStateManager()->Pop();
	}

	KCL::Texture *NewNotificationManagerDX::CreateTexture(const KCL::Image* img, bool releaseUponCommit)
	{
		return new DXB::DXBTexture(img, releaseUponCommit);
	}
}