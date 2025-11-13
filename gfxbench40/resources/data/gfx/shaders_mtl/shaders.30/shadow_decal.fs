/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

_float3 get_world_pos(texture2d<hfloat> tex, sampler sam, _float2 out_texcoord, _float4 view_dir, constant FrameConsts* frameConsts)
{
	hfloat d = tex.sample(sam, hfloat2(out_texcoord) ).x;
	
	d = frameConsts->depth_parameters.y / (d - frameConsts->depth_parameters.x);
	
	return d * view_dir.xyz / view_dir.w + _float3(frameConsts->view_posXYZ_time.xyz);
}

//TODO: FIXME. Currently PCF is bugged on shipping hardware. This is fixed in the next seed.
constexpr sampler shadowsamp(coord::normalized, filter::linear, address::clamp_to_edge, compare_func::less_equal);

fragment _float4 shader_main(LightingVertexOutput input [[stage_in]],
							constant FrameConsts* frameConst [[buffer(10)]],
							constant MeshConsts* meshConst [[buffer(12)]],
							constant LightConsts* lightConst [[buffer(13)]],
							texture2d<_float> unit0 [[texture(0)]],
							texture2d<_float> unit1 [[texture(1)]],
							texture2d<hfloat> unit3 [[texture(2)]],
							texture2d<_float> unit4 [[texture(3)]],
                            // Front end busted
                            depth2d<hfloat> shadow_unit0 [[texture(4)]],
							sampler sam0 [[sampler(0)]],
							sampler sam1 [[sampler(1)]],
							sampler sam3 [[sampler(2)]],
							sampler sam4 [[sampler(3)]],
							sampler shadowSampler [[sampler(4)]])

{
	_float4 frag_color;
	const _float inv_res0 = _float(1.0) /  _float(1024.0);
	const _float inv_res1 =  _float(0.707) /  _float(1024.0);
	const _float shadow_strength =  _float(0.25);
	
	_float shadow = 1.0;
	
	_float2 out_texcoord = (input.out_pos.xy / input.out_pos.w * _float(0.5)) + _float2(0.5,0.5);
	
	//invert y coord because we're reading the rendered depth
	out_texcoord.y = _float(1.0f) - out_texcoord.y;
	
	_float3 view_dir = normalize(input.view_dir.xyz);
	
	//TODO: bug here? should use normalized view dir?
	_float3 position = get_world_pos(unit3, sam3,out_texcoord, input.view_dir, frameConst);
    
	hfloat4 p = hfloat4( hfloat3(position), 1.0);
	
	hfloat4 shadow_texcoord = hfloat4x4(lightConst[0].shadowMatrix0) * p;
	
	//project
	shadow_texcoord.xyz = shadow_texcoord.xyz / shadow_texcoord.w;
	
	hfloat refvalue = clamp(shadow_texcoord.z, hfloat(0.0), hfloat(1.0) );
	
    shadow = shadow_unit0.sample_compare(shadowsamp, hfloat2(shadow_texcoord.xy), refvalue);
//	float shadowDist = shadow_unit0.sample(shadowSampler, shadow_texcoord.xy).x;
//	
//	if(refvalue-0.001 > shadowDist)
//	{
//		shadow = 0.0;
//	}
	
	frag_color = mix(_float4(0.25),_float4(1.0),shadow);
	
	return frag_color;
}


