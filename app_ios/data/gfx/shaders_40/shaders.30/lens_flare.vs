/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef USE_UBOs
	#include cameraConsts;
	#include meshConsts;
	#include lightLensFlareConsts;
#else
	uniform mat4 mvp;
	uniform vec2 inv_resolution;
	uniform vec3 light_pos;
#endif

in vec3 in_position;
in vec2 in_texcoord0;
	
	
out vec2 out_texcoord0;


void main()
{    
	vec3 up; 
	vec3 right;
	vec3 fwd;
	vec2 size;

	//vec4 pos0 = mvp * vec4( light_pos, 1);
	mat4 mvp_1 = mvp;
	
#ifdef USE_UBOs
	vec4 pos0 = mvp_1 * vec4( light_pos_pad.xyz, 1);
#else
	vec4 pos0 = mvp_1 * vec4( light_pos, 1);
#endif	
	
	pos0.xy /= pos0.w;
	
	size.x = in_position.x;
	#ifdef USE_UBOs
		size.y = in_position.y * (1.0 / inv_resolutionXY_pad.x) / (1.0 / inv_resolutionXY_pad.y);
	#else
		size.y = in_position.y * (1.0 / inv_resolution.x) / (1.0 / inv_resolution.y);
	#endif

	vec2 pos1 = -pos0.xy;
	
	vec2 pos = mix( pos0.xy, pos1, in_position.z);
	
	gl_Position.x = size.x + pos.x;
	gl_Position.y = size.y + pos.y;
	gl_Position.z = 0.0;
	gl_Position.w = 1.0;

	out_texcoord0 = in_texcoord0;
}
