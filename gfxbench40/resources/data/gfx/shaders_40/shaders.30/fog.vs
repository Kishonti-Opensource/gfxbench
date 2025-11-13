/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef USE_UBOs
	#include lightShaftConsts;
#else
	uniform mat4 mvp;
	uniform mat4 mv;
	uniform mat4 shadow_matrix0;
#endif

layout (location = 0) in vec3 in_position;
	
out vec4 out_pos;
out vec4 out_pos_hom; //homogeneous vertpos
out vec4 shadow_texcoord;

void main()
{    
	out_pos_hom = mvp * vec4( in_position, 1.0);
	gl_Position = out_pos_hom;
	
	out_pos.xyz = in_position;
	out_pos.w = -vec4(mv * vec4( in_position, 1.0)).z;
	shadow_texcoord = shadow_matrix0 * vec4( in_position, 1.0);
}
