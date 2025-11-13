/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 model;
uniform vec4 particle_data[50];
uniform vec4 particle_color[50];
uniform mat4 world_fit_matrix;


attribute vec3 in_position;
attribute vec2 in_texcoord0;
	
	
varying vec2 out_texcoord0;
varying vec4 out_color;
varying float out_life;
varying vec2 world_texcoord;

void main()
{    
	vec3 up; 
	vec3 right;
	vec3 fwd;

	up = vec3( mv[0][1], mv[1][1], mv[2][1]);

	right = vec3( mv[0][0], mv[1][0], mv[2][0]);
	
	int idx = int(in_position.x);

	vec2 offset = in_texcoord0 * 2.0 - 1.0;
	
	offset *=  0.2 + sqrt(particle_data[idx].w) * 1.8;

	vec3 position = particle_data[idx].xyz + offset.x * right + offset.y * up;

	gl_Position = mvp * vec4( position, 1.0);

	float deltay = floor( particle_color[idx].w * 63.0);
	float deltax = floor( mod( deltay, 8.0)) ;
	
	deltay = floor( deltay / 8.0);
	
	out_texcoord0 = vec2( (deltax + in_texcoord0.x) / 8.0, (1.0 + deltay - in_texcoord0.y) / 8.0);
		
	out_life = particle_data[idx].w;
	
	out_color = particle_color[idx];

	world_texcoord = (world_fit_matrix * vec4( position, 1.0)).xy;
}
