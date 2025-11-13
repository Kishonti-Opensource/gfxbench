/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform vec2 offset_2d;

attribute vec3 in_position;

varying vec2 out_texcoord0;
varying vec2 out_texcoord1;
varying vec2 out_texcoord2;

void main()
{    
	gl_Position = vec4( in_position, 1.0);
	out_texcoord0 = in_position.xy * 0.5 + 0.5;
	out_texcoord1 = out_texcoord0 + offset_2d;
	out_texcoord2 = out_texcoord0 - offset_2d;
}
