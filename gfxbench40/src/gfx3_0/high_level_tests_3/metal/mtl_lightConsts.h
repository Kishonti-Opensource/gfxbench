/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
struct lightConsts
{
	vec4 light_colorXYZ_pad;	
	vec4 light_posXYZ_pad;	
	vec4 light_x;
	vec4 spotcosXY_attenZ_pad;
 	mat4 shadowMatrix0;
};
