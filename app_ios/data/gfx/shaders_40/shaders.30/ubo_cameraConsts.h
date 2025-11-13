/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
layout(std140) uniform cameraConsts
{
	uniform highp	vec4 depth_parameters;
	uniform highp 	vec4 view_dirXYZ_pad;
	uniform highp 	vec4 view_posXYZ_normalized_time;
	uniform highp	vec4 inv_resolutionXY_pad;
	uniform highp	mat4 vp;
};