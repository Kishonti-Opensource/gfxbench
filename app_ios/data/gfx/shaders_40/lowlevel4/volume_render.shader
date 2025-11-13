/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#line 1
uniform mat4 mvp;
uniform mat4 mv;

uniform sampler2D color_map;
uniform sampler2D normal_map;
uniform sampler2D depth_map;
uniform sampler2D ssao_map;
uniform samplerCube env_map;
uniform sampler3D texture_3D_unit0;

uniform float tessellation_factor;
uniform vec3 view_pos;
uniform vec2 hdr_params;
uniform vec4 reflection;
uniform vec3 forward_vector;

struct CLAABB2x4
{
	vec3 min;
	vec3 max;
};


struct CLRay
{
	vec3 org;
	vec3 dir;
};


bool IntersectAABB2x4( CLAABB2x4 aabb, CLRay ray, out float tnear, out float tfar)
{	
	vec3 dirInv= vec3(1.0f) / ray.dir;

	vec3 tnear4 = dirInv * (aabb.min - ray.org);
    vec3 tfar4 = dirInv * (aabb.max - ray.org);

    vec3 t0 = min(tnear4, tfar4);
    vec3 t1 = max(tnear4, tfar4);

    tnear = max(max(t0.x, t0.y), t0.z);
    tfar = min(min(t1.x, t1.y), t1.z);

    return (tfar >= tnear) && (tfar >= 0.0f);
}


#ifdef TYPE_vertex

in vec3 in_position;
out vec3 out_position;
out vec3 out_view_dir;

void main()
{	
	gl_Position = mvp * vec4( in_position, 1.0);
	out_position = in_position;
	out_view_dir = out_position - view_pos;
}

#endif


#ifdef TYPE_fragment

in vec3 out_position;
in vec3 out_view_dir;

out vec4 frag_color;

void main()
{	
	CLAABB2x4 aabb;
	CLRay ray;
	float tnear;
	float tfar;

	aabb.min = vec3( 0.0, 0.0, 0.0);
	aabb.max = vec3( 1.0, 1.0, 1.0);
	ray.org = view_pos;
	ray.dir = normalize( out_view_dir);
	
	IntersectAABB2x4( aabb, ray, tnear, tfar);

	vec4 c = vec4( 0.0, 0.0, 0.0, 1.0);

	for( float i=tnear; i<tfar; i+=0.02)
	{
		vec3 pp = ray.org + ray.dir * i;
		vec4 v = texture( texture_3D_unit0, pp);

		//http://graphicsrunner.blogspot.hu/2009/01/volume-rendering-101.html
		//c.xyz = c.xyz + (1.0 - c.a) * v.a * v.xyz;
		//c.a = c.a + (1.0 - c.a) * v.a; 
		//https://developer.nvidia.com/content/transparency-or-translucency-rendering
		c.xyz = c.a * (v.a * v.xyz) + c.xyz;
		c.a = (1.0 - v.a) * c.a;
	}

	frag_color = vec4( c);
	// frag_color.w = 0.0;
}

#endif
