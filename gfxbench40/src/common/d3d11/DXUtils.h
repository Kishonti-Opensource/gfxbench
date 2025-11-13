/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once

#include <DirectXMath.h>
#include "platform.h"
#include <exception>

namespace DX
{
#ifdef _DEBUG

#define DXHELPER_STR_HELPER(x) #x
#define DXHELPER_STR(x) DXHELPER_STR_HELPER(x)
#define __VS_CODE_LOCATION__				__FUNCSIG__## " [" ##__FILE__##" line." ##DXHELPER_STR(__LINE__)## "]"
#define DX_THROW_IF_FAILED(expression)	{ \
	HRESULT __hr = (expression);	\
	if (FAILED(__hr)) {	\
	switch (__hr) {	\
	case DXGI_ERROR_DEVICE_REMOVED: {	\
			HRESULT removeReason = DX::getDevice()->GetDeviceRemovedReason(); \
			throw std::exception("Unexpected error code at: " __VS_CODE_LOCATION__); \
		} \
	default: \
		throw std::exception("Unexpected error code at: " __VS_CODE_LOCATION__); \
}}}\

#else

#define DX_THROW_IF_FAILED(expression)	{ HRESULT __hr = (expression); if (FAILED(__hr)) { throw std::exception("Unexpected error code at: " __FUNCSIG__); } } 

#endif

	inline DirectX::XMFLOAT4X4A Float4x4toXMFloat4x4ATransposed(const float* m4x4)
	{
		
		return DirectX::XMFLOAT4X4A( 
				m4x4[0], m4x4[4], m4x4[8],  m4x4[12],
				m4x4[1], m4x4[5], m4x4[9],  m4x4[13],
				m4x4[2], m4x4[6], m4x4[10], m4x4[14],
				m4x4[3], m4x4[7], m4x4[11], m4x4[15]
			);
	}

	inline DirectX::XMFLOAT4X4A Float3x3toXMFloat4x4ATransposed(float* m3x3)
	{
		return DirectX::XMFLOAT4X4A( 
				m3x3[0], m3x3[3], m3x3[6], 0,
				m3x3[1], m3x3[4], m3x3[7], 0,
				m3x3[2], m3x3[5], m3x3[8], 0,
				      0,       0,       0, 1
			);
	}

	inline DirectX::XMFLOAT4A Float3toXMFloat4A(float* v3)
	{
		return DirectX::XMFLOAT4A( 
				v3[0], v3[1], v3[2], 0
			);
	}
}