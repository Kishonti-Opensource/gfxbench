/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//SKY IS DRAWN TWICE - FIRST, INTO G-BUFFER, BUT ONLY VELOCITY PART, SECOND AFTER LIGHTING BUT BEFORE TONEMAPPING, INTO THE COMBINED LIGHT BUFFER

#include "common.h"
#include "mb_common.h"
#include "attribute_locations.h"

uniform vec4 cam_near_far_pid_vpscale;

#ifdef TYPE_vertex

layout(location = ATTR_POSITION_LOCATION)  in vec3 in_position;
layout(location = ATTR_TEXCOORD0_LOCATION) in vec2 in_texcoord0;
layout(location = ATTR_NORMAL_LOCATION)    in vec3 in_normal;
layout(location = ATTR_TANGENT_LOCATION)   in vec3 in_tangent;

out vec2 v_texcoord;
out vec3 v_normal;
out vec3 v_tangent;

#ifdef VELOCITY_BUFFER
out vec4 scPos ;
out vec4 prevScPos ;
#endif

#ifdef PARABOLOID
out float v_side;
#endif

void main()
{	

	vec3 pos = in_position;
	
	pos.y -= 100.4;

#ifdef PARABOLOID
	vec4 _scPos = mv * vec4( pos, 1.0);

	_scPos.z = _scPos.z * cam_near_far_pid_vpscale.z;
		
	v_side = _scPos.z ; //verts falling to the "other" parab get negative here, cullable in PS
		
	//TODO handle directions
	float L = length(_scPos.xyz);
	_scPos = _scPos / L;
	
	
	
	_scPos.z = _scPos.z + 1.0;
	_scPos.x = _scPos.x / _scPos.z;
	_scPos.y = _scPos.y / _scPos.z;	
	
	_scPos.z = (L - cam_near_far_pid_vpscale.x) / (cam_near_far_pid_vpscale.y - cam_near_far_pid_vpscale.x);// * sign(v_side); //for proper depth sorting
	
	_scPos += normalize(_scPos)*0.04; //small bias to remove seam
	_scPos.z = _scPos.z * 2.0 - 1.0; // fit to clipspace		
	
	_scPos.w = 1.0;
	
#else
	vec4 _scPos = mvp * vec4( pos, 1.0);
#endif

#ifdef VELOCITY_BUFFER
	scPos = _scPos ;
	prevScPos = mvp2 * vec4(pos, 1.0) ; 
#endif

    gl_Position = _scPos ;
	gl_Position.z = gl_Position.w;

	
	

	v_texcoord = in_texcoord0;
		
	v_normal = in_normal;
	v_tangent = in_tangent;
}

#endif


#ifdef TYPE_fragment

in vec2 v_texcoord;

layout(location = 0) out /*highp*/ vec4 frag_color;

#ifdef VELOCITY_BUFFER
layout(location = 1) out VELOCITY_BUFFER_TYPE velocity ;

in vec4 scPos ;
in vec4 prevScPos ;
#endif

#ifdef PARABOLOID
in float v_side;
#endif

#ifdef G_BUFFER_PASS
layout(location = 2) out vec4 frag_normal;
layout(location = 3) out vec4 frag_params;
#endif

void main()
{
#ifdef PARABOLOID
	if(v_side < 0.0)
		discard;
#endif

#ifdef VELOCITY_BUFFER
	//velocity = pack2FloatToVec4( velocityFunction(scPos,prevScPos) );
	velocity = pack_velocity( velocityFunction(scPos,prevScPos) );
#endif	
	
#ifdef G_BUFFER_PASS //has to be rendered to G-buffer - screen velocity needs updating
	frag_color = vec4(0.0, 0.0, 0.0, 0.0); //8888, albedoRGB, emissive - dummy data
	frag_normal = vec4(0.0,-1.0,0.0,0.0); //8888, encoded world normal + staticAO + staticShadow - dummy data
	frag_params = vec4(0.0,0.0,0.0,0.0); //8888, specCol, smoothness	- dummy data
#else //cubemap and light_combine passes
	vec4 color = texture( texture_unit0, v_texcoord);
	vec3 skyCol = RGBMtoRGB( color);	
	float skyLum = getLum(skyCol);
	float skyBoost = 1.0;// + pow(max(0.0, skyLum - 0.4), 10.0);
	vec3 result_color = skyCol * skyBoost; //HACK: boost sky colors where too dark
		
#ifdef LIGHTCOMBINE_PASS
	frag_color = RGBtoRGBD_lightcombine(result_color); //fp16
#else
	frag_color = RGBtoRGBM_cubemap(result_color); //fp16
#endif

#endif
}

#endif
