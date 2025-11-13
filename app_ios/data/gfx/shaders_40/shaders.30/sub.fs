/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
in vec2 out_texcoord0;
out vec4 frag_color;

uniform sampler2D texture_unit0;
uniform sampler2D texture_unit1;


void main()
{   
	vec3 c2 = texture( texture_unit1, out_texcoord0, 15.0).xyz;
	float l2 = 0.0;//dot( c2, vec3( 0.2126 , 0.7152 , 0.0722 ));



	vec4 cc = texture( texture_unit0, out_texcoord0);
	vec3 c = texture( texture_unit0, out_texcoord0).xyz;
	float l = dot( c, vec3( 0.2126 , 0.7152 , 0.0722 )) - l2;
	
	l = clamp( cc.w + pow(l, 4.0) * 128.0, 0.0, 1.0);
	
	c = mix( vec3( 0.0, 0.0, 0.0), c, l);

	frag_color = vec4( c, 1.0);
}
