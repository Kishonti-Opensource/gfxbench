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
uniform sampler2D shadow_unit0;
uniform samplerCube normalization_cubemap;
uniform vec2 inv_resolution;
uniform float alpha_threshold;


varying vec2 out_texcoord0;
varying vec2 out_texcoord1;
varying vec3 out_texcoord4;
varying vec3 out_view_dir;
varying vec3 out_normal;
varying vec3 out_tangent;
varying float fog_distance;
varying float dof;
varying vec2 out_texcoord01;
varying vec2 out_texcoord02;
varying vec2 out_texcoord03;
varying vec2 out_texcoord04;





void main()
{   
	vec3 color = texture2D( texture_unit0, out_texcoord0).xyz;
	
	if( (1.0 - out_texcoord1.t) > alpha_threshold)
	{
		color = vec3( 0.5);
	}

	gl_FragColor = vec4( color, 1.0);
}
