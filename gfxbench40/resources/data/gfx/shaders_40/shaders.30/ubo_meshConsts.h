/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
layout(std140) uniform meshConsts
{
	uniform highp mat4 mvp;
	uniform highp mat4 mv;
	uniform highp mat4 inv_modelview;
}; 