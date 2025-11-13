/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D texture_unit0;
uniform sampler2D texture_unit1;
uniform highp sampler2D texture_unit3; // depth
uniform sampler2D texture_unit4;

#ifdef USE_UBOs
	#include frame;
	#include cameraConsts;		
#else
	uniform vec4 depth_parameters;
	uniform vec3 view_pos;
	uniform mat4 shadow_matrix0;
#endif


uniform sampler2DShadow shadow_unit0;

in vec4 out_pos;
in vec4 out_view_dir;
out vec4 frag_color;



vec3 get_world_pos( vec2 out_texcoord) 
{
	highp float d_highp = texture( texture_unit3, out_texcoord).x;
	float d = depth_parameters.y / (d_highp - depth_parameters.x);	
	
#ifdef USE_UBOs
	return d * out_view_dir.xyz / out_view_dir.w + view_posXYZ_normalized_time.xyz;
#else
	return d * out_view_dir.xyz / out_view_dir.w + view_pos;
#endif	
}

void main()
{
	const float inv_res0 = 1.0 / 1024.0;
	const float inv_res1 = 0.707 / 1024.0;
	const float shadow_strength = 0.25;
	
	float shadow = 1.0;
	
	vec2 out_texcoord = (out_pos.xy / out_pos.w * 0.5) + vec2(0.5,0.5);
	
	vec3 view_dir = normalize( out_view_dir.xyz);

	vec3 position = get_world_pos( out_texcoord);

	vec4 shadow_texcoord = shadow_matrix0 * vec4( position, 1.0);

	shadow = textureProj( shadow_unit0, shadow_texcoord);

	frag_color = mix(vec4(0.25),vec4(1.0),shadow);
}
