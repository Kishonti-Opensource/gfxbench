/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
varying vec2 out_texcoord0;
uniform sampler2D texture_unit0;


varying float out_life;
varying vec4 out_color;
varying vec2 world_texcoord;

void main()
{   
	vec3 texel = texture2D( texture_unit0, out_texcoord0).xyz;

    gl_FragColor.xyz = texel * out_color.xyz;
    gl_FragColor.w = texel.x * out_life;
}
