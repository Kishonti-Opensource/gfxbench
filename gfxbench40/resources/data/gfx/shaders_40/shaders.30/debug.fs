/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform vec3 background_color;

in vec3 out_normal;
in vec3 out_pos;
out vec4 frag_color;


void main()
{   
	frag_color = vec4( background_color, 0.01);
}

