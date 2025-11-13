/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NGL_D3D12_ACTION_H
#define NGL_D3D12_ACTION_H

enum D3D12_Action
{
	NGL_D3D12_ACTION_NO_ACTION = 0,

	NGL_D3D12_ACTION_SET_CLEAR_RED = 203,
	NGL_D3D12_ACTION_SET_CLEAR_GREEN = 204,
	NGL_D3D12_ACTION_SET_CLEAR_BLUE = 205,

	NGL_D3D12_ACTION_RESIZE = 235173,

	NGL_D3D12_ACTION_UNRESERVED
};

#endif
