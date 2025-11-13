/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
in vec3 in_position;
in vec2 in_texcoord0;

out vec2 out_texcoord0;

void main()
{    
	gl_Position = vec4( in_position.xy * 2.0 - 1.0, 0.0, 1.0);
	out_texcoord0 = in_texcoord0;
}
