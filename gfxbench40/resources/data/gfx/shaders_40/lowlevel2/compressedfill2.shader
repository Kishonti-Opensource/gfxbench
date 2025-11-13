/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_vertex
uniform float u_rotation;
uniform float u_aspectRatio;
uniform vec4 u_offsets; // pos_x-y offsets, u-v offsets

in vec4 in_vertex; // pos_x, pos_y, u, v

out vec2 v_texCoord0;
out vec2 v_texCoord1;
out vec2 v_texCoord2;

mat2 rotateZ(float alpha)
{
	return mat2(
	cos(alpha), -sin(alpha),
	sin(alpha), cos(alpha));
}

void main()
{
#ifdef RESIZE_PASS
	v_texCoord0 = in_vertex.zw;
	gl_Position = vec4(in_vertex.xy * 2.0, 0.0, 1.0);	
#else	
	#ifdef CUBE_DEPTH_PASS
		vec2 aspect_uv = in_vertex.zw * vec2(u_aspectRatio, 1.0);			
		v_texCoord0 = in_vertex.zw;
		
		mat2 rot1 = rotateZ(-u_rotation);
		v_texCoord1 = aspect_uv * rot1; 
		
		mat2 rot2 = rotateZ(-u_rotation * 0.5);
		v_texCoord2 = aspect_uv * rot2;
				
		gl_Position = vec4(in_vertex.xy * 2.0, 0.0, 1.0);		
	#else		
		vec2 aspect_uv = in_vertex.zw * 0.5 + u_offsets.zw;
		aspect_uv *= vec2(u_aspectRatio, 1.0);
		
		v_texCoord0 = aspect_uv + vec2(u_rotation * -0.3, 0.0);		
		
		mat2 rot = rotateZ(-u_rotation * 0.2);
		v_texCoord1 = aspect_uv * rot;
		
		v_texCoord2 = aspect_uv + vec2(u_rotation * -0.1, 0.0);
		v_texCoord2 *= 0.25;
		
		gl_Position = vec4(in_vertex.xy + u_offsets.xy, 0.0, 1.0);
	#endif	
#endif
}
#endif

#ifdef TYPE_fragment
uniform sampler2D texture_unit0;
uniform sampler2D texture_unit2;
uniform sampler2D texture_unit3;

#ifdef CUBE_DEPTH_PASS
uniform samplerCube envmap0;
#else
uniform sampler2D texture_unit1;
#endif

in vec2 v_texCoord0;
in vec2 v_texCoord1;
in vec2 v_texCoord2;

out vec4 frag_color;

void main()
{
#ifdef RESIZE_PASS
	frag_color = texture(texture_unit0, vec2(v_texCoord0.x,  v_texCoord0.y));	
#elif defined CUBE_DEPTH_PASS
	vec4 sample0 = texture(texture_unit0, v_texCoord0);
	vec4 sample1 = texture(envmap0, vec3(v_texCoord1, 1.0 - v_texCoord1.x)); // cube map
	vec4 sample2 = vec4(texture(texture_unit2, v_texCoord2).x); // depth texture
	vec4 sample3 = texture(texture_unit3, v_texCoord0);
	vec4 layers = sample1 * 0.02 + sample2 * 0.02;
	frag_color = vec4(sample0.xyz + layers.xyz, sample3.w);		
#else
	vec4 sample0 = texture(texture_unit0, v_texCoord0);	
	vec4 sample1 = texture(texture_unit1, v_texCoord0);	
	vec4 sample2 = texture(texture_unit2, v_texCoord1);	
	vec4 sample3 = texture(texture_unit3, v_texCoord2);	
	
	vec4 layers = mix(sample3, sample2,sample2.w);
	layers += layers * sample1 * 0.5 + sample0 * 0.01;
	frag_color = vec4(layers.xyz, 1.0);	
#endif
}
#endif
