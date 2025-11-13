/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "DX.h"
#include "DXUtils.h"
#include <ppltasks.h>

#include "d3d11/shader.h"
#include "d3d11/vbopool.h"
#include "dxb_state_manager.h"

//#define USE_WRAPPER

#if defined USE_WRAPPER
#include "D3D11DeviceContextWrapper.h"
#include "D3D11DeviceWrapper.h"
#endif
#ifdef _DEBUG
#define CHECK_CONTEXT_ATTACHED	assert(NULL != g_dxGraphicsContext)
#else
#define CHECK_CONTEXT_ATTACHED
#endif

namespace DX
{
	DXGraphicsContext *g_dxGraphicsContext;
	static std::auto_ptr<StateManager> g_stateManager;
#if defined USE_WRAPPER
	static std::auto_ptr<D3D11DeviceContext1Wrapper> g_dxGraphicsContextWrapper;
	static std::auto_ptr<D3D11Device1Wrapper> g_dxGraphicsDeviceWrapper;
#endif

	bool Attach(DXGraphicsContext* context)
	{
		Release();
		
		g_dxGraphicsContext = context;

#if defined USE_WRAPPER
		g_dxGraphicsContextWrapper.reset(D3D11DeviceContext1Wrapper::Wrap(g_dxGraphicsContext->getD3D11DeviceContext()));
		g_dxGraphicsDeviceWrapper.reset(D3D11Device1Wrapper::Wrap(g_dxGraphicsContext->getD3D11Device()));
#endif

		StateManager* manager = new StateManager();
		manager->SetBlendEnabled(false);
		manager->SetDepthFunction(D3D11_COMPARISON_LESS);
		manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ALL);
		manager->SetRasterFrontClockwise(true);
		manager->SetRasterCullMode(D3D11_CULL_NONE);
		manager->SetViewportDepthrange(0.0f,1.0f);
		manager->ApplyStates(true);

		g_stateManager.reset(manager);

		return true;
	}

	void Release()
	{
		if (isContextAttached())
		{
			ID3D11RenderTargetView* nullViews[] = {nullptr};
			getContext()->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
		
			Shader::DeleteShaders();
			VboPool::DeleteInstance();
			IndexBufferPool::DeleteInstance();

			getContext()->ClearState();
			//data->m_d3dContext->Flush();
			Flush();

#if defined USE_WRAPPER
			g_dxGraphicsContextWrapper->Unwrap();
			g_dxGraphicsDeviceWrapper->Unwrap();
#endif

			g_dxGraphicsContext = NULL;
		}
	}

	void Clear(float r, float g, float b, float a)
	{
		getStateManager()->SetBlendEnabled(false);

		const float color[] = { r, g, b, a };
		getContext()->ClearRenderTargetView(
			g_dxGraphicsContext->getD3D11RenderTargetView(),
			color
			);

		getContext()->ClearDepthStencilView(
			g_dxGraphicsContext->getD3D11DepthStencilView(),
			D3D11_CLEAR_DEPTH,
			1.0f,
			0
			);
	}

	void Clear()
	{
		//Clear(0.098f, 0.098f, 0.439f, 1.000f);
		Clear(0.0f, 0.0f, 0.0f, 1.000f);
	}

	void DiscardDepth()
	{
		getContext()->DiscardView(g_dxGraphicsContext->getD3D11DepthStencilView());
	}

	void Discard()
	{
		getContext()->DiscardView(g_dxGraphicsContext->getD3D11RenderTargetView());
		getContext()->ClearDepthStencilView(
			g_dxGraphicsContext->getD3D11DepthStencilView(),
			D3D11_CLEAR_DEPTH,
			1.0f,
			0
			);
	}

	void Flush()
	{
		if (!isContextAttached())
		{
			return;
		}

		D3D11_QUERY_DESC queryDesc = {D3D11_QUERY_EVENT, 0};
		ID3D11Query* pQuery;

		if (SUCCEEDED(getDevice()->CreateQuery(&queryDesc, &pQuery)))
		{
			getContext()->End(pQuery);
		}
		else
		{
			pQuery = NULL;
		}

		getContext()->Flush();

		if (pQuery)
		{
			HRESULT hr = getContext()->GetData(pQuery, NULL, 0, 0);
            if(hr != S_OK)
			{
				// Poll on the query, no need to flush the pipeline as we've flushed it above.
                while (getContext()->GetData(pQuery, 0, NULL, D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK);
			}
			pQuery->Release();
		}
	}

	void BindDefaultRenderTargetView()
	{
		CHECK_CONTEXT_ATTACHED;

		ID3D11RenderTargetView* rtv = getGraphicsContext()->getD3D11RenderTargetView();
		ID3D11DepthStencilView* dsv = getGraphicsContext()->getD3D11DepthStencilView();
		D3D11_VIEWPORT *vp = getGraphicsContext()->getDefaultViewport();

		getContext()->OMSetRenderTargets(1, &rtv, dsv );
		DX::getStateManager()->SetViewport(CD3D11_VIEWPORT(*vp));
	}

	bool isContextAttached()
	{
		return NULL != g_dxGraphicsContext;
	}
	
	ID3D11Device1* getDevice()
	{
		CHECK_CONTEXT_ATTACHED;
#if defined USE_WRAPPER
		return g_dxGraphicsDeviceWrapper.get();
#else
		return g_dxGraphicsContext->getD3D11Device();
#endif
	}

	ID3D11DeviceContext1* getContext()
	{
		CHECK_CONTEXT_ATTACHED;
#if defined USE_WRAPPER
		return g_dxGraphicsContextWrapper.get();
#else
		return g_dxGraphicsContext->getD3D11DeviceContext();
#endif
	}

	DXGraphicsContext* getGraphicsContext()
	{
		return g_dxGraphicsContext;
	}

	StateManager* getStateManager()
	{
		return g_stateManager.get();
	}

	float getWidth()
	{
		return isContextAttached() ? g_dxGraphicsContext->getDefaultViewport()->Width : 0;
	}

	float getHeight()
	{
		return isContextAttached() ? g_dxGraphicsContext->getDefaultViewport()->Height : 0;
	}

	void swapBuffers()
	{
		CHECK_CONTEXT_ATTACHED;
		g_dxGraphicsContext->swapBuffers();
	}
}