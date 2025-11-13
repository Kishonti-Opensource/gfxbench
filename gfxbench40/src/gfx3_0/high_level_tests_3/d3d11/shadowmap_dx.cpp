/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "shadowmap.h"
#include "platform.h"
#include "d3d11/shader.h"
#include <string>
#include "d3d11/fbo3.h"
#include "d3d11/vbopool.h"
#include "shadowmap.h"

using namespace std;

ShadowMap* ShadowMap::Create( int size, const std::string &m, uint32 &fullscreen_quad_vbo, bool is_blur_enabled)
{
	if( m == "simple projective")
	{
		return new ShadowMap( size, m, fullscreen_quad_vbo, is_blur_enabled);
	}
	else if( m == "depth map(depth)")
	{
		return new ShadowMap( size, m, fullscreen_quad_vbo, is_blur_enabled);
	}
	else if( m == "depth map(color)")
	{
		return new ShadowMap( size, m, fullscreen_quad_vbo, is_blur_enabled);
	}
	else
	{
		return 0;
	}
}


ShadowMap::ShadowMap( int size, const std::string &m, uint32 &fullscreen_quad_vbo, bool is_blur_enabled)
:
m_size( size),
m_blur( is_blur_enabled && m == "simple projective" ? new Blur(fullscreen_quad_vbo, m_size) : 0),
m_rboid( 0)
{
	if( m == "simple projective")
	{
		m_FBO = new FBO;
		m_FBO->init( size, size, RGB565_Linear, DEPTH_None);
		m_FBO->set_viewport( 1, 1, size - 2, size - 2);

	}
	else if( m == "depth map(depth)")
	{

		m_FBO = new FBO;
		m_FBO->initDepthOnly(size, size);
		m_FBO->set_viewport( 1, 1, size - 2, size - 2);
	}
	else if( m == "depth map(color)")
	{
		m_FBO = new FBO;
		m_FBO->init( size, size, RGB888_Linear, DEPTH_16_RB);
		m_FBO->set_viewport( 1, 1, size - 2, size - 2);
	}

	Bind();	
	Unbind();
}


ShadowMap::~ShadowMap()
{
	delete m_blur;
	delete m_FBO;
}


void ShadowMap::Bind()
{
	FBO::bind( m_FBO);
}
	
void ShadowMap::Unbind()
{
	FBO::discardDepth(m_FBO);
	ApplyBlur();
	FBO::bind( 0);
}

void ShadowMap::Clear()
{
	FBO::clear( m_FBO);
}

void ShadowMap::ApplyBlur()
{
	if (!IsBlurEnabled())
		return;
	FBO::bind( m_blur->m_aux_fbo1);
	FBO::discard( m_blur->m_aux_fbo1);
	
	{
		m_blur->m_shader->Bind();

		DX::getContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		DX::getContext()->IASetInputLayout( m_blur->m_inputLayout.Get());

		VboPool::Instance()->BindBuffer( m_blur->m_vbo);
		IndexBufferPool::Instance()->BindBuffer( m_blur->m_ebo);

		ID3D11ShaderResourceView* textureView = m_FBO->Get();
		DX::getContext()->PSSetShaderResources(0, 1, &textureView);

		DX::getContext()->VSSetConstantBuffers(
			0,
			1,
			m_blur->m_constantBuffer.GetAddressOf()
			);

		m_blur->m_constantBufferData.offset_2d.x = 1.0f / m_size;
		m_blur->m_constantBufferData.offset_2d.y = 0.0f;

		D3D11_MAPPED_SUBRESOURCE sr;
		DX::getContext()->Map(m_blur->m_constantBuffer.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&sr);
		CopyMemory(sr.pData,&m_blur->m_constantBufferData,sizeof(m_blur->m_constantBufferData));
		DX::getContext()->Unmap(m_blur->m_constantBuffer.Get(),0);
		
		DX::getStateManager()->ApplyStates();
		DX::getContext()->DrawIndexed( 6, 0, 0);
	}

	FBO::bind( m_blur->m_aux_fbo2);
	FBO::discard( m_blur->m_aux_fbo2);

	{
		ID3D11ShaderResourceView* textureView = m_blur->m_aux_fbo1->Get();
		DX::getContext()->PSSetShaderResources(0, 1, &textureView);

		DX::getContext()->VSSetConstantBuffers(
			0,
			1,
			m_blur->m_constantBuffer.GetAddressOf()
			);

		m_blur->m_constantBufferData.offset_2d.x = 0.0f;
		m_blur->m_constantBufferData.offset_2d.y = 1.0f / m_size;

		D3D11_MAPPED_SUBRESOURCE sr;
		DX::getContext()->Map(m_blur->m_constantBuffer.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&sr);
		CopyMemory(sr.pData,&m_blur->m_constantBufferData,sizeof(m_blur->m_constantBufferData));
		DX::getContext()->Unmap(m_blur->m_constantBuffer.Get(),0);
		
		DX::getStateManager()->ApplyStates();
		DX::getContext()->DrawIndexed( 6, 0, 0);
	}
}


ShadowMap::Blur::Blur(KCL::uint32 &blur_fullscreen_quad_vbo, int size)
	:
	m_fullscreen_quad_vbo( blur_fullscreen_quad_vbo)
{
/*	for(int i=0; i<2; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, m_aux_texture[i]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);

		glBindTexture(GL_TEXTURE_2D, 0);


		//FBO::bind( m_aux_fbo[i]);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_aux_texture[i], 0);
		//FBO::bind( 0);

	}	*/

	m_aux_fbo1 = new FBO;
	m_aux_fbo1->init( size, size, RGB565_Linear, DEPTH_None);

	m_aux_fbo2 = new FBO;
	m_aux_fbo2->init( size, size, RGB565_Linear, DEPTH_None);

	KCL::KCL_Status t;
	m_shader = Shader::CreateShader( "blur.vs", "blur.fs", 0, t);

	{
		float screenAlignedQuad[] =
		{
			-1, -1, 0,
			1, -1, 0,
			1,  1, 0,
			-1,  1, 0,
		};

		const uint16 idx[] = {0, 1, 2, 2, 3, 0}; //6

		uint32 offset;
		VboPool::Instance()->AddData(4*3*sizeof(float), (const void*)screenAlignedQuad, m_vbo, offset, 3*sizeof(float));
		IndexBufferPool::Instance()->AddData(6*sizeof(uint16), (const void*)idx, m_ebo, offset);

		std::vector<D3D11_INPUT_ELEMENT_DESC> vertexDesc;
		
		D3D11_INPUT_ELEMENT_DESC position   = { "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 };

		vertexDesc.push_back( position   );

		if( m_shader)
		{
			DX_THROW_IF_FAILED(
				DX::getDevice()->CreateInputLayout(
				&vertexDesc[0],
				vertexDesc.size(),
				&m_shader->m_vs.m_bytes[0],
				m_shader->m_vs.m_bytes.size(),
				&m_inputLayout
				)
				);
		}
	}
	{
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateBuffer(
			&CD3D11_BUFFER_DESC(sizeof(ConstantBuffer), D3D11_BIND_CONSTANT_BUFFER,D3D11_USAGE_DYNAMIC,D3D11_CPU_ACCESS_WRITE),
			nullptr,
			&m_constantBuffer
			)
			);
	}
}


ShadowMap::Blur::~Blur()
{
	delete m_aux_fbo1;
	delete m_aux_fbo2;
}
