/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef USE_UBOs
	#include meshConsts;
#else
	uniform mat4 mvp;
#endif

in vec3 in_position;
in vec2 in_texcoord0;
out vec2 out_texcoord0;

void main()
{    
	//gl_Position = mvp * vec4( in_position, 1.0);
	mat4 mvp_1 = mvp;
	gl_Position = mvp_1 * vec4( in_position, 1.0);
	
	gl_Position.z = gl_Position.w;
	out_texcoord0 = in_texcoord0;
}
