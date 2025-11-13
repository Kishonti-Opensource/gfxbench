/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once

#include "test_base.h"
#include "kcl_buffer.h"
#include "d3d11/dxb_texture.h"

#include <DirectXMath.h>
#include <d3d11_1.h>

#define SAFE_DELETE(x)	if (NULL != (x)) { delete (x); (x) = NULL; }

enum BlendState
{
	Blend_Overwrite,
	Blend_Add,
	Blend_Alpha,
	Blend_PremultipliedAlpha,
	Blend_ConstantAlpha
};

enum DepthState
{
	Depth_Disabled,
	Depth_EqualNoWrite,
	Depth_LessEqual,
	Depth_LessEqualNoWrite,
	Depth_GreaterEqual,
	Depth_GreaterEqualNoWrite
};

struct BillboardVertex
{
	KCL::Vector3D Position;
	KCL::Vector2D  TexCoord;
	KCL::Vector4D Color;

	BillboardVertex()
	{
	}

	BillboardVertex(float posX, float posY, float posZ, float texU, float texV, float colR, float colG, float colB, float colA)
	{
		Position.set(posX, posY, posZ);
		TexCoord.set(texU, texV);
		Color.set(colR, colG, colB, colA);
	}
};

class  LowLevelBase: public GLB::TestBase
{
private:
	DXB::DXBTextureFactory textureFactory;
	ID3D11BlendState* m_blendStateAdd;
	ID3D11BlendState* m_blendStateAlpha;
	ID3D11BlendState* m_blendStateConstantAlpha;
	ID3D11BlendState* m_blendStateOverwrite;
	ID3D11BlendState* m_blendStatePremulAlpha;

	ID3D11DepthStencilState* m_depthDisabled;
	ID3D11DepthStencilState* m_depthEqualNoWrite;
	ID3D11DepthStencilState* m_depthLessEqual;
	ID3D11DepthStencilState* m_depthLessEqualNoWrite;
	ID3D11DepthStencilState* m_depthGreaterEqual;
	ID3D11DepthStencilState* m_depthGreaterEqualNoWrite;

	ID3D11InputLayout*	m_defaultInputLayout;

	BlendState m_blendState;
	DepthState m_depthState;

protected:
	float m_screenWidth;
	float m_screenHeight;
	float m_invScreenWidth;
	float m_invScreenHeight;
	float m_aspectRatio;

	KCL::Vector3D m_clearColor;
	Shader* m_defaultShader;
	KCL::VertexBuffer* m_quadVertices;
	KCL::IndexBuffer* m_quadIndices;
	KCL::VertexBuffer* m_boundVertices;
	KCL::IndexBuffer* m_boundIndices;

public:
	LowLevelBase(const GlobalTestEnvironment* const gte);
	~LowLevelBase();

	virtual KCL::TextureFactory &TextureFactory()
	{
		return textureFactory;
	};

	/* override */ virtual KCL::KCL_Status init();
	/* override */ virtual void resetEnv();
	/* override */ virtual void FreeResources();

	/**
		Clears internal rendering state. Every state after this call will be applied to the DX API.
	*/
	virtual void clearState();

protected:
	void clearColor(float r, float g, float b, float a) const;
	void setDepthState(DepthState ds);
	void setBlendState(BlendState bs, float blendFactor = 1.0f);
	inline void drawTriangles(KCL::VertexBuffer* vertices, KCL::IndexBuffer* indices)
	{
		drawTriangles(vertices, indices, indices->getIndexCount()); 
	}

	void drawTriangles(KCL::VertexBuffer* vertices, KCL::IndexBuffer* indices, int indexCount);
};