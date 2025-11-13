/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
varying vec3 out_texcoord0;
uniform sampler2D shadow_unit0;



float DecodeFloatRGB( vec3 rgba ) 
{
	return dot( rgba, vec3(1.0, 1.0/255.0, 1.0/65025.0) );
}

float shadow()
{
#if defined RGB_ENCODED
	float color0 = DecodeFloatRGB( texture2D( shadow_unit0, out_texcoord0.xy).xyz);
#else
	float color0 = texture2D( shadow_unit0, out_texcoord0.xy).x;
#endif


	if( out_texcoord0.z < 1.0 && out_texcoord0.z > 0.0)
	{
		if( color0 < out_texcoord0.z)
		{
			return 0.33;
		}
	}
	return 1.0;
}

void main()
{   	
	float color0 = shadow();
	vec4 color = vec4(color0, color0, color0, 1.0);

	gl_FragColor = color;
}
