/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform vec3 global_light_dir;
uniform vec3 global_light_color;
uniform vec3 view_dir;
uniform vec3 background_color;
uniform float time;
uniform sampler2D texture_unit0;
uniform sampler2D texture_unit1;
uniform sampler2D texture_unit2;
uniform sampler2D texture_unit3;
uniform float diffuse_intensity;
uniform float specular_intensity;
uniform float specular_exponent;
uniform float reflect_intensity;
uniform samplerCube envmap0;
uniform samplerCube envmap1;
uniform float envmaps_interpolator;
uniform float transparency;
uniform sampler2D shadow_unit;
uniform samplerCube normalization_cubemap;
uniform vec2 inv_resolution;


in vec2 out_texcoord0;
in vec2 out_texcoord1;
in vec3 out_texcoord4;
in vec3 out_view_dir;
in vec3 out_normal;
in vec3 out_tangent;
in float fog_distance;
in float dof;
in vec2 out_texcoord01;
in vec2 out_texcoord02;
in vec2 out_texcoord03;
in vec2 out_texcoord04;

out vec4 frag_color;

void main()
{   
	const int n = 4;
	mediump vec3 motion = texture( texture_unit1, out_texcoord0).xyz; 
	vec4 texel = texture( texture_unit0, out_texcoord0);
	vec3 color = texel.xyz / float( n); 

	motion.xy = (motion.xy - 0.5) * vec2( 0.0666667,.125);
	motion.xy *= texel.a;
	
	for( int i=1; i<n; i++)
	{
		vec2 tc = out_texcoord0 - motion.xy * float( i);
		color += texture( texture_unit0, tc).xyz / float( n); 
	}

	frag_color.xyz = color;
}
