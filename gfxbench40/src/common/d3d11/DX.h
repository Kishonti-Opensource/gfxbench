/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once

#include <d3d11_1.h>
#include <memory>
#include <wrl/client.h>

#include "dxb_state_manager.h"
#include "graphics\dxgraphicscontext.h"

namespace DX
{
	bool Attach(DXGraphicsContext* context);
	void Release();

	void Clear();
	void Clear(float r, float g, float b, float a);
	void DiscardDepth();
	void Discard();
    void BindDefaultRenderTargetView();
	
	void Flush();
		
	bool isContextAttached();
	ID3D11Device1* getDevice();
	ID3D11DeviceContext1* getContext();
	DXGraphicsContext* getGraphicsContext();
	float getWidth();
	float getHeight();
	void swapBuffers();
	StateManager* getStateManager();
}
