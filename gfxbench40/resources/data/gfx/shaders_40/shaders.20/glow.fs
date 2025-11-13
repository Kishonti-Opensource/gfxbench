/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
varying vec2 out_texcoord0;
uniform sampler2D texture_unit0;
uniform vec3 color;

void main()
{   
	vec4 texel = texture2D( texture_unit0, out_texcoord0) * vec4( color, 1.0);
	gl_FragColor = vec4( texel.xyz, 0.0);
}

