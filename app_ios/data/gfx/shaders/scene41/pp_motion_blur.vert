/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
in vec2 in_position;
in vec2 in_texcoord0_;

out vec2 texcoord;
void main()
{
	vec4 in_pos = vec4( in_position, 0.0, 1.0);

	gl_Position = in_pos;
	texcoord = in_texcoord0_;
}
