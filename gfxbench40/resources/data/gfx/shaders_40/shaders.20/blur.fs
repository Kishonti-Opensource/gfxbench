/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
varying vec2 out_texcoord0;
varying vec2 out_texcoord1;
varying vec2 out_texcoord2;

uniform sampler2D texture_unit0;


void main()
{   
	float c = texture2D( texture_unit0, out_texcoord0).x * 0.333333;
	c += texture2D( texture_unit0, out_texcoord1).x * 0.333333;
	c += texture2D( texture_unit0, out_texcoord2).x * 0.333333;

	gl_FragColor = vec4( c, c, c, 1.0);
}
