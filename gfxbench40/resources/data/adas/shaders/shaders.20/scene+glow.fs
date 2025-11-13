/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
varying vec2 out_texcoord0;

uniform sampler2D texture_unit0;
uniform sampler2D texture_unit1;


void main()
{   
	vec4 color = texture2D( texture_unit0, out_texcoord0);
	vec4 glow = texture2D( texture_unit1, out_texcoord0);

	gl_FragColor = color + glow - glow * color;
	//gl_FragColor = glow;
	//gl_FragColor = color;

	// gl_FragColor = vec4( 1.0);
	//gl_FragColor.xy = out_texcoord0;
}
