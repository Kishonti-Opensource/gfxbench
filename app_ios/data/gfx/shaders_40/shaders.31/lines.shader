/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 mvp;

#ifdef TYPE_vertex

in vec4 in_position;
out float intensity;

void main()
{	
	intensity = in_position.w;
	gl_Position = mvp * vec4(in_position.xyz, 1.0);
}

#endif

#ifdef TYPE_fragment

uniform vec4 color;
in float intensity;
out vec4 frag_color;

void main()
{	
	frag_color = color * intensity;	
}

#endif 