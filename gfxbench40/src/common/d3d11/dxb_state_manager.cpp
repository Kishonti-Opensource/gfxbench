/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dxb_state_manager.h"

#include "DXUtils.h"
#include "DX.h"

using namespace DX;

const CD3D11_BLEND_DESC defaultBlendDesc(D3D11_DEFAULT);
const CD3D11_RASTERIZER_DESC defaultRasterizerDesc(D3D11_DEFAULT);
const CD3D11_DEPTH_STENCIL_DESC defaultDepthStencilDesc(D3D11_DEFAULT);

bool DX::operator==(const D3D11_RENDER_TARGET_BLEND_DESC &first, const D3D11_RENDER_TARGET_BLEND_DESC &second)
{
	if ( 
		first.BlendEnable == false && 
		second.BlendEnable == false && 
		first.RenderTargetWriteMask == second.RenderTargetWriteMask 
		)
		return true;

	return (
		first.BlendEnable           == second.BlendEnable           &&
		first.BlendOp               == second.BlendOp               &&
		first.BlendOpAlpha          == second.BlendOpAlpha          &&
		first.DestBlend             == second.DestBlend             &&
		first.DestBlendAlpha        == second.DestBlendAlpha        &&
		first.SrcBlend              == second.SrcBlend              &&
		first.SrcBlendAlpha         == second.SrcBlendAlpha         &&
		first.RenderTargetWriteMask == second.RenderTargetWriteMask
		);
}

bool DX::operator==(const CD3D11_BLEND_DESC &first, const CD3D11_BLEND_DESC &second)
{
	return (
		first.AlphaToCoverageEnable  == second.AlphaToCoverageEnable  &&
		first.IndependentBlendEnable == second.IndependentBlendEnable &&
		first.RenderTarget[0]        == second.RenderTarget[0]        &&
		first.RenderTarget[1]        == second.RenderTarget[1]        &&
		first.RenderTarget[2]        == second.RenderTarget[2]        &&
		first.RenderTarget[3]        == second.RenderTarget[3]        &&
		first.RenderTarget[4]        == second.RenderTarget[4]        &&
		first.RenderTarget[5]        == second.RenderTarget[5]        &&
		first.RenderTarget[6]        == second.RenderTarget[6]        &&
		first.RenderTarget[7]        == second.RenderTarget[7]
		);
}

bool DX::operator==(const D3D11_DEPTH_STENCILOP_DESC &first, const D3D11_DEPTH_STENCILOP_DESC &second)
{
	return (
		first.StencilDepthFailOp == second.StencilDepthFailOp &&
		first.StencilFailOp      == second.StencilFailOp      &&
		first.StencilFunc        == second.StencilFunc        &&
		first.StencilPassOp      == second.StencilPassOp
		);
}

bool DX::operator==(const CD3D11_DEPTH_STENCIL_DESC &first, const CD3D11_DEPTH_STENCIL_DESC &second)
{
	return (
		first.DepthEnable       == second.DepthEnable       &&
		first.DepthFunc         == second.DepthFunc         &&
		first.DepthWriteMask    == second.DepthWriteMask    &&
		first.StencilEnable     == second.StencilEnable     &&
		first.StencilReadMask   == second.StencilReadMask   &&
		first.StencilWriteMask  == second.StencilWriteMask  &&
		first.FrontFace         == second.FrontFace         &&
		first.BackFace          == second.BackFace
		);
}

bool DX::operator==(const CD3D11_RASTERIZER_DESC &first, const CD3D11_RASTERIZER_DESC &second)
{
	return (
		first.AntialiasedLineEnable == second.AntialiasedLineEnable &&
		first.MultisampleEnable     == second.MultisampleEnable     &&
		first.CullMode              == second.CullMode              &&
		first.FrontCounterClockwise == second.FrontCounterClockwise &&
		first.FillMode              == second.FillMode              &&
		first.DepthBias             == second.DepthBias             &&
		first.DepthBiasClamp        == second.DepthBiasClamp        &&
		first.DepthClipEnable       == second.DepthClipEnable       &&
		first.ScissorEnable         == second.ScissorEnable         &&
		first.SlopeScaledDepthBias  == second.SlopeScaledDepthBias
		);
}

States::States()
{
	m_depth_stencil    = defaultDepthStencilDesc;
	m_blend	           = defaultBlendDesc;
	m_blend_factor[0]  = 0;
	m_blend_factor[1]  = 0;
	m_blend_factor[2]  = 0;
	m_blend_factor[3]  = 0;
	m_blend_samplemask = 0xffffffff;
	m_rasterizer       = defaultRasterizerDesc;
	m_viewport         = CD3D11_VIEWPORT(0.0f, 0.0f, 1.0f, 1.0f);
	m_depth_stencil.StencilEnable = false;
}

#if !defined STATE_MANAGER_USE_NEW

StateManager::StateManager() : m_last_state()
{
	m_state_stack.push(m_last_state);
}

void StateManager::ApplyStates(bool forced)
{
	ApplyStateBlend(forced);
	ApplyStateDepth(forced);
	ApplyStateRaster(forced);
	ApplyStateViewport(forced);
}

#ifdef _DEBUG
inline void createDiffStr(const char* const last, const char* const curr, char* const diff)
{
	const char* lastptr = last;
	const char* currptr = curr;
	char* diffptr = diff;
	while (((*lastptr) != 0) && ((*currptr) != 0))
	{
		if (*lastptr != *currptr) *diffptr = '*'; else *diffptr = ' ';
		++lastptr;
		++currptr;
		++diffptr;
	}
	*diffptr = 0;
}

void CompareBlend(CD3D11_BLEND_DESC &currentBlendDesc, FLOAT currentBlendFactor[4], UINT currentSampleMask,
	CD3D11_BLEND_DESC lastBlendDesc, FLOAT lastBlendFactor[4], UINT lastSampleMask,
	CD3D11_BLEND_DESC nextBlendDesc, FLOAT nextBlendFactor[4], UINT nextSampleMask
	)
{
	if (!(currentBlendDesc == lastBlendDesc) && lastBlendDesc == nextBlendDesc
		|| currentBlendFactor[0] != lastBlendFactor[0] && lastBlendFactor[0] == nextBlendFactor[0]
		|| currentBlendFactor[1] != lastBlendFactor[1] && lastBlendFactor[1] == nextBlendFactor[1]
		|| currentBlendFactor[2] != lastBlendFactor[2] && lastBlendFactor[2] == nextBlendFactor[2]
		|| currentBlendFactor[3] != lastBlendFactor[3] && lastBlendFactor[3] == nextBlendFactor[3]
		|| currentSampleMask != lastSampleMask && lastSampleMask == nextSampleMask
		)
	{
		printf("WARNING, blend state will not be set, because the last cached state and the desired state equals, but the current state differs!\n\n");
		char last[200];
		char curr[200];
		char diff[200];

		sprintf(last,"%d, %d, %d, %d, %d, %d, %d, %08x, %d, %d, %d, %d, %08x, %d, %d", lastBlendDesc.RenderTarget[0].BlendEnable,
			lastBlendDesc.RenderTarget[0].BlendOp, lastBlendDesc.RenderTarget[0].SrcBlend, lastBlendDesc.RenderTarget[0].DestBlend,
			lastBlendDesc.RenderTarget[0].BlendOpAlpha, lastBlendDesc.RenderTarget[0].SrcBlendAlpha, lastBlendDesc.RenderTarget[0].DestBlendAlpha,
			lastBlendDesc.RenderTarget[0].RenderTargetWriteMask, lastBlendFactor[0], lastBlendFactor[1], lastBlendFactor[2], lastBlendFactor[3],
			lastSampleMask, lastBlendDesc.AlphaToCoverageEnable, lastBlendDesc.IndependentBlendEnable);
		sprintf(curr,"%d, %d, %d, %d, %d, %d, %d, %08x, %d, %d, %d, %d, %08x, %d, %d", currentBlendDesc.RenderTarget[0].BlendEnable,
			currentBlendDesc.RenderTarget[0].BlendOp, currentBlendDesc.RenderTarget[0].SrcBlend, currentBlendDesc.RenderTarget[0].DestBlend,
			currentBlendDesc.RenderTarget[0].BlendOpAlpha, currentBlendDesc.RenderTarget[0].SrcBlendAlpha, currentBlendDesc.RenderTarget[0].DestBlendAlpha,
			currentBlendDesc.RenderTarget[0].RenderTargetWriteMask, currentBlendFactor[0], currentBlendFactor[1], currentBlendFactor[2], currentBlendFactor[3],
			currentSampleMask, currentBlendDesc.AlphaToCoverageEnable, currentBlendDesc.IndependentBlendEnable);
		createDiffStr(last, curr, diff);
		printf("Last = %s\nCurr = %s\n       %s\n\n", last, curr, diff);
	}
}

void CompareDepth(CD3D11_DEPTH_STENCIL_DESC &currentDepthStencilDesc,
	CD3D11_DEPTH_STENCIL_DESC &lastDepthStencilDesc,
	CD3D11_DEPTH_STENCIL_DESC &nextDepthStencilDesc
	)
{
	if (!(currentDepthStencilDesc == lastDepthStencilDesc) && lastDepthStencilDesc == nextDepthStencilDesc)
	{
		printf("WARNING, depth-stencil state will not be set, because the last cached state and the desired state equals, but the current state differs!\n\n");
		char last[200];
		char curr[200];
		char diff[200];

		sprintf(last, "%d, %d, %08x, %08x, %08x, %d, %d, %d, %d, %d, %d, %d, %d, %d", lastDepthStencilDesc.DepthEnable, lastDepthStencilDesc.StencilEnable,
			lastDepthStencilDesc.StencilWriteMask, lastDepthStencilDesc.StencilReadMask,
			lastDepthStencilDesc.DepthWriteMask, lastDepthStencilDesc.DepthFunc,
			lastDepthStencilDesc.FrontFace.StencilFunc, lastDepthStencilDesc.FrontFace.StencilPassOp,
			lastDepthStencilDesc.FrontFace.StencilFailOp, lastDepthStencilDesc.FrontFace.StencilDepthFailOp,
			lastDepthStencilDesc.BackFace.StencilFunc, lastDepthStencilDesc.BackFace.StencilPassOp,
			lastDepthStencilDesc.BackFace.StencilFailOp, lastDepthStencilDesc.BackFace.StencilDepthFailOp
			);
		sprintf(curr, "%d, %d, %08x, %08x, %08x, %d, %d, %d, %d, %d, %d, %d, %d, %d", currentDepthStencilDesc.DepthEnable, currentDepthStencilDesc.StencilEnable,
			currentDepthStencilDesc.StencilWriteMask, currentDepthStencilDesc.StencilReadMask,
			currentDepthStencilDesc.DepthWriteMask, currentDepthStencilDesc.DepthFunc,
			currentDepthStencilDesc.FrontFace.StencilFunc, currentDepthStencilDesc.FrontFace.StencilPassOp,
			currentDepthStencilDesc.FrontFace.StencilFailOp, currentDepthStencilDesc.FrontFace.StencilDepthFailOp,
			currentDepthStencilDesc.BackFace.StencilFunc, currentDepthStencilDesc.BackFace.StencilPassOp,
			currentDepthStencilDesc.BackFace.StencilFailOp, currentDepthStencilDesc.BackFace.StencilDepthFailOp
			);

		createDiffStr(last, curr, diff);
		printf("Last = %s\nCurr = %s\n       %s\n\n", last, curr, diff);
	}
}

void CompareRaster(CD3D11_RASTERIZER_DESC &currentRasterizerDesc,
	CD3D11_RASTERIZER_DESC &lastRasterizerDesc,
	CD3D11_RASTERIZER_DESC &nextRasterizerDesc
	)
{
	if (!(currentRasterizerDesc == lastRasterizerDesc) && lastRasterizerDesc == nextRasterizerDesc)
	{
		printf("WARNING, rasterizer state will not be set, because the last cached state and the desired state equals, but the current state differs!\n\n");
		char last[200];
		char curr[200];
		char diff[200];

		sprintf(last, "%d, %d, %d, %f, %d, %d, %d, %d, %d, %f", lastRasterizerDesc.AntialiasedLineEnable, lastRasterizerDesc.CullMode, lastRasterizerDesc.DepthBias, lastRasterizerDesc.DepthBiasClamp,
			lastRasterizerDesc.DepthClipEnable, lastRasterizerDesc.FillMode, lastRasterizerDesc.FrontCounterClockwise, lastRasterizerDesc.MultisampleEnable,
			lastRasterizerDesc.ScissorEnable, lastRasterizerDesc.SlopeScaledDepthBias);
		sprintf(curr, "%d, %d, %d, %f, %d, %d, %d, %d, %d, %f", currentRasterizerDesc.AntialiasedLineEnable, currentRasterizerDesc.CullMode, currentRasterizerDesc.DepthBias, currentRasterizerDesc.DepthBiasClamp,
			currentRasterizerDesc.DepthClipEnable, currentRasterizerDesc.FillMode, currentRasterizerDesc.FrontCounterClockwise, currentRasterizerDesc.MultisampleEnable,
			currentRasterizerDesc.ScissorEnable, currentRasterizerDesc.SlopeScaledDepthBias);

		createDiffStr(last, curr, diff);
		printf("Last = %s\nCurr = %s\n       %s\n\n", last, curr, diff);
	}
}

void CompareViewport(CD3D11_VIEWPORT  &currentViewport,
	CD3D11_VIEWPORT &lastViewport,
	CD3D11_VIEWPORT &nextViewport
	)
{
	if (!(currentViewport == lastViewport) && lastViewport == nextViewport)
	{
		printf("WARNING, viewport will not be set, because the last cached viewport and the desired viewport equals, but the current viewport differs!\n\n");
		char last[200];
		char curr[200];
		char diff[200];

		sprintf(last, "%d %d %d %d %f %f", lastViewport.TopLeftX, lastViewport.TopLeftY, lastViewport.Width, lastViewport.Height, lastViewport.MinDepth, lastViewport.MaxDepth);
		sprintf(curr, "%d %d %d %d %f %f", currentViewport.TopLeftX, currentViewport.TopLeftY, currentViewport.Width, currentViewport.Height, currentViewport.MinDepth, currentViewport.MaxDepth);

		createDiffStr(last, curr, diff);
		printf("Last = %s\nCurr = %s\n       %s\n\n", last, curr, diff);
	}
}
#endif


void StateManager::ApplyStateBlend(bool forced)
{
	States &desired = m_state_stack.top();

#if defined _DEBUG && defined REPORT_STATE_MANAGER_OVERRIDE
	{
		ID3D11BlendState *bState = NULL;
		FLOAT blendfac[4];
		UINT samplemask;
		DX::getContext()->OMGetBlendState(&bState, blendfac, &samplemask);
		CD3D11_BLEND_DESC bDesc = defaultBlendDesc;
		if (bState != NULL)
		{
			bState->GetDesc(&bDesc);
		}

		if (!forced) CompareBlend(bDesc, blendfac, samplemask, m_last_state.m_blend, m_last_state.m_blend_factor, m_last_state.m_blend_samplemask, desired.m_blend, desired.m_blend_factor, desired.m_blend_samplemask);

	}
#endif

	if ( 
		forced ||
		!( 
			desired.m_blend            == m_last_state.m_blend            &&
			desired.m_blend_factor[0]  == m_last_state.m_blend_factor[0]  &&
			desired.m_blend_factor[1]  == m_last_state.m_blend_factor[1]  &&
			desired.m_blend_factor[2]  == m_last_state.m_blend_factor[2]  &&
			desired.m_blend_factor[3]  == m_last_state.m_blend_factor[3]  &&
			desired.m_blend_samplemask == m_last_state.m_blend_samplemask
		)
		)
	{
		ID3D11BlendState *bState = NULL;
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateBlendState(&desired.m_blend,&bState)
			);

		DX::getContext()->OMSetBlendState(bState, desired.m_blend_factor, desired.m_blend_samplemask);
		bState->Release();

		m_last_state.m_blend            = desired.m_blend;
		m_last_state.m_blend_factor[0]  = desired.m_blend_factor[0];
		m_last_state.m_blend_factor[1]  = desired.m_blend_factor[1];
		m_last_state.m_blend_factor[2]  = desired.m_blend_factor[2];
		m_last_state.m_blend_factor[3]  = desired.m_blend_factor[3];
		m_last_state.m_blend_samplemask = desired.m_blend_samplemask;
	}
}

void StateManager::ApplyStateDepth(bool forced)
{
	States &desired = m_state_stack.top();

#if defined _DEBUG && defined REPORT_STATE_MANAGER_OVERRIDE
	{
		ID3D11DepthStencilState* dsState = NULL;
		UINT stencilRef;
		DX::getContext()->OMGetDepthStencilState(&dsState, &stencilRef);
		CD3D11_DEPTH_STENCIL_DESC dsDesc = defaultDepthStencilDesc;
		if (dsState != NULL)
		{
			dsState->GetDesc(&dsDesc);
		}

		if (!forced) CompareDepth(dsDesc, m_last_state.m_depth_stencil, desired.m_depth_stencil);
	}
#endif

	if (forced || !(desired.m_depth_stencil == m_last_state.m_depth_stencil))
	{
		ID3D11DepthStencilState* dsState = NULL;
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateDepthStencilState(&desired.m_depth_stencil, &dsState)
			);

		DX::getContext()->OMSetDepthStencilState(dsState, 1);
		dsState->Release();

		m_last_state.m_depth_stencil = desired.m_depth_stencil;
	}
}

void StateManager::ApplyStateRaster(bool forced)
{
	States &desired = m_state_stack.top();

#if defined _DEBUG && defined REPORT_STATE_MANAGER_OVERRIDE
	{
		ID3D11RasterizerState *rState;
		DX::getContext()->RSGetState(&rState);
		CD3D11_RASTERIZER_DESC rDesc = defaultRasterizerDesc;
		if (rState != NULL)
		{
			rState->GetDesc(&rDesc);
		}

		if (!forced) CompareRaster(rDesc, m_last_state.m_rasterizer, desired.m_rasterizer);
	}
#endif

	if (forced || ! (desired.m_rasterizer == m_last_state.m_rasterizer) )
	{
		ID3D11RasterizerState *rState = NULL;
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateRasterizerState( &desired.m_rasterizer, &rState )
			);

		DX::getContext()->RSSetState( rState );
		rState->Release();

		m_last_state.m_rasterizer = desired.m_rasterizer;
	}
}

void StateManager::ApplyStateViewport(bool forced)
{
	States &desired = m_state_stack.top();
	
#if defined _DEBUG && defined REPORT_STATE_MANAGER_OVERRIDE
	{
		CD3D11_VIEWPORT currentViewport;
		UINT viewports = 1;
		DX::getContext()->RSGetViewports(&viewports, &currentViewport);

		if (!forced) CompareViewport(currentViewport, m_last_state.m_viewport, desired.m_viewport);
	}
#endif

	if (forced || !(desired.m_viewport == m_last_state.m_viewport))
	{
		DX::getContext()->RSSetViewports(1, &desired.m_viewport);

		m_last_state.m_viewport = desired.m_viewport;
	}
}

void StateManager::Push()
{
	m_state_stack.push( m_state_stack.top() );
	//Same as current, no apply
}

void StateManager::Push(const States &states)
{
	m_state_stack.push( m_state_stack.top() );
	ApplyStates();
}

void StateManager::Pop()
{
	if (m_state_stack.size()>1)
		m_state_stack.pop();

	ApplyStates();
}

void StateManager::SetBlendEnabled(BOOL Enabled)
{
	States &desired = m_state_stack.top();

	desired.m_blend.RenderTarget[0].BlendEnable = Enabled;
}

void StateManager::SetBlendFunction(D3D11_BLEND SrcBlend, D3D11_BLEND DestBlend, D3D11_BLEND_OP BlendOp, D3D11_BLEND SrcBlendAlpha, D3D11_BLEND DestBlendAlpha, D3D11_BLEND_OP BlendOpAlpha)
{
	States &desired = m_state_stack.top();

	desired.m_blend.RenderTarget[0].SrcBlend       = SrcBlend;
	desired.m_blend.RenderTarget[0].DestBlend      = DestBlend;
	desired.m_blend.RenderTarget[0].BlendOp        = BlendOp;
	desired.m_blend.RenderTarget[0].SrcBlendAlpha  = SrcBlendAlpha;
	desired.m_blend.RenderTarget[0].DestBlendAlpha = DestBlendAlpha;
	desired.m_blend.RenderTarget[0].BlendOpAlpha   = BlendOpAlpha;
}

void StateManager::SetBlendFactor(FLOAT *BlendFactor)
{
	States &desired = m_state_stack.top();
	
	desired.m_blend_factor[0] = BlendFactor[0];
	desired.m_blend_factor[1] = BlendFactor[1];
	desired.m_blend_factor[2] = BlendFactor[2];
	desired.m_blend_factor[3] = BlendFactor[3];
}

void StateManager::SetBlendColorMask(UINT8 Mask)
{
	States &desired = m_state_stack.top();
	
	desired.m_blend.RenderTarget[0].RenderTargetWriteMask = Mask;
}

void StateManager::SetBlendSampleMask(UINT8 Mask)
{
	States &desired = m_state_stack.top();
	
	desired.m_blend_samplemask = Mask;
}
		
void StateManager::SetDepthEnabled(BOOL Enabled)
{
	States &desired = m_state_stack.top();
	
	desired.m_depth_stencil.DepthEnable = Enabled;
}

void StateManager::SetDepthFunction(D3D11_COMPARISON_FUNC DepthFunc)
{
	States &desired = m_state_stack.top();

	desired.m_depth_stencil.DepthFunc = DepthFunc;
}

void StateManager::SetDepthMask(D3D11_DEPTH_WRITE_MASK DepthWriteMask)
{
	States &desired = m_state_stack.top();

	desired.m_depth_stencil.DepthWriteMask = DepthWriteMask;
}

void StateManager::SetRasterFrontClockwise(BOOL Clockwise)
{
	States &desired = m_state_stack.top();

	desired.m_rasterizer.FrontCounterClockwise = ! Clockwise;
}

void StateManager::SetRasterCullMode(D3D11_CULL_MODE CullMode)
{
	States &desired = m_state_stack.top();

	desired.m_rasterizer.CullMode = CullMode;
}

void StateManager::SetViewport(const CD3D11_VIEWPORT &ViewPort)
{
	States &desired = m_state_stack.top();

	desired.m_viewport = ViewPort;
}

void StateManager::SetViewportNoDepth(const CD3D11_VIEWPORT &ViewPort)
{
	States &desired = m_state_stack.top();

	desired.m_viewport.TopLeftX = ViewPort.TopLeftX;
	desired.m_viewport.TopLeftY = ViewPort.TopLeftY;
	desired.m_viewport.Width    = ViewPort.Width;
	desired.m_viewport.Height   = ViewPort.Height;
}

void StateManager::SetViewportDepthrange(float MinDepth, float MaxDepth)
{
	States &desired = m_state_stack.top();

	desired.m_viewport.MinDepth = MinDepth;
	desired.m_viewport.MaxDepth = MaxDepth;
}

#else

StateManager::StateManager()
{
	FLOAT blend[]={0.0f,0.0f,0.0f,0.0f};
	SetBlendFactor(blend);
	ID3D11DepthStencilState* currentDepthStencilState;
	UINT currentStencilRef;

	DX::getContext()->OMGetDepthStencilState(&currentDepthStencilState,&currentStencilRef);
	DX::getContext()->OMSetDepthStencilState(currentDepthStencilState,1);
	if (currentDepthStencilState != NULL) currentDepthStencilState->Release();
}

void StateManager::ApplyStates(bool)
{
}

void StateManager::Push()
{
	UINT viewportnum = 1;
	InternalStateDesc currentState;

	DX::getContext()->OMGetBlendState(&currentState.blendState, currentState.blendFactor, &currentState.sampleMask);
	DX::getContext()->OMGetDepthStencilState(&currentState.depthStencilState,&currentState.stencilRef);
	DX::getContext()->RSGetState(&currentState.rasterizerState);
	DX::getContext()->RSGetViewports(&viewportnum,&currentState.viewportState);
	m_state_stack.push(currentState);
}

void StateManager::Push(const States &states)
{
	Push();
	ID3D11BlendState *blendState;
	DX::getDevice()->CreateBlendState(&states.m_blend,&blendState);
	DX::getContext()->OMSetBlendState(blendState,states.m_blend_factor,states.m_blend_samplemask);
	blendState->Release();
	ID3D11DepthStencilState *depthStencilState;
	DX::getDevice()->CreateDepthStencilState(&states.m_depth_stencil,&depthStencilState);
	DX::getContext()->OMSetDepthStencilState(depthStencilState,1);
	depthStencilState->Release();
	ID3D11RasterizerState *rasterizerState;
	DX::getDevice()->CreateRasterizerState(&states.m_rasterizer,&rasterizerState);
	DX::getContext()->RSSetState(rasterizerState);
	rasterizerState->Release();
	DX::getContext()->RSSetViewports(1,&states.m_viewport);
}

void StateManager::Pop()
{
	InternalStateDesc top = m_state_stack.top();
	m_state_stack.pop();
	DX::getContext()->OMSetBlendState(top.blendState,top.blendFactor,top.sampleMask);
	top.blendState->Release();
	DX::getContext()->OMSetDepthStencilState(top.depthStencilState,top.stencilRef);
	top.depthStencilState->Release();
	DX::getContext()->RSSetState(top.rasterizerState);
	top.rasterizerState->Release();
	DX::getContext()->RSSetViewports(1,&top.viewportState);
}

void StateManager::SetBlendEnabled(BOOL Enabled)
{
	ID3D11BlendState* currentBlendState;
	D3D11_BLEND_DESC currentBlendDesc;
	FLOAT currentBlendFactor[4];
	UINT currentSampleMask;

	DX::getContext()->OMGetBlendState(&currentBlendState,currentBlendFactor,&currentSampleMask);
	if (currentBlendState!=NULL)
	{
		currentBlendState->GetDesc(&currentBlendDesc);
		currentBlendState->Release();
	}
	else {
		currentBlendDesc = defaultBlendDesc;
	}
	currentBlendDesc.RenderTarget[0].BlendEnable= Enabled;
	DX::getDevice()->CreateBlendState(&currentBlendDesc,&currentBlendState);
	DX::getContext()->OMSetBlendState(currentBlendState,currentBlendFactor,currentSampleMask);
	currentBlendState->Release();
}

void StateManager::SetBlendFunction(D3D11_BLEND SrcBlend, D3D11_BLEND DestBlend, D3D11_BLEND_OP BlendOp, D3D11_BLEND SrcBlendAlpha, D3D11_BLEND DestBlendAlpha, D3D11_BLEND_OP BlendOpAlpha)
{
	ID3D11BlendState* currentBlendState;
	D3D11_BLEND_DESC currentBlendDesc;
	FLOAT currentBlendFactor[4];
	UINT currentSampleMask;

	DX::getContext()->OMGetBlendState(&currentBlendState,currentBlendFactor,&currentSampleMask);
	if (currentBlendState!=NULL)
	{
		currentBlendState->GetDesc(&currentBlendDesc);
		currentBlendState->Release();
	}
	else {
		currentBlendDesc = defaultBlendDesc;
	}
	currentBlendDesc.RenderTarget[0].SrcBlend = SrcBlend;
	currentBlendDesc.RenderTarget[0].DestBlend = DestBlend;
	currentBlendDesc.RenderTarget[0].BlendOp = BlendOp;
	currentBlendDesc.RenderTarget[0].SrcBlendAlpha = SrcBlendAlpha;
	currentBlendDesc.RenderTarget[0].DestBlendAlpha = DestBlendAlpha;
	currentBlendDesc.RenderTarget[0].BlendOpAlpha = BlendOpAlpha;
	DX::getDevice()->CreateBlendState(&currentBlendDesc,&currentBlendState);
	DX::getContext()->OMSetBlendState(currentBlendState,currentBlendFactor,currentSampleMask);
	currentBlendState->Release();
}

void StateManager::SetBlendFactor(FLOAT *BlendFactor)
{
	ID3D11BlendState* currentBlendState;
	FLOAT currentBlendFactor[4];
	UINT currentSampleMask;

	DX::getContext()->OMGetBlendState(&currentBlendState,currentBlendFactor,&currentSampleMask);
	DX::getContext()->OMSetBlendState(currentBlendState,BlendFactor,currentSampleMask);
	if (currentBlendState != NULL) currentBlendState->Release();
}

void StateManager::SetBlendColorMask(UINT8 Mask)
{
	ID3D11BlendState* currentBlendState;
	D3D11_BLEND_DESC currentBlendDesc;
	FLOAT currentBlendFactor[4];
	UINT currentSampleMask;

	DX::getContext()->OMGetBlendState(&currentBlendState,currentBlendFactor,&currentSampleMask);
	if (currentBlendState!=NULL)
	{
		currentBlendState->GetDesc(&currentBlendDesc);
		currentBlendState->Release();
	}
	else {
		currentBlendDesc = defaultBlendDesc;
	}
	currentBlendDesc.RenderTarget[0].RenderTargetWriteMask = Mask;
	DX::getDevice()->CreateBlendState(&currentBlendDesc,&currentBlendState);
	DX::getContext()->OMSetBlendState(currentBlendState,currentBlendFactor,currentSampleMask);
	currentBlendState->Release();
}

void StateManager::SetBlendSampleMask(UINT8 Mask)
{
	ID3D11BlendState* currentBlendState;
	FLOAT currentBlendFactor[4];
	UINT currentSampleMask;

	DX::getContext()->OMGetBlendState(&currentBlendState,currentBlendFactor,&currentSampleMask);
	DX::getContext()->OMSetBlendState(currentBlendState,currentBlendFactor,Mask);
	if (currentBlendState != NULL) currentBlendState->Release();
}

void StateManager::SetDepthEnabled(BOOL Enabled)
{
	ID3D11DepthStencilState* currentDepthStencilState;
	D3D11_DEPTH_STENCIL_DESC currentDepthStencilDesc;
	UINT currentStencilRef;

	DX::getContext()->OMGetDepthStencilState(&currentDepthStencilState,&currentStencilRef);
	if (currentDepthStencilState!=NULL)
	{
		currentDepthStencilState->GetDesc(&currentDepthStencilDesc);
		currentDepthStencilState->Release();
	}
	else {
		currentDepthStencilDesc = defaultDepthStencilDesc;
	}
	currentDepthStencilDesc.DepthEnable = Enabled;
	DX::getDevice()->CreateDepthStencilState(&currentDepthStencilDesc,&currentDepthStencilState);
	DX::getContext()->OMSetDepthStencilState(currentDepthStencilState,currentStencilRef);
	currentDepthStencilState->Release();
}

void StateManager::SetDepthFunction(D3D11_COMPARISON_FUNC DepthFunc)
{
	ID3D11DepthStencilState* currentDepthStencilState;
	D3D11_DEPTH_STENCIL_DESC currentDepthStencilDesc;
	UINT currentStencilRef;

	DX::getContext()->OMGetDepthStencilState(&currentDepthStencilState,&currentStencilRef);
	if (currentDepthStencilState!=NULL)
	{
		currentDepthStencilState->GetDesc(&currentDepthStencilDesc);
		currentDepthStencilState->Release();
	}
	else {
		currentDepthStencilDesc = defaultDepthStencilDesc;
	}
	currentDepthStencilDesc.DepthFunc = DepthFunc;
	DX::getDevice()->CreateDepthStencilState(&currentDepthStencilDesc,&currentDepthStencilState);
	DX::getContext()->OMSetDepthStencilState(currentDepthStencilState,currentStencilRef);
	currentDepthStencilState->Release();
}

void StateManager::SetDepthMask(D3D11_DEPTH_WRITE_MASK DepthWriteMask)
{
	ID3D11DepthStencilState* currentDepthStencilState;
	D3D11_DEPTH_STENCIL_DESC currentDepthStencilDesc;
	UINT currentStencilRef;

	DX::getContext()->OMGetDepthStencilState(&currentDepthStencilState,&currentStencilRef);
	if (currentDepthStencilState!=NULL)
	{
		currentDepthStencilState->GetDesc(&currentDepthStencilDesc);
		currentDepthStencilState->Release();
	}
	else {
		currentDepthStencilDesc = defaultDepthStencilDesc;
	}
	currentDepthStencilDesc.DepthWriteMask = DepthWriteMask;
	DX::getDevice()->CreateDepthStencilState(&currentDepthStencilDesc,&currentDepthStencilState);
	DX::getContext()->OMSetDepthStencilState(currentDepthStencilState,currentStencilRef);
	currentDepthStencilState->Release();
}

void StateManager::SetRasterFrontClockwise(BOOL Clockwise)
{
	ID3D11RasterizerState* currentRasterizerState;
	D3D11_RASTERIZER_DESC currentRasterizerDesc;

	DX::getContext()->RSGetState(&currentRasterizerState);
	if (currentRasterizerState!=NULL)
	{
		currentRasterizerState->GetDesc(&currentRasterizerDesc);
		currentRasterizerState->Release();
	}
	else {
		currentRasterizerDesc = defaultRasterizerDesc;
	}
	currentRasterizerDesc.FrontCounterClockwise = ! Clockwise;
	DX::getDevice()->CreateRasterizerState(&currentRasterizerDesc,&currentRasterizerState);
	DX::getContext()->RSSetState(currentRasterizerState);
	currentRasterizerState->Release();
}

void StateManager::SetRasterCullMode(D3D11_CULL_MODE CullMode)
{
	ID3D11RasterizerState* currentRasterizerState;
	D3D11_RASTERIZER_DESC currentRasterizerDesc;

	DX::getContext()->RSGetState(&currentRasterizerState);
	if (currentRasterizerState!=NULL)
	{
		currentRasterizerState->GetDesc(&currentRasterizerDesc);
		currentRasterizerState->Release();
	}
	else {
		currentRasterizerDesc = defaultRasterizerDesc;
	}
	currentRasterizerDesc.CullMode = CullMode;
	DX::getDevice()->CreateRasterizerState(&currentRasterizerDesc,&currentRasterizerState);
	DX::getContext()->RSSetState(currentRasterizerState);
	currentRasterizerState->Release();
}

void StateManager::SetViewport(const CD3D11_VIEWPORT &ViewPort)
{
	DX::getContext()->RSSetViewports(1,&ViewPort);
}

void StateManager::SetViewportNoDepth(const CD3D11_VIEWPORT &ViewPort)
{
	UINT viewportNum = 1;
	D3D11_VIEWPORT currentViewport;
	DX::getContext()->RSGetViewports(&viewportNum,&currentViewport);
	currentViewport.TopLeftX = ViewPort.TopLeftX;
	currentViewport.TopLeftY = ViewPort.TopLeftY;
	currentViewport.Width = ViewPort.Width;
	currentViewport.Height = ViewPort.Height;
	DX::getContext()->RSSetViewports(1,&currentViewport);
}

void StateManager::SetViewportDepthrange(float MinDepth, float MaxDepth)
{
	UINT viewportNum = 1;
	D3D11_VIEWPORT currentViewport;
	DX::getContext()->RSGetViewports(&viewportNum,&currentViewport);
	currentViewport.MinDepth = MinDepth;
	currentViewport.MaxDepth = MaxDepth;
	DX::getContext()->RSSetViewports(1,&currentViewport);
}

#endif