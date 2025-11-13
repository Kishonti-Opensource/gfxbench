/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"

#ifdef TYPE_fragment

highp in vec2 out_texcoord0;
out vec3 frag_color;

uniform float camera_focus;
uniform float dof_strength;

// texture_unit0 - color texture
// texture_unit1 - blurred texture
// texture_unit2 - depth

float Dof(sampler2D depthTex,highp vec2 uv)
{
	float d_s = (dof_strength * 0.1);

	float depth = getLinearDepth(depthTex, uv);

	float dof = clamp( abs(log(depth/camera_focus)) / d_s, 0.0, 1.0);

	dof = pow( dof, 0.6);

	return dof;
}


void main()
{
	vec4 c0 = texture( texture_unit0, out_texcoord0);
	vec4 c1 = texture( texture_unit1, out_texcoord0);

	float f = clamp(2.0*Dof(texture_unit2, out_texcoord0)-0.75,0.0,1.0) ; 

#ifdef DRAW_GS_WIREFRAME	
	f = 0.0;
#endif
	vec3 final =  mix(c0.xyz,c1.xyz, f ) ; //NOTE: Use 0.0 instead of f to turn off DoF

	frag_color = final ;
}


#endif


#ifdef TYPE_vertex

in vec3 in_position;

out vec2 out_texcoord0;

void main()
{    
	gl_Position = vec4( in_position, 1.0);
	out_texcoord0 = in_position.xy * 0.5 + 0.5;
}



#endif