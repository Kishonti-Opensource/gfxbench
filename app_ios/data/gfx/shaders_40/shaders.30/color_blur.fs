/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
in vec2 out_texcoord0;
in vec2 out_texcoord1;
in vec2 out_texcoord2;
out vec4 frag_color;

uniform sampler2D texture_unit0;


void main()
{   
	vec4 c = texture( texture_unit0, out_texcoord0);
	c += texture( texture_unit0, out_texcoord1);
	c += texture( texture_unit0, out_texcoord2);
	c *= 0.3333333;

	frag_color = c;
}
