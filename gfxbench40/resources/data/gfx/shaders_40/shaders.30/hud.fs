/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
in vec2 out_texcoord0;

uniform sampler2D texture_unit0;

in vec2 cicc;
out vec4 frag_color;

void main()
{   
	vec4 c0 = texture( texture_unit0, out_texcoord0);
	
	frag_color = c0 * vec4( 1.0);
}
