/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 mvp;
uniform mat4 mv;
uniform float fog_density;

attribute vec3 in_position;
attribute vec2 in_texcoord0;

varying vec2 out_texcoord0;

#if defined FOG
varying float fog_distance;
#endif	


void main()
{    
	gl_Position = mvp * vec4( in_position, 1.0);
	gl_Position.z = gl_Position.w;
	out_texcoord0 = in_texcoord0;

#if defined FOG
	vec4 fog_position = mv * vec4( in_position, 1.0);
	fog_distance = clamp( -fog_position.z * fog_density, 0.0, 1.0);
#endif
}
