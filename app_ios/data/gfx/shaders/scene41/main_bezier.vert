/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 mvp;

in vec3 in_position;

void main()
{
	gl_Position = vec4(in_position, 1.0);
}
