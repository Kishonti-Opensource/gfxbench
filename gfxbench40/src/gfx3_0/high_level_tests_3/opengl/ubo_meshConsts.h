/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
layout(std140) uniform meshConsts
{
	mat4 mvp;
	mat4 mv;
	mat4 model;
	mat4 inv_model;
	mat4 inv_modelview;
}; 