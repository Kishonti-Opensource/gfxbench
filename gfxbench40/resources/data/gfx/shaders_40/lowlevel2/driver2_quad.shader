/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_vertex 
in vec2 myVertex;
out vec2 texcoord;

void main()
{
	gl_Position = vec4(myVertex, 1.0, 1.0);
	texcoord = (myVertex+vec2(1.0))/2.0;
}
#endif

#ifdef TYPE_fragment
uniform sampler2D texIn;
in vec2 texcoord;
out vec4 color;
void main()
{
	color = texture(texIn, texcoord);
}
#endif 