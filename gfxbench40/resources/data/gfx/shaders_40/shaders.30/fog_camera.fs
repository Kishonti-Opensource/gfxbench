/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

uniform sampler2D texture_unit0; //depth
uniform sampler2D texture_unit1; //volume color
uniform sampler3D texture_3D_unit0;

uniform float time;
uniform vec3 light_color;
uniform vec2 inv_resolution;
uniform vec4 depth_parameters;

in vec4 out_pos;
in vec4 shadow_texcoord;

out vec4 frag_color;

#ifdef GL_ES
precision highp float;
#endif

void main()
{   
	float t = time * 8.0;
	
	vec2 depth_texcoord = vec2( gl_FragCoord.xy) * inv_resolution;
	float d = texture( texture_unit0, depth_texcoord).x;
	float z_view_depth = depth_parameters.y / (d - depth_parameters.x);
	float depthDiffScale = 0.1;
	float depth_atten = clamp((z_view_depth - out_pos.w) * depthDiffScale, 0.0, 1.0);

	vec3 out_texcoord0 = shadow_texcoord.xyz / shadow_texcoord.w;
	
	float world_atten = clamp((5.0 - out_pos.y) / 5.0, 0.0, 1.0); ///texture( texture_unit0, out_pos.xz / 50.0) *

	//atten = texture( texture_3D_unit0, vec3(out_pos.xz / 120.0 + t / 10.0, t)).x * world_atten; //texture( texture_3D_unit0, vec3(out_texcoord0.xy,0));
	float atten = 1.0 * texture( texture_3D_unit0, vec3(out_pos.xz / 120.0 + t / 10.0, out_pos.y / 20.0 - t)).x * world_atten * 5.0; //texture( texture_3D_unit0, vec3(out_texcoord0.xy,0));
/*
	vec2 offset = vec2(-112.0, -98.0);
	float scale = 1.0 / 408.8;
	vec2 volColUV = (out_pos.zx - offset) * vec2(scale, scale);
	vec3 volumeColor = texture( texture_unit1, volColUV ).xyz * 2.0 * clamp((15.0 - out_pos.y) / 5.0, 0.0, 1.0);
*/	
	frag_color = vec4( light_color.xyz /*+ volumeColor*/, atten) * vec4( 0.2, 0.2, 0.2, 0.2);

	//frag_color = vec4(z_view_depth.xxx/15,1.0);
	
	
	
	
	float fogValue = texture( texture_3D_unit0, vec3(out_pos.xz / 60.0 + t / 2.0, out_pos.y / 5.0 - t)).x;
	
	float fogColor = 0.2;
	//frag_color = vec4(fogColor, fogColor, fogColor, fogValue * world_atten * depth_atten * 0.6);
	frag_color = vec4(fogColor, fogColor, fogColor, fogValue * 2.0 * world_atten * depth_atten * 1.0);
	
	//frag_color = vec4(0,0,0,0);
	
	//discard;
	
	//frag_color = vec4(depth_atten,depth_atten,depth_atten,1.0);
	//frag_color.xyz = out_pos.www/10;
	//frag_color.w = 1.0;
	//frag_color = vec4(out_scrollspeed,out_scrollspeed,out_scrollspeed,1);
	
	//frag_color = vec4(texture( texture_unit0, depth_texcoord).xxx,1);
}

