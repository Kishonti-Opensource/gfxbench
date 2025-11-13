/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
in vec2 out_texcoord0;
out vec4 frag_color;

uniform sampler2D texture_unit0;
uniform sampler2D texture_unit1;
uniform sampler2D texture_unit2;
uniform sampler2D texture_unit3;
uniform sampler2D texture_unit4;
uniform sampler2D texture_unit5;
uniform vec4 depth_parameters;
uniform vec2 inv_resolution;
uniform float camera_focus_inv; // 1.0 / camera_focus;
uniform float dof_strength_inv; // 1.0 / (dof_strength * 0.1)


float GetLinearDepth()
{
	float d = texture( texture_unit2, out_texcoord0).x;

	d = depth_parameters.y / (d - depth_parameters.x);

	return d;
}

float Dof()
{
	float depth = GetLinearDepth();
	
	float dof = clamp( abs(log(depth * camera_focus_inv)) * dof_strength_inv, 0.0, 1.0);

	dof = pow( dof, 0.6);

	return dof;
}


void main()
{
	vec4 c0 = texture( texture_unit0, out_texcoord0);
	//vec4 c1 = texture( texture_unit1, out_texcoord0);
	// vec4 c2 = texture( texture_unit2, out_texcoord0);
	
	// vec4 c4 = texture( texture_unit4, out_texcoord0);
	// vec4 c5 = texture( texture_unit5, out_texcoord0);

	

	vec4 final;
#ifdef DOF_ENABLED
	vec4 c3 = texture( texture_unit3, out_texcoord0);
	float f = Dof() ; //max mipcount is 4, set by code
	final = mix(c0,c3, clamp( 2.0*f, 0.0, 1.0) );
#else
	final = c0;
#endif
	// final = c3;

	//float coc = clamp(2*f,0,1) ;
	//final = vec4(coc,-coc,0.0,1.0) ;

	frag_color = final * vec4( 1.0);
}
