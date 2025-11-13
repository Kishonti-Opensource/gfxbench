/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//CHEAP SHADER - USED TO RENDER INTO CUBEMAP

#include "common.h"
#include "mb_common.h"
#include "attribute_locations.h"

uniform vec4 carindex_translucency_ssaostr_fovscale;
uniform vec4 cam_near_far_pid_vpscale;

#define PARABOLOID

#ifdef TYPE_vertex

layout(location = ATTR_POSITION_LOCATION)  in vec3 in_position;
layout(location = ATTR_TEXCOORD0_LOCATION) in vec2 in_texcoord0;
layout(location = ATTR_TEXCOORD1_LOCATION) in vec2 in_texcoord1;
layout(location = ATTR_NORMAL_LOCATION)    in vec3 in_normal;
layout(location = ATTR_TANGENT_LOCATION)   in vec3 in_tangent;

out vec2 v_texcoord0;
out vec2 v_texcoord1;
out vec3 v_normal;
out vec3 v_worldpos;

#ifdef VELOCITY_BUFFER
out vec4 scPos ;
out vec4 prevScPos ;
#endif

#ifdef PARABOLOID
out float v_side;
#endif

void main()
{	

#ifdef PARABOLOID

#ifdef INSTANCING		
	int instance_id = gl_InstanceID + instance_offset;
	mat4 instance_mv = view * instance_data[instance_id].model;
	vec4 _scPos = instance_mv * vec4( in_position, 1.0);
#else
	vec4 _scPos = mv * vec4( in_position, 1.0);
#endif

	_scPos.z = _scPos.z * cam_near_far_pid_vpscale.z;

	v_side = _scPos.z; //verts falling to the "other" parab get negative here, cullable in PS

	float L = length(_scPos.xyz);
	_scPos = _scPos / L;
	
	_scPos.z = _scPos.z + 1.0;
	_scPos.x = _scPos.x / _scPos.z;
	_scPos.y = _scPos.y / _scPos.z;	
	
	_scPos.z = (L - cam_near_far_pid_vpscale.x) / (cam_near_far_pid_vpscale.y - cam_near_far_pid_vpscale.x);// * sign(v_side); //for proper depth sorting
	
	_scPos += normalize(_scPos)*0.01; //small bias to remove seam
	_scPos.z = _scPos.z * 2.0 - 1.0; // fit to clipspace	
	
	_scPos.w = 1.0;	

    gl_Position = _scPos ;

#else

#ifdef INSTANCING		
	int instance_id = gl_InstanceID + instance_offset;
	mat4 instance_mvp = vp * instance_data[instance_id].model;
	vec4 _scPos = instance_mvp * vec4( in_position, 1.0);
#else
	vec4 _scPos = mvp * vec4( in_position, 1.0);
#endif

#ifdef VELOCITY_BUFFER
	scPos = _scPos ;
	prevScPos = mvp2 * vec4(in_position, 1.0) ; 
#endif
	
    gl_Position = _scPos ;
	
#endif	
	
	vec3 normal = in_normal;
	vec3 tangent = in_tangent;
	decodeFromByteVec3(normal);
	decodeFromByteVec3(tangent);

#ifdef INSTANCING		
	mat4 inv_model_ = instance_data[instance_id].inv_model;
	mat4 model_ = instance_data[instance_id].model;
#else
	mat4 inv_model_ = inv_model;
	mat4 model_ = model;
#endif

	vec4 tmp;
	tmp = vec4( normal, 0.0) * inv_model_;
	v_normal = tmp.xyz;

	v_texcoord0 = in_texcoord0;
	v_texcoord1 = in_texcoord1;
	
	v_worldpos = (model_ * vec4( in_position, 1.0)).xyz;
}

#endif


#ifdef TYPE_fragment

in vec2 v_texcoord0;
in vec2 v_texcoord1;
in vec3 v_normal;
/* highp */ in vec3 v_worldpos;

layout(location = 0) out vec4 frag_color;

#ifdef VELOCITY_BUFFER
layout(location = 1) out VELOCITY_BUFFER_TYPE velocity ;

in vec4 scPos ;
in vec4 prevScPos ;
#endif

#ifdef G_BUFFER_PASS
layout(location = 2) out vec4 frag_normal;
layout(location = 3) out vec4 frag_params;
#endif	

#ifdef PARABOLOID
in float v_side;
#endif

void main()
{		
#ifdef PARABOLOID
	if(v_side < 0.0)
		discard;
#endif

	vec3 albedo = texture( texture_unit0, v_texcoord0).xyz;
	vec4 specCol_emissive = texture( texture_unit2, v_texcoord0);
	vec3 specular_color = specCol_emissive.xyz;
	
	float local_emissive = 0.0;
#ifdef HAS_LOCAL_EMISSIVE //TODO: use material flag (+mat file, editor)
	local_emissive = pow(specCol_emissive.w, 3.0) * 20.0; //give emissive source data non-linear curve, and give range as well
#endif
	float emissive = local_emissive;
	
	//TODO: use baked AO in reflection pass
	//vec3 shadow_ao_emissive = texture( texture_unit1, v_texcoord1).xyz;
	
	//hack: make metals mid-grey in reflection pass
	float specStrength = specular_color.y; //this is greyscale
	albedo = mix(albedo, vec3(0.5,0.5,0.5), pow(specStrength, 0.5)); 	
	
	
#ifdef VELOCITY_BUFFER
	//velocity = pack2FloatToVec4( velocityFunction(scPos,prevScPos) );
	velocity = pack_velocity( velocityFunction(scPos,prevScPos) );
#endif		
	
	vec3 N = normalize(v_normal);
	
	float static_emissive = 0.0;
	float baked_ao = 1.0;
	float baked_shadow = 1.0;
	
	/*highp*/ vec2 uv = v_worldpos.xz / 1950.0;
	uv += vec2(-0.5, 0.5);	
	vec2 baked_ao_shadow = texture(texture_unit7, uv).xy;
	baked_ao = baked_ao_shadow.x;
	baked_shadow = baked_ao_shadow.y;
	
#ifdef G_BUFFER_PASS
	ERROR
#else //dynamic cubemap, simple forward lit path
	vec3 L = global_light_dir.xyz;
	float two_sided_lighting = 0.0;
	
	vec3 view_dir = view_pos - v_worldpos;
	vec3 V = normalize(view_dir);
	
	vec3 diffuse = getPBRdiffuse(N, L, V, two_sided_lighting, baked_shadow, v_worldpos);
	vec3 ambient = getPBRambient(N, carindex_translucency_ssaostr_fovscale.x * 255.0);
	
	vec3 result_color = vec3(emissive) + albedo * (static_emissive + ambient * baked_ao + diffuse);	
	frag_color = RGBtoRGBM_cubemap(result_color);
#endif
}

#endif
