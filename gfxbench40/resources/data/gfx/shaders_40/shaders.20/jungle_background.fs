/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform vec3 background_color;
uniform sampler2D texture_unit0;
uniform sampler2D texture_unit2;

varying vec2 out_texcoord0;

#if defined FOG
varying float fog_distance;
#endif	


void main()
{   
#if defined MASK
	vec4 mask = texture2D( texture_unit2, out_texcoord0);
#else
	const vec4 mask = vec4( 1.0, 1.0, 1.0, 1.0);
#endif

	vec3 color = texture2D( texture_unit0, out_texcoord0).xyz;

#if defined ALPHA_TEST
	if( mask.x < 0.6)
	{
		discard;
	}
#endif


#if defined FOG
	color = mix( color, background_color, fog_distance * fog_distance);
#endif	

	
#if defined TRANSPARENCY
	gl_FragColor = vec4( color, transparency * mask.x);
#else
	gl_FragColor = vec4( color, 1.0);
#endif
}

