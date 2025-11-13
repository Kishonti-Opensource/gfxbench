/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ubo_bindings.h"

uniform mat4 mvp;

uniform highp vec2 sun_dir ;

layout(std140, binding = LENSFLARE_BINDING_SLOT) uniform Visibility
{
    mediump int _visibility ; 
};

#ifdef TYPE_vertex

in vec3 in_position;
in vec2 in_texcoord0;

out vec2 v_texcoord;

uniform highp float flare_size ;
uniform highp float flare_distance ;


void main()
{	
	// flare strength 0.0 .. 1.0
	float strength = float( _visibility )/float(MAX_SAMPLE) ;

	// minimum size
	float minimum_size = 0.6 ;
	
	strength = (1.0 - minimum_size )*strength + minimum_size ;
	
	vec2 vc = in_position.xy ;
	vc *= strength * flare_size ;
	vc += -flare_distance * sun_dir ;

	vec4 out_pos = mvp * vec4( vc, 0.0, 1.0) ;
	
	gl_Position = out_pos ;
	
	v_texcoord = in_texcoord0;
}

#endif


#ifdef TYPE_fragment

in vec2 v_texcoord;

out vec4 frag_color;

layout (binding = 0) uniform highp sampler2D color_tex ;
layout (binding = 1) uniform highp sampler2D dirt_tex ;

vec4 texture2DDistorted(sampler2D Texture, vec2 TexCoord, vec2 Offset)
{
	return vec4(
		texture(Texture, TexCoord + Offset * -1.0).r,
		texture(Texture, TexCoord + Offset * 0.0).g,
		texture(Texture, TexCoord + Offset * 1.0).b,
		1.0
	);
}

void main()
{	
	vec4 color ;
	float strength = float( _visibility )/float(MAX_SAMPLE) ;
	
	
	float maximum_stength = 0.20 ;
	float distortion_strength = 0.02 ;
	

	strength = clamp(strength,0.0,1.0) ;
	strength = maximum_stength * strength ;
	
	vec2 screen_coord = vec2(gl_FragCoord.x/float(VIEWPORT_WIDTH), gl_FragCoord.y/float(VIEWPORT_HEIGHT) ) ;
	
	vec2 distortion = abs(2.0 * screen_coord - 1.0) ;
	
	distortion *= distortion_strength ;
	
	color = strength * texture2DDistorted(color_tex, v_texcoord, distortion) * vec4( 5.0 *texture2DDistorted(dirt_tex, screen_coord, distortion ).xyz + vec3(0.5) , 1.0) ;
	
	frag_color = vec4( color.xyz, 0.0 );
}

#endif
