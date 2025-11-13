/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//varying vec2 depth;

vec3 EncodeFloatRGB( float v ) 
{
	vec3 enc = vec3(1.0, 255.0, 65025.0) * v;
	enc = fract(enc);
	//enc -= enc.yzz * vec3( 1.0/255.0, 1.0/255.0, 0.0);
	enc.xy -= enc.yz * (1.0/255.0);
	return enc;
}

void main()
{   
	gl_FragColor = vec4( 0.3);

#if defined RGB_ENCODED
	//float d = depth.x / depth.y;
	
	//d = d * 0.5 + 0.5;	
	
	//gl_FragColor.xyz = EncodeFloatRGB( d);
    gl_FragColor.xyz = EncodeFloatRGB( gl_FragCoord.z);
	
#endif

}

