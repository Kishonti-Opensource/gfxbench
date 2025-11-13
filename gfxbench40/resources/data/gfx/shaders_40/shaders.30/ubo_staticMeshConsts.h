/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
layout(std140) uniform staticMeshConsts
{
	uniform highp mat4 model;
	uniform highp mat4 inv_model;
	uniform lowp vec4 color_pad;
}; 