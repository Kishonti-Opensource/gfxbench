/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
layout(std140) uniform lightShaftConsts
{
	uniform highp		mat4 mvp;
	uniform highp		mat4 mv;
	uniform highp		mat4 shadow_matrix0;
	
	uniform mediump		vec4 light_color_pad;
	uniform mediump		vec4 light_pos_pad;
	uniform mediump		vec4 light_x;
	uniform mediump		vec4 spot_cos_attenuation_parameter_pad;
}; 