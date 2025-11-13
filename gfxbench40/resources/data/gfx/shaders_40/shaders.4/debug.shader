/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#ifdef TYPE_vertex
uniform mat4 vp;

in vec3 in_position;
in vec4 in_color;

out vec4 v_color;
void main()
{	
	v_color = in_color;
	gl_Position = vp * vec4(in_position, 1.0);
}
#endif

#ifdef TYPE_fragment
in vec4 v_color;
out vec4 frag_color;

void main()
{	
	frag_color = v_color;
}

#endif
