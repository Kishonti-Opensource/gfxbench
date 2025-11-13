/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef USE_UBOs
	#include cameraConsts;
	#include meshConsts;
	#include staticMeshConsts;
#else
	uniform mat4 mvp;
	uniform mat4 mv;
	uniform mat4 model;
	uniform vec3 view_pos;
#endif

in vec3 in_position;
in vec2 in_texcoord0;

#ifdef INSTANCING
//in mat4 in_instance_mv;
in vec4 in_instance_mv0;
in vec4 in_instance_mv1;
in vec4 in_instance_mv2;
in vec4 in_instance_mv3;

in mat4 in_instance_inv_mv;
#endif
	
out vec2 out_texcoord0;


void main()
{    
	vec3 up; 
	vec3 right;
	vec3 fwd;

#ifdef INSTANCING
	mat4 in_instance_mv = mat4(in_instance_mv0, in_instance_mv1, in_instance_mv2, in_instance_mv3);
#endif
	
#ifdef AALIGNED	
	up = vec3( 0.0, 1.0, 0.0);
#else
	up = vec3( mv[0][1], mv[1][1], mv[2][1]);
#endif

	right = vec3( mv[0][0], mv[1][0], mv[2][0]);
	
#ifdef USE_UBOs
	vec3 delta = vec3( model[3][0], model[3][1], model[3][2]) - view_posXYZ_normalized_time.xyz;
#else
	vec3 delta = vec3( model[3][0], model[3][1], model[3][2]) - view_pos;
#endif
	delta = normalize( delta);

	vec3 position = in_position.x * right + in_position.y * up + in_position.z * delta;

#ifdef INSTANCING
    mat4 mvp2 = mvp * in_instance_mv;

	gl_Position = mvp2 * vec4( position, 1.0);
#else
	gl_Position = mvp * vec4( position, 1.0);
#endif

	out_texcoord0 = in_texcoord0;
}
