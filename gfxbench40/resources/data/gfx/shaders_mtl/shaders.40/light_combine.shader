/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"

struct VertexInput
{
	hfloat3 in_position [[attribute(0)]];
};

struct VertexOutput
{
	hfloat4 out_position [[position]];
	hfloat2 out_texcoord0;
};

#ifdef TYPE_fragment



//TODO: channel view based on user input
//TODO: reflections with indirect reads - write refl.idx into G-Buffer a switch between
//		runtime updated and static baked ones here

/*
	Inputs:
		0 Albedo 	//albedoRGB, emissive
		1 Normals 	//encoded view space normal
		2 Params 	//specCol, smoothness
		3 Depth
		4 Shadows	//blurred SSAO, blurred dynamic shadow
		+ Envmaps
*/

constexpr sampler sampler0(coord::normalized, filter::nearest, mip_filter::none, address::clamp_to_edge);
constexpr sampler sampler1(coord::normalized, filter::nearest, mip_filter::none, address::clamp_to_edge);
constexpr sampler sampler2(coord::normalized, filter::nearest, mip_filter::none, address::clamp_to_edge);
constexpr sampler sampler4(coord::normalized, min_filter::nearest, mag_filter::linear, mip_filter::linear, address::repeat);
constexpr sampler depth_sampler(coord::normalized, filter::linear, address::clamp_to_edge);

fragment hfloat4 shader_main(VertexOutput input [[stage_in]]
							,constant FrameUniforms * frame_consts [[buffer(FRAME_UNIFORMS_SLOT)]]
							,constant FilterUniforms * filter_consts [[buffer(FILTER_UNIFORMS_SLOT)]]
							,texture2d<hfloat> texture_unit0 [[texture(TEXTURE_UNIT0_SLOT)]]
							,texture2d<_float> texture_unit1 [[texture(TEXTURE_UNIT1_SLOT)]]
							,texture2d<_float> texture_unit2 [[texture(TEXTURE_UNIT2_SLOT)]]
							,texture2d<_float> texture_unit4 [[texture(TEXTURE_UNIT4_SLOT)]]
							,texture2d<_float> texture_unit7 [[texture(TEXTURE_UNIT7_SLOT)]]
							,texture2d_array<_float> envmap1_dp [[texture(ENVMAP1_DP_SLOT)]]
							,texture2d_array<_float> envmap2_dp [[texture(ENVMAP2_DP_SLOT)]]
#ifdef HAS_TEXTURE_CUBE_MAP_ARRAY_EXT
	#ifdef PLATFORM_OSX
							,texturecube_array<_float> static_envmaps [[texture(STATIC_ENVMAPS_SLOT_0)]]
	#else
							,array<texturecube<_float>, NUM_STATIC_ENVMAPS> static_envmaps [[texture(STATIC_ENVMAPS_SLOT_0)]]
	#endif
#endif
							,depth2d<hfloat> depth_unit0 [[texture(DEPTH_UNIT0_SLOT)]]
#ifdef HAS_TEXTURE_CUBE_MAP_ARRAY_EXT
                            ,sampler static_envmaps_sampler [[sampler(STATIC_ENVMAPS_SLOT_0)]]
#endif
                            ,sampler sampler7 [[sampler(TEXTURE_UNIT7_SLOT)]])
{
	//sample
	_float4 albedo_emissive_translucency = _float4(texture_unit0.sample(sampler0, input.out_texcoord0));
	hfloat3 normalVS = decodeNormal(texture_unit1, sampler1, input.out_texcoord0);
	_float4 specCol_envmapIdx_smoothness_ao = texture_unit2.sample(sampler2, input.out_texcoord0); //specCol, envmapIdx, smoothness, ao

	_float2 dynAO_Shadow = texture_unit4.sample(sampler4, input.out_texcoord0).xy;
	_float dynAO = dynAO_Shadow.x;
	_float dynShadow = dynAO_Shadow.y;

	//decode
	_float3 albedo = albedo_emissive_translucency.xyz;
	_float emissive_translucency = albedo_emissive_translucency.w;

	hfloat3 normalWS = normalize(filter_consts->inv_view * hfloat4(normalVS, 0.0)).xyz;

	_float specCol = specCol_envmapIdx_smoothness_ao.x;
	_float roughness = 1.0 - specCol_envmapIdx_smoothness_ao.z;

	_float staticAO = specCol_envmapIdx_smoothness_ao.w;

	_float shadow = dynShadow * dynAO; //HACK: until we spray negative point-lights for every bush, this darkens their core

	//combine
	hfloat3 originWS = getPositionWS(depth_unit0, depth_sampler, input.out_texcoord0, filter_consts->depth_parameters, filter_consts->corners, filter_consts->view_pos); //reads depth
	hfloat3 view_dir = filter_consts->view_pos - originWS;

	_float cubeIndex = specCol_envmapIdx_smoothness_ao.y * 255.0;

	_float3 result_color = getPBRlighting(envmap1_dp,
										envmap2_dp,
#ifdef HAS_TEXTURE_CUBE_MAP_ARRAY_EXT
										static_envmaps,
                                        static_envmaps_sampler,
#endif
										texture_unit7,
										sampler7,
										view_dir, normalWS, roughness, cubeIndex, specCol, albedo, emissive_translucency, staticAO, dynAO, shadow, originWS,
										frame_consts->ground_color, frame_consts->sky_color, frame_consts->global_light_dir, frame_consts->global_light_color, filter_consts->dpcam_view);

	hfloat3 res = applyFog(result_color, length(view_dir), normalize(view_dir), frame_consts->global_light_dir, frame_consts->global_light_color, frame_consts->fogCol);

#ifdef DEBUG_SHADOW
	_float4 shadow_coord; //not used
	int cascade_index = getMapBasedCascadeIndex(_float4(originWS, 1.0), shadow_coord, filter_consts->cascaded_shadow_matrices);
	if (cascade_index = 0)
		res *= _float3(1.0, 0.2, 0.2);
	if (cascade_index = 1)
		res *= _float3(0.2, 1.0, 0.2);
	if (cascade_index = 2)
		res *= _float3(0.2, 0.2, 1.0);
	if (cascade_index = 3)
		res *= _float3(1.0, 0.2, 1.0);
#endif

	return hfloat4(RGBtoRGBD_lightcombine(_float3(res)));
}
#endif

#ifdef TYPE_vertex

vertex VertexOutput shader_main(VertexInput input [[stage_in]])
{
	VertexOutput output;

	output.out_position = _float4(input.in_position, 1.0);
	output.out_texcoord0 = input.in_position.xy * _float2(0.5, -0.5) + 0.5;

	return output;
}

#endif
