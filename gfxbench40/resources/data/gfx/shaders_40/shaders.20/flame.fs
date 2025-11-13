/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
varying vec2 out_texcoord0;
uniform sampler2D texture_unit0;
uniform sampler2D texture_unit1;
uniform float time;
uniform vec3 color;


void main()
{   
	vec2 noise = texture2D(texture_unit1, vec2( out_texcoord0.s, out_texcoord0.t + 100.0 * time)).rg;
	
	noise.s = noise.s * 2.0 - 1.0;
	
	vec2 flameCoord;

	flameCoord.s = out_texcoord0.s + noise.s * (1.0 / 8.0);
	flameCoord.t = out_texcoord0.t + noise.t * (1.0 / 32.0);


	gl_FragColor = texture2D( texture_unit0, flameCoord) * vec4( color, 1.0);
}

