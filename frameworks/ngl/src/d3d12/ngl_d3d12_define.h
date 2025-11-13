/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NGL_D3D12_DEFINE_H
#define NGL_D3D12_DEFINE_H

#include <cassert>
#include <mutex>
#include <map>
#include <exception>

#include "ngl.h"
#include "ngl_internal.h"

#include <D3D12/d3d12.h>
#include <D3D12/d3dx12/d3dx12.h>
#include "d3dcompiler.h"
#include "dxgi1_3.h"


#define ThrowIfFailed(hr) if (FAILED(hr)) throw std::exception()

template <class T> 
inline void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = nullptr;
	}
}
#define SAFE_RELEASE(X) SafeRelease(&X)


#define NGL_DX12_SYSTEM_ATTACHMENT 0
#define NGL_DX12_RESOURCE_STATE_UNKNOWN (D3D12_RESOURCE_STATES)-1
#define NGL_DX12_FENCE_VALUE_UNSET (uint64_t)-1
#define NGL_DX12_DESCRIPTOR_INDEX_UNSET (uint32_t)-1

#define NGL_DX12_SWAP_CHAIN_BUFFER_COUNT 3
#define NGL_DX12_SYNC_INTERVAL 0
#define NGL_DX12_SYNC_INTERVAL_VSYNC 1

//#define NGL_D3D12_ENABLE_DEBUG_LAYER 1

//#define NGL_DX12_USE_SOFTWARE_ADAPTER
//#define NGL_DX12_PRINT_SHADERS
//#define NGL_DX12_PRINT_ROOT_SIGNATURE
//#define NGL_DX12_VISUALIZE_BUFFER_SWAP
//#define NGL_DX12_DEBUG_TRANSITIONS
//#define NGL_DX12_PRINT_TRANSITIONS
//#define NGL_DX12_DEBUG_CONSTANT_BUFFER_UPLOAD
//#define NGL_DX12_DEBUG_PSO_CREATION
//#define NGL_DX12_DEBUG_LARGE_CBVS
//#define NGL_DX12_DEBUG_BUFFER_SUBRESOURCE_VIEWS

// Disables hardwired root constant threshold and calculates available
// space based on NGL_DX12_ROOT_ARGUMENT_SPACE_TARGET_SIZE
#define NGL_DX12_DYNAMIC_ROOT_CONSTANT_UPLOAD

// After populating the root signature, leftover space may be filled with root constants.
// 0 disables root constants. Cannot be larger than 64.
// Only effective when NGL_DX12_DYNAMIC_ROOT_CONSTANT_UPLOAD is defined
#define NGL_DX12_ROOT_ARGUMENT_SPACE_TARGET_SIZE 64

// Debug layer is not mandatory in debug mode, but it's a good default
#if _DEBUG && !defined(NGL_D3D12_ENABLE_DEBUG_LAYER)
	// Comment out / unset NGL_D3D12_ENABLE_DEBUG_LAYER if you want to debug without the debug layers
	#define NGL_D3D12_ENABLE_DEBUG_LAYER 1
#endif

#if NGL_D3D12_ENABLE_DEBUG_LAYER
	#include "DXGIDebug.h"
#endif

#endif
