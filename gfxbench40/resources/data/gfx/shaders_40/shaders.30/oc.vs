/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef USE_UBOs
	#include cameraConsts;
#else
	uniform mat4 vp;
#endif

layout(location = 0) in vec3 in_position;
in vec3 in_normal;
	
out vec3 out_normal;
out vec3 out_pos;

void main()
{    
	out_normal = in_normal;
	gl_Position = vp * vec4( in_position, 1.0);
	gl_PointSize = 6.0;
	
	out_pos = in_position;
}
