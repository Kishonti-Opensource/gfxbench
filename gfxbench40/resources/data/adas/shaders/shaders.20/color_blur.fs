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
	vec4 c = texture2D( texture_unit0, out_texcoord0);
	c += texture2D( texture_unit0, out_texcoord1);
	c += texture2D( texture_unit0, out_texcoord2);
	c *= 0.3333333;

	gl_FragColor = c;
}
