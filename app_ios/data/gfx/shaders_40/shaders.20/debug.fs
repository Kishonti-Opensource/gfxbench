/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform vec3 background_color;
varying vec3 out_normal;

void main()
{   
	gl_FragColor = vec4( background_color, 1.0);
	//gl_FragColor = vec4( out_normal, 1.0);
}

