/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <d3d11_1.h>
#include <stack>

#define STATE_MANAGER_USE_NEW
//#define REPORT_STATE_MANAGER_OVERRIDE // only works for old statemanager. Reports states that are overridden by StateManager.

namespace DX
{
	class States
	{
	public:
		States();
		
		CD3D11_BLEND_DESC			m_blend;
		FLOAT						m_blend_factor[4];
		UINT 						m_blend_samplemask;
		CD3D11_DEPTH_STENCIL_DESC	m_depth_stencil;
		CD3D11_RASTERIZER_DESC		m_rasterizer;
		CD3D11_VIEWPORT				m_viewport;
	};

	bool operator==(const D3D11_RENDER_TARGET_BLEND_DESC &first, const D3D11_RENDER_TARGET_BLEND_DESC &second);
	bool operator==(const D3D11_DEPTH_STENCILOP_DESC &first, const D3D11_DEPTH_STENCILOP_DESC &second);
	bool operator==(const CD3D11_BLEND_DESC &first, const CD3D11_BLEND_DESC &second);
	bool operator==(const CD3D11_DEPTH_STENCIL_DESC &first, const CD3D11_DEPTH_STENCIL_DESC &second);
	bool operator==(const CD3D11_RASTERIZER_DESC &first, const CD3D11_RASTERIZER_DESC &second);

	class StateManager
	{
	protected:
#if defined STATE_MANAGER_USE_NEW
		struct InternalStateDesc
		{
			ID3D11BlendState* blendState;
			FLOAT blendFactor[4];
			UINT sampleMask;
			ID3D11DepthStencilState* depthStencilState;
			ID3D11RasterizerState* rasterizerState;
			UINT stencilRef;
			D3D11_VIEWPORT viewportState;
		};

		std::stack<InternalStateDesc> m_state_stack;
#else
		std::stack<States>	m_state_stack;
		States				m_last_state;

		void ApplyStateBlend(bool forced = false);
		void ApplyStateDepth(bool forced = false);
		void ApplyStateRaster(bool forced = false);
		void ApplyStateViewport(bool forced = false);
#endif
	public:
		StateManager();
		
		void ApplyStates(bool forced = false);

		void Push();
		void Push(const States &states);
		void Pop();
		
		void SetBlendEnabled(BOOL Enabled);
		void SetBlendFunction(
			D3D11_BLEND SrcBlend,                      D3D11_BLEND DestBlend,                       D3D11_BLEND_OP BlendOp=D3D11_BLEND_OP_ADD, 
			D3D11_BLEND SrcBlendAlpha=D3D11_BLEND_ONE, D3D11_BLEND DestBlendAlpha=D3D11_BLEND_ZERO, D3D11_BLEND_OP BlendOpAlpha=D3D11_BLEND_OP_ADD
			);
		void SetBlendFactor(FLOAT *BlendFactor);
		void SetBlendColorMask(UINT8 Mask);
		void SetBlendSampleMask(UINT8 Mask);
		
		void SetDepthEnabled(BOOL Enabled);
		void SetDepthFunction(D3D11_COMPARISON_FUNC DepthFunc);
		void SetDepthMask(D3D11_DEPTH_WRITE_MASK DepthWriteMask);

		void SetRasterFrontClockwise(BOOL Clockwise);
		void SetRasterCullMode(D3D11_CULL_MODE CullMode);
		
		void SetViewport(const CD3D11_VIEWPORT &ViewPort);
		void SetViewportNoDepth(const CD3D11_VIEWPORT &ViewPort);
		void SetViewportDepthrange(float MinDepth=0.0f, float MaxDepth=1.0f);

	};

}//namespace GLB

#endif
