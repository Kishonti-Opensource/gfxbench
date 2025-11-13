/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//CHEAP SHADER - USED TO RENDER INTO CUBEMAP

#include "common.h"

struct VertexInput
{
	hfloat3 in_position		[[attribute(0)]];
	hfloat3 in_normal		[[attribute(1)]];
	hfloat3 in_tangent		[[attribute(2)]];
	hfloat2 in_texcoord0	[[attribute(3)]];
	hfloat2 in_texcoord1	[[attribute(4)]];
};

struct VertexOutput
{
	hfloat4 v_position [[position]];
	hfloat2 v_texcoord0;
	hfloat2 v_texcoord1;
	hfloat3 v_normal;
	hfloat3 v_worldpos;

#ifdef VELOCITY_BUFFER
	hfloat4 scPos;
	hfloat4 prevScPos;
#endif

	hfloat v_side;
};

struct FragmentOutput
{
	_float4 frag_color	[[color(0)]];
#ifdef VELOCITY_BUFFER
	_float4 velocity	[[color(1)]];
#endif
#ifdef G_BUFFER_PASS
	_float4 frag_normal	[[color(2)]];
	_float4 frag_params	[[color(3)]];
#endif
};

#ifdef TYPE_vertex

vertex VertexOutput shader_main(VertexInput input [[stage_in]]
								,constant VertexUniforms * uniforms [[buffer(VERTEX_UNIFORMS_SLOT)]]
#ifdef INSTANCING
								,device InstanceData * instance_data [[buffer(INSTANCE_UNIFORMS_SLOT)]]
								,uint i_id [[ instance_id ]]
#endif
								)
{
	VertexOutput output;

#ifdef INSTANCING
	_float4x4 instance_mv = uniforms->view * instance_data[i_id].model;
	_float4 _scPos = instance_mv * _float4(input.in_position, 1.0);
#else
	_float4 _scPos = uniforms->mv * _float4(input.in_position, 1.0);
#endif

	_scPos.z = _scPos.z * uniforms->cam_near_far_pid_vpscale.z;

	output.v_side = _scPos.z; //verts falling to the "other" parab get negative here, cullable in PS

	_float L = length(_scPos.xyz);
	_scPos = _scPos / L;
	
	_scPos.z = _scPos.z + 1.0;
	_scPos.x = _scPos.x / _scPos.z;
	_scPos.y = _scPos.y / _scPos.z;	
	
	_scPos.z = (L - uniforms->cam_near_far_pid_vpscale.x) / (uniforms->cam_near_far_pid_vpscale.y - uniforms->cam_near_far_pid_vpscale.x);// * sign(v_side); //for proper depth sorting
	
	_scPos += normalize(_scPos)*0.01; //small bias to remove seam
	
	_scPos.w = 1.0;	

    output.v_position = _scPos;

	_float3 normal = decodeFromByteVec3(input.in_normal);
	_float3 tangent = decodeFromByteVec3(input.in_tangent);

#ifdef INSTANCING
	_float4x4 inv_model_ = instance_data[i_id].inv_model;
	_float4x4 model_ = instance_data[i_id].model;
#else
	_float4x4 inv_model_ = uniforms->inv_model;
	_float4x4 model_ = uniforms->model;
#endif

	_float4 tmp;
	tmp = _float4(normal, 0.0) * inv_model_;
	output.v_normal = tmp.xyz;

	output.v_texcoord0 = input.in_texcoord0;
	output.v_texcoord1 = input.in_texcoord1;
	
	output.v_worldpos = (model_ * _float4(input.in_position, 1.0)).xyz;

	return output;
}

#endif


#ifdef TYPE_fragment

fragment FragmentOutput shader_main(VertexOutput input [[stage_in]]
									,constant FrameUniforms * frame_consts [[buffer(FRAME_UNIFORMS_SLOT)]]
									,constant FragmentUniforms * frag_consts [[buffer(FRAGMENT_UNIFORMS_SLOT)]]
									,texture2d<_float> texture_unit0 [[texture(TEXTURE_UNIT0_SLOT)]]
									,texture2d<_float> texture_unit1 [[texture(TEXTURE_UNIT1_SLOT)]]
									,texture2d<_float> texture_unit2 [[texture(TEXTURE_UNIT2_SLOT)]]
									,texture2d<_float> texture_unit7 [[texture(TEXTURE_UNIT7_SLOT)]]
									,sampler sampler0 [[sampler(TEXTURE_UNIT0_SLOT)]]
									,sampler sampler1 [[sampler(TEXTURE_UNIT1_SLOT)]]
									,sampler sampler2 [[sampler(TEXTURE_UNIT2_SLOT)]]
									,sampler sampler7 [[sampler(TEXTURE_UNIT7_SLOT)]])
{
	FragmentOutput output;

	if(input.v_side < 0.0)
		discard_fragment();

	_float3 albedo = texture_unit0.sample(sampler0, input.v_texcoord0).xyz;
	_float4 specCol_emissive = texture_unit2.sample(sampler2, input.v_texcoord0);
	_float3 specular_color = specCol_emissive.xyz;
	
	_float local_emissive = 0.0;
#ifdef HAS_LOCAL_EMISSIVE //TODO: use material flag (+mat file, editor)
	local_emissive = powr(specCol_emissive.w, _float(3.0)) * 20.0; //give emissive source data non-linear curve, and give range as well
#endif
	_float emissive = local_emissive;
	
	//TODO: use baked AO in reflection pass
	//vec3 shadow_ao_emissive = texture( texture_unit1, v_texcoord1).xyz;
	
	//hack: make metals mid-grey in reflection pass
	_float specStrength = specular_color.y; //this is greyscale
	albedo = mix(albedo, _float3(0.5,0.5,0.5), powr(specStrength, _float(0.5)));
	
	
#ifdef VELOCITY_BUFFER
	velocity = pack_velocity( velocityFunction(scPos,prevScPos,frame_consts->mb_velocity_min_max_sfactor_pad) );
#endif		
	
	_float3 N = _float3(normalize(input.v_normal));
	
	_float static_emissive = 0.0;
	_float baked_ao = 1.0;
	_float baked_shadow = 1.0;
	
	/*highp*/ hfloat2 uv = input.v_worldpos.xz / 1950.0;
	uv += hfloat2(-0.5, 0.5);
	_float2 baked_ao_shadow = texture_unit7.sample(sampler7, uv).xy;
	baked_ao = baked_ao_shadow.x;
	baked_shadow = baked_ao_shadow.y;
	
#ifdef G_BUFFER_PASS
	ERROR
#else //dynamic cubemap, simple forward lit path
	_float3 L = _float3(frame_consts->global_light_dir.xyz);
	_float two_sided_lighting = 0.0;
	
	_float3 view_dir = _float3(frag_consts->view_pos - input.v_worldpos);
	_float3 V = normalize(view_dir);

	_float3 diffuse = getPBRdiffuse(texture_unit7, sampler7, N, L, V, two_sided_lighting, baked_shadow, input.v_worldpos, frame_consts->global_light_color);
	_float3 ambient = getPBRambient(N, frag_consts->carindex_translucency_ssaostr_fovscale.x * 255.0, frame_consts->ground_color, frame_consts->sky_color);

	_float3 result_color = _float3(emissive) + albedo * (static_emissive + ambient * baked_ao + diffuse);
	output.frag_color = RGBtoRGBM_cubemap(result_color);
#endif
	return output;
}

#endif
