/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
in vec3 in_position;
in vec2 in_texcoord;

out vec2 v_texcoord;

void main()
{    
	gl_Position = vec4( in_position, 1.0);
	v_texcoord = in_texcoord;
}
