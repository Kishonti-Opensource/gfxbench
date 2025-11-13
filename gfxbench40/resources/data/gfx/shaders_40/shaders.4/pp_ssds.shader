/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"

#ifdef TYPE_fragment

highp in vec2 out_texcoord0;

highp out vec4 frag_color;

void main() 
{
	highp float depth = texture(depth_unit0, out_texcoord0).x;
	highp float linear_depth = getLinearDepth(depth); 
	highp vec3 world_pos = getPositionWS(linear_depth, out_texcoord0);
	
	frag_color.x = texture(texture_unit1, out_texcoord0.xy).x; // Copy the AO
	frag_color.y = shadowCascaded(depth, vec4(world_pos, 1.0)); // Shadow
	frag_color.zw = EncodeLinearDepthToVec2(linear_depth); // Encode the linear depth to the BA channels
}

#endif

#ifdef TYPE_vertex

in vec3 in_position;
out vec2 out_texcoord0;

void main()
{
	gl_Position = vec4( in_position, 1.0);
	out_texcoord0 = in_position.xy * 0.5 + 0.5;
}


#endif
