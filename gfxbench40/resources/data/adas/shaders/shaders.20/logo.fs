/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D texture_unit0;

varying vec2 v_texcoord;

void main()
{	
	vec4 color = texture2D( texture_unit0, v_texcoord);

	gl_FragColor = vec4( color);
}
