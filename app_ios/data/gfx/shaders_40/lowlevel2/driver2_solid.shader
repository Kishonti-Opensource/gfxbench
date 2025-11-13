/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_vertex 

in vec3 myVertex;

uniform vec2 u_position;
uniform vec2 u_gridsize;
uniform vec2 u_margin;
uniform vec2 u_globmargin;
uniform mat2 u_transform_matrix;

out vec2 texpos;

void main()
{
	vec2 cellsize = (vec2(2.0)-u_globmargin*2.0)/u_gridsize;
	vec2 localCoord = myVertex.xy * (vec2(1.0) - u_margin) * cellsize / 2.0;
	vec2 pos = vec2(-1.0) + u_globmargin + cellsize/2.0 + cellsize*u_position;
	vec2 abspos = pos + localCoord;
	gl_Position = vec4(abspos*u_transform_matrix, myVertex.z, 1.0);
	texpos = abspos/2.0+vec2(0.5);
}

#endif

#ifdef TYPE_fragment

uniform sampler2D texIn;
uniform vec3 u_color;

in vec2 texpos;
out vec4 color;
void main()
{
	color = vec4(u_color+texture(texIn,texpos).rgb-vec3(0.5), 1.0);
} 

#endif 