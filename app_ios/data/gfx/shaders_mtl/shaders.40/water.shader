/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"
#include "attribute_locations.h"

#ifndef G_BUFFER_PASS
#error - CHECK SHADER DEFINES GIVEN TO COMPILER
#endif

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
	hfloat4 out_position [[position]];
	hfloat2 out_texcoord0;
	hfloat2 out_texcoord1;
	hfloat3 out_normal;
	hfloat3 out_tangent;
	hfloat3 out_worldpos;

#ifdef VELOCITY_BUFFER
	hfloat4 out_scPos;
	hfloat4 out_prevScPos;
#endif
};

struct FragmentOutput
{
	_float4 frag_color	[[color(0)]];
#ifdef VELOCITY_BUFFER
	_float4 velocity	[[color(1)]];
#endif
	_float4 frag_normal	[[color(2)]];
	_float4 frag_params	[[color(3)]];
};

#ifdef TYPE_vertex

/***********************************************************************************/
//                          VERTEX SHADER
/***********************************************************************************/

vertex VertexOutput shader_main(VertexInput input [[stage_in]]
								,constant VertexUniforms * uniforms [[buffer(VERTEX_UNIFORMS_SLOT)]])
{
	VertexOutput output;

	_float3 normal = decodeFromByteVec3(input.in_normal);
	_float3 tangent = decodeFromByteVec3(input.in_tangent);

    _float4 _scPos = uniforms->mvp * _float4(input.in_position, 1.0);
    _float4 world_pos = uniforms->model * _float4(input.in_position, 1.0);

    output.out_worldpos = world_pos.xyz;
    output.out_position = _scPos;

#ifdef VELOCITY_BUFFER
    output.out_scPos = _scPos;
    output.out_prevScPos = uniforms->mvp2 * _float4(input.in_position, 1.0);
#endif

    _float4 tmp;
    tmp = _float4(normal, 0.0) * uniforms->inv_model;
    output.out_normal = tmp.xyz;

    tmp = _float4(tangent, 0.0) * uniforms->inv_model;
	output.out_tangent = tmp.xyz;

    output.out_texcoord0 = input.in_texcoord0;
    output.out_texcoord1 = input.in_texcoord1;

	return output;
}

#endif

#ifdef TYPE_fragment

/***********************************************************************************/
//                          FRAGMENT SHADER
/***********************************************************************************/

fragment FragmentOutput shader_main(VertexOutput input [[stage_in]]
									,constant FrameUniforms * frame_consts [[buffer(FRAME_UNIFORMS_SLOT)]]
									,constant FragmentUniforms * frag_consts [[buffer(FRAGMENT_UNIFORMS_SLOT)]]
									,texture2d<_float> texture_unit3 [[texture(TEXTURE_UNIT3_SLOT)]]
									,texture2d<_float> texture_unit5 [[texture(TEXTURE_UNIT5_SLOT)]]
									,texture2d<_float> texture_unit6 [[texture(TEXTURE_UNIT6_SLOT)]]
									,sampler sampler3 [[sampler(TEXTURE_UNIT3_SLOT)]]
									,sampler sampler5 [[sampler(TEXTURE_UNIT5_SLOT)]]
									,sampler sampler6 [[sampler(TEXTURE_UNIT6_SLOT)]])
{
	FragmentOutput output;

    // View direction
    _float3 V = _float3(normalize(frag_consts->view_pos - input.out_worldpos));

    // Mix the normal vector with some wave like noise
    _float time = frame_consts->time_dt_pad2.x * 0.04;
    _float3 tex_normal = texture_unit3.sample(sampler3, input.out_texcoord0 + hfloat2(0.0, -time)).xyz;
    _float3 tex_normal2 = texture_unit3.sample(sampler3, input.out_texcoord0 + hfloat2(-time, 0.0)).xyz;
    tex_normal = (tex_normal + tex_normal2) * 0.5;
    _float3 N = normalize(calcWorldNormal(tex_normal, input.out_normal, input.out_tangent));
    _float3 NV = _float3(normalize(hfloat4(hfloat3(N), 0) * frag_consts->inv_view).xyz); //view-space normals

    _float3 refract_dir = refract(-V, N, 1.01);
    _float refract_factor = dot(refract_dir, -N);

    // Texture coordinates of the sea floor depth texture
    // 1950 is the half size of the scene
	hfloat2 depth_uv = input.out_worldpos.xz / 1950.0;
    depth_uv += hfloat2(0.5, 0.5);
    _float sea_depth = clamp(1.0 - texture_unit6.sample(sampler6, depth_uv).z,0.0,1.0);

    // The colours of the ocean...
    const _float3 water_green = _float3(9.0, 143.0, 134.0) / 255.0;
    const _float3 water_albedo = _float3(52.0, 98.0, 140.0) / 255.0 / 4.0;
    _float3 seafloor = texture_unit5.sample(sampler5, input.out_texcoord0).xyz;

    // Mix the colours of the ocean...
    _float3 bikini = mix(seafloor, water_green, sea_depth);
    _float3 albedo = mix(bikini, water_albedo, sea_depth);
    albedo = mix(albedo, water_albedo, _float(1.0) - refract_factor);

    output.frag_color = _float4(albedo, 0.0); //albedoRGB, emissives
    output.frag_normal = _float4(encodeNormal(hfloat3(NV))); //encoded view normal
    output.frag_params = _float4((1.0 - dot(N, V)) * 0.5, 100.0 / 255.0, 1.0 , 1.0);  //specCol, envmapIdx, smoothness, ao
#ifdef EDITOR_MODE
    if (editor_mesh_selected == 1)
    {
        output.frag_color = _float4(albedo * editor_select_color , 0.0);
        output.frag_normal = _float4(encodeNormal(NV));
        output.frag_params = _float4(0.0, 0.0, 0.0, 1.0);
    }
#endif

#ifdef VELOCITY_BUFFER
    output.velocity = _float4(pack_velocity(hfloat2(velocityFunction(input.out_scPos,input.out_prevScPos,frame_consts->mb_velocity_min_max_sfactor_pad))));
#endif

	return output;
}

#endif // END OF FRAG SHADER
