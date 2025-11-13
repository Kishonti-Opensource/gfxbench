/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
varying vec3 out_texcoord0;
uniform sampler2D shadow_unit0;

void main()
{   	
	vec4 color = vec4( 1.0);

	if( out_texcoord0.z < 1.0 && out_texcoord0.z > 0.0)
	{
		float color0 = texture2D( shadow_unit0, out_texcoord0.xy).x;
		color = vec4( color0, color0, color0, 1.0);
	}
	
	
	gl_FragColor = color;
	//gl_FragColor.x = 1.0;
}

