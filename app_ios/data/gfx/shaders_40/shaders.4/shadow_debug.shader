/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#ifdef TYPE_vertex

in vec4 in_position;

out vec2 v_uv;
void main()
{	
	v_uv = in_position.zw;
	gl_Position = vec4( in_position.xy, 0.0, 1.0);
}
#endif


#ifdef TYPE_fragment

uniform highp sampler2DArray tex;
uniform float layer;

in vec2 v_uv;
out vec4 frag_color;

void main()
{	
	highp vec3 value;
	value.rgb = texture(tex, vec3(v_uv.x, v_uv.y, layer)).xyz;	

	//normalize(value);	
	frag_color = vec4(value , 1.0);

}

#endif