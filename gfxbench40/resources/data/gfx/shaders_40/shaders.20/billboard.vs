/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 model;
uniform vec3 view_pos;

attribute vec3 in_position;
attribute vec2 in_texcoord0;
	
	
varying vec2 out_texcoord0;


void main()
{    
	vec3 up; 
	vec3 right;
	vec3 fwd;

#ifdef AALIGNED	
	up = vec3( 0.0, 1.0, 0.0);
#else
	up = vec3( mv[0][1], mv[1][1], mv[2][1]);
#endif

	right = vec3( mv[0][0], mv[1][0], mv[2][0]);
	
	vec3 delta = vec3( model[3][0], model[3][1], model[3][2]) - view_pos;
	delta = normalize( delta);

	vec3 position = in_position.x * right + in_position.y * up + in_position.z * delta;

	gl_Position = mvp * vec4( position, 1);

	out_texcoord0 = in_texcoord0;
}
