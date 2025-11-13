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

/*
#define MBLUR_ALT

#ifdef MBLUR_ALT
varying vec2 out_diff;

void main()
{
	vec2 diff = out_diff;

	diff *= 8.0;
	//diff = clamp(diff, -0.02, 0.02);
	diff.x *= 1.875;
	diff += 0.5;
	gl_FragColor = vec4( diff, 0.0, 0.0);
}
#else
*/
varying vec4 out_curpos;
varying vec4 out_prevpos;






void main()
{   
	vec2 a = (out_curpos.xy / out_curpos.w);
	vec2 b = (out_prevpos.xy / out_prevpos.w);

	vec2 diff = a - b;
	float l = length( diff);
	
	if( l > 0.0)
	{
		diff.x /= 0.0666667;
		diff.y /= 0.125;
		diff += 0.5;
	}
	else
	{
		diff = vec2( 0.5);
	}

	gl_FragColor = vec4( diff, 0.0, 0.0);
}

//#endif
