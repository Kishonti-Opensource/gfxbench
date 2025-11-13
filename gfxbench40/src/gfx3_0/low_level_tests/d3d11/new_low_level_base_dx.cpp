/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "new_low_level_base_dx.h"
#include "d3d11/fbo3.h"
#include "d3d11/vbopool.h"

#define SAFE_RELEASE(x)	if (NULL != (x)) { delete (x); (x) = NULL; }

const UINT g_lowLevelVertexLayoutCount = 3;
const D3D11_INPUT_ELEMENT_DESC g_lowLevelVertexLayout[] =
{
	//  SemanticName; SemanticIndex; Format; InputSlot; AlignedByteOffset; InputSlotClass; InstanceDataStepRate;
	{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};


LowLevelBase::LowLevelBase(const GlobalTestEnvironment* const gte) : 
	GLB::TestBase(gte),	
	m_blendState((BlendState)-1),
	m_depthState((DepthState)-1),
	m_quadIndices(NULL),
	m_quadVertices(NULL),
	m_blendStateAdd(NULL),
	m_blendStateAlpha(NULL),
	m_blendStateConstantAlpha(NULL),
	m_blendStatePremulAlpha(NULL),
	m_blendStateOverwrite(NULL),
	m_depthDisabled(NULL),
	m_depthEqualNoWrite(NULL),
	m_depthLessEqual(NULL),
	m_depthLessEqualNoWrite(NULL),
	m_depthGreaterEqual(NULL),
	m_depthGreaterEqualNoWrite(NULL),
	m_boundVertices(NULL),
	m_boundIndices(NULL),
	m_defaultInputLayout(NULL),
	m_defaultShader(NULL)
{
}

LowLevelBase::~LowLevelBase()
{
}

void LowLevelBase::setDepthState(DepthState ds)
{
	if (ds == m_depthState)
	{
		return;
	}

	switch (ds)
	{
	case Depth_Disabled:
		DX::getContext()->OMSetDepthStencilState(m_depthDisabled, 0);
		break;

	case Depth_EqualNoWrite:
		DX::getContext()->OMSetDepthStencilState(m_depthEqualNoWrite, 0);
		break;

	case Depth_LessEqual:
		DX::getContext()->OMSetDepthStencilState(m_depthLessEqual, 0);
		break;

	case Depth_LessEqualNoWrite:
		DX::getContext()->OMSetDepthStencilState(m_depthLessEqualNoWrite, 0);
		break;

	case Depth_GreaterEqual:
		DX::getContext()->OMSetDepthStencilState(m_depthGreaterEqual, 0);
		break;

	case Depth_GreaterEqualNoWrite:
		DX::getContext()->OMSetDepthStencilState(m_depthGreaterEqualNoWrite, 0);
		break;

	default:
		return;
	}

	m_depthState = ds;
}

void LowLevelBase::setBlendState(BlendState bs, float blendFactor)
{
	if ((bs == m_blendState) && (bs != Blend_ConstantAlpha))
	{
		return;
	}

	switch (bs)
	{
	case Blend_Overwrite:
		DX::getContext()->OMSetBlendState(m_blendStateOverwrite, NULL, 0xffffffff);
		break;

	case Blend_Add:
		DX::getContext()->OMSetBlendState(m_blendStateAdd, NULL, 0xffffffff);
		break;

	case Blend_Alpha:
		DX::getContext()->OMSetBlendState(m_blendStateAlpha, NULL, 0xffffffff);
		break;

	case Blend_PremultipliedAlpha:
		DX::getContext()->OMSetBlendState(m_blendStatePremulAlpha, NULL, 0xffffffff);
		break;

	case Blend_ConstantAlpha:
		{
			float blendFactors[4] = 
			{
				blendFactor, blendFactor, blendFactor, blendFactor
			};

			DX::getContext()->OMSetBlendState(m_blendStateConstantAlpha, blendFactors, 0xffffffff);
			break;
		}

	default:
		return;
	}

	m_blendState = bs;
}

KCL::KCL_Status LowLevelBase::init()
{
	Shader::InitShaders("lowlevel",true);
	m_screenWidth = DX::getWidth();
	m_screenHeight = DX::getHeight();
	m_invScreenWidth = 1.0f / m_screenWidth;
	m_invScreenHeight = 1.0f / m_screenHeight;
	m_aspectRatio = m_screenWidth * m_invScreenHeight;

	// Init depth states...
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));

	DX_THROW_IF_FAILED(DX::getDevice()->CreateDepthStencilState(&dsDesc, &m_depthDisabled));

	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateDepthStencilState(&dsDesc, &m_depthEqualNoWrite));

	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateDepthStencilState(&dsDesc, &m_depthLessEqualNoWrite));

	dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateDepthStencilState(&dsDesc, &m_depthGreaterEqualNoWrite));

	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateDepthStencilState(&dsDesc, &m_depthGreaterEqual));

	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateDepthStencilState(&dsDesc, &m_depthLessEqual));

	// Init blend states...
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.IndependentBlendEnable = FALSE;

	D3D11_RENDER_TARGET_BLEND_DESC& rtbd = blendDesc.RenderTarget[0];
	ZeroMemory(&rtbd, sizeof(rtbd));
	rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateBlendState(&blendDesc, &m_blendStateOverwrite));

	rtbd.BlendEnable = TRUE;
	rtbd.BlendOp =
		rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;

	rtbd.SrcBlend =
		rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlend = 
		rtbd.DestBlendAlpha = D3D11_BLEND_ONE;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateBlendState(&blendDesc, &m_blendStateAdd));

	rtbd.DestBlend =
		rtbd.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateBlendState(&blendDesc, &m_blendStatePremulAlpha));

	rtbd.SrcBlend =
		rtbd.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateBlendState(&blendDesc, &m_blendStateAlpha));

	rtbd.DestBlend =
		rtbd.DestBlendAlpha = D3D11_BLEND_INV_BLEND_FACTOR;
	rtbd.SrcBlend =
		rtbd.SrcBlendAlpha = D3D11_BLEND_BLEND_FACTOR;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateBlendState(&blendDesc, &m_blendStateConstantAlpha));

	BillboardVertex vertices[] =
	{
		// Pos_XYZ, Tex_UV, Color_RGBA
		BillboardVertex(-1, -1, 0,	0, 0,	0, 0, 0, 1),
		BillboardVertex(-1,  1, 0,	0, 1,	0, 1, 0, 1),
		BillboardVertex( 1, -1, 0,	1, 0,	1, 0, 0, 1),
		BillboardVertex( 1,  1, 0,	1, 1,	1, 1, 0, 1)
	};

	m_quadVertices = KCL::VertexBuffer::factory->CreateBuffer<BillboardVertex>(vertices, 4);
	m_quadVertices->commit();

	KCL::uint16 indices[] = 
	{
		0, 1, 2, 1, 2, 3 
	};

	m_quadIndices = KCL::IndexBuffer::factory->CreateBuffer(indices, 6);
	m_quadIndices->commit();

	KCL::KCL_Status result;
	m_defaultShader = Shader::CreateShader("LowLevel_PassThrough.vs", "LowLevel_Color.fs", NULL, result);

	if (m_defaultShader == NULL)
	{
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateInputLayout(
			g_lowLevelVertexLayout,
			g_lowLevelVertexLayoutCount,
			&m_defaultShader->m_vs.m_bytes[0],
			m_defaultShader->m_vs.m_bytes.size(),
			&m_defaultInputLayout));

	DX::getContext()->IASetInputLayout(m_defaultInputLayout);
	DX::getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DX::StateManager* manager = DX::getStateManager();
	manager->SetDepthEnabled(false);
	manager->SetViewportDepthrange();
	manager->SetBlendEnabled(true);
	manager->SetBlendFunction(D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD);
	manager->SetRasterCullMode(D3D11_CULL_NONE);
	manager->ApplyStates(true);

	return KCL::KCL_TESTERROR_NOERROR;
}

void LowLevelBase::clearState()
{
	m_blendState = (BlendState)-1;
	m_depthState = (DepthState)-1;
	m_boundVertices = NULL;
	m_boundIndices = NULL;
}

void LowLevelBase::resetEnv()
{
	DX::getContext()->IASetInputLayout(m_defaultInputLayout);
	DX::getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	clearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, 1.0f);
	clearState();

	////DX::StateManager* manager = DX::getStateManager();
	////manager->SetDepthEnabled(false);
	////manager->SetViewportDepthrange();
	////manager->SetBlendEnabled(true);
	////manager->SetBlendFunction(D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD);
	////manager->SetRasterCullMode(D3D11_CULL_NONE);
	////manager->ApplyStates(true);
}

void LowLevelBase::FreeResources()
{
	SAFE_DELETE(m_quadIndices);
	SAFE_DELETE(m_quadVertices);

	SAFE_RELEASE(m_blendStateAdd);
	SAFE_RELEASE(m_blendStateAlpha);
	SAFE_RELEASE(m_blendStateConstantAlpha);
	SAFE_RELEASE(m_blendStateOverwrite);
	SAFE_RELEASE(m_blendStatePremulAlpha);

	SAFE_RELEASE(m_depthDisabled);
	SAFE_RELEASE(m_depthEqualNoWrite);
	SAFE_RELEASE(m_depthLessEqual);
	SAFE_RELEASE(m_depthLessEqualNoWrite);
	SAFE_RELEASE(m_depthGreaterEqual);
	SAFE_RELEASE(m_depthGreaterEqualNoWrite);

	SAFE_RELEASE(m_defaultInputLayout);

	// Restore sync between state manager and API by forcing all settings.
	DX::getStateManager()->ApplyStates(true);
}

void LowLevelBase::clearColor(float r, float g, float b, float a) const
{
	GLB::FBO::clear(NULL, r, g, b, a);
}

void LowLevelBase::drawTriangles(KCL::VertexBuffer* vertices, KCL::IndexBuffer* indices, int indexCount)
{
	if (vertices != m_boundVertices)
	{
		VboPool::Instance()->ForgetLastBound();
		vertices->bind(0);
		m_boundVertices = vertices;
	}

	if (indices != m_boundIndices)
	{
		IndexBufferPool::Instance()->ForgetLastBound();
		indices->bind(0);
		m_boundIndices = indices;
	}

	DX::getContext()->DrawIndexed(indexCount, 0, 0);
}
