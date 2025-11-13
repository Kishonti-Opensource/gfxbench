/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
struct frameConsts
{
	vec4 depth_parameters;
	vec4 global_light_dirXYZ_pad;
	vec4 global_light_colorXYZ_pad;
	vec4 view_dirXYZ_pad;
	vec4 view_posXYZ_time;
	vec4 background_colorXYZ_fogdensity;
	vec4 inv_resolutionXY_pad;
};