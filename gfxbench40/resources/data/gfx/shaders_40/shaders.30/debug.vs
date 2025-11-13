/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 mvp;

in vec3 in_position;
in vec3 in_normal;
	
out vec3 out_normal;
out vec3 out_pos;

void main()
{    
	out_normal = in_normal;
	gl_Position = mvp * vec4( in_position, 1.0);
	
	out_pos = in_position;
}
