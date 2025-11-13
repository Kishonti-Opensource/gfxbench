/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 mvp;

attribute vec3 in_position;
attribute vec2 in_texcoord0;
	
varying vec2 out_texcoord0;

void main()
{    
	gl_Position = mvp * vec4( in_position, 1.0);
	gl_Position.z = gl_Position.w;
	out_texcoord0 = in_texcoord0;
}
