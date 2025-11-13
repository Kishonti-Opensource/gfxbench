/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D texture_unit0;

#ifdef TYPE_vertex

in vec2 in_position;
in vec2 in_texcoord;

out vec2 v_texcoord;

void main()
{	
	gl_Position = vec4(in_position.xy, 0.0, 1.0);
	v_texcoord = in_texcoord;
}

#endif


#ifdef TYPE_fragment

in vec2 v_texcoord;
out vec4 frag_color;

void main()
{	
	vec4 color = texture(texture_unit0, v_texcoord);
	frag_color = vec4(color);
}

#endif
