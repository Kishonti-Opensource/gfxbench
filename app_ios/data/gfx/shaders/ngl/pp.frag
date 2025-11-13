/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D texture0;

in vec2 v_texcoord;
out vec4 FragColor;

void main(void)
{
	FragColor = texture( texture0, v_texcoord);
	//FragColor += vec4(0.1);
	//FragColor = vec4(v_texcoord, 0.0, 1.0);
}
