/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//SKY IS DRAWN TWICE - FIRST, INTO G-BUFFER, BUT ONLY VELOCITY PART, SECOND AFTER LIGHTING BUT BEFORE TONEMAPPING, INTO THE COMBINED LIGHT BUFFER
#include "common.h"
#include "attribute_locations.h"

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
	hfloat2 v_texcoord;
	hfloat3 v_normal;
	hfloat3 v_tangent;
	hfloat3 v_worldpos;

#ifdef VELOCITY_BUFFER
	hfloat4 scPos;
	hfloat4 prevScPos;
#endif

#ifdef PARABOLOID
	hfloat v_side;
#endif
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
								,constant VertexUniforms * uniforms [[buffer(VERTEX_UNIFORMS_SLOT)]])
{
	VertexOutput output;

	_float3 pos = input.in_position;
	
	pos.y -= 100.4;

#ifdef PARABOLOID
	_float4 _scPos = uniforms->mv * _float4( pos, 1.0);

	_scPos.z = _scPos.z * uniforms->cam_near_far_pid_vpscale.z;
		
	output.v_side = _scPos.z ; //verts falling to the "other" parab get negative here, cullable in PS
		
	//TODO handle directions
	_float L = length(_scPos.xyz);
	_scPos = _scPos / L;
	
	
	
	_scPos.z = _scPos.z + 1.0;
	_scPos.x = _scPos.x / _scPos.z;
	_scPos.y = _scPos.y / _scPos.z;	
	
	_scPos.z = (L - uniforms->cam_near_far_pid_vpscale.x) / (uniforms->cam_near_far_pid_vpscale.y - uniforms->cam_near_far_pid_vpscale.x);// * sign(output.v_side); //for proper depth sorting
	
	_scPos += normalize(_scPos)*0.04; //small bias to remove seam
	_scPos.z = _scPos.z * 2.0 - 1.0; // fit to clipspace		
	
	_scPos.w = 1.0;
	
#else
	_float4 _scPos = uniforms->mvp * _float4( pos, 1.0);
#endif

#ifdef VELOCITY_BUFFER
	output.scPos = _scPos ;
	output.prevScPos = uniforms->mvp2 * _float4(pos, 1.0) ;
#endif

    output.v_position = _scPos;
	output.v_position.z = output.v_position.w;

	output.v_texcoord = input.in_texcoord0;
		
	output.v_normal = input.in_normal;
	output.v_tangent = input.in_tangent;

	return output;
}

#endif


#ifdef TYPE_fragment

fragment FragmentOutput shader_main(VertexOutput input [[stage_in]]
									,constant FrameUniforms * frame_consts [[buffer(FRAME_UNIFORMS_SLOT)]]
									,constant FragmentUniforms * frag_consts [[buffer(FRAGMENT_UNIFORMS_SLOT)]]
									,texture2d<_float> texture_unit0 [[texture(TEXTURE_UNIT0_SLOT)]]
									,sampler sampler0 [[sampler(TEXTURE_UNIT0_SLOT)]])
{
	FragmentOutput output;

#ifdef PARABOLOID
	if(input.v_side < 0.0)
		discard_fragment();
#endif

#ifdef VELOCITY_BUFFER
	output.velocity = _float4(pack_velocity(hfloat2(velocityFunction(input.scPos,input.prevScPos,frame_consts->mb_velocity_min_max_sfactor_pad))));
#endif	
	
#ifdef G_BUFFER_PASS //has to be rendered to G-buffer - screen velocity needs updating
	output.frag_color = _float4(0.0, 0.0, 0.0, 0.0); //8888, albedoRGB, emissive - dummy data
	output.frag_normal = _float4(0.0,-1.0,0.0,0.0); //8888, encoded world normal + staticAO + staticShadow - dummy data
	output.frag_params = _float4(0.0,0.0,0.0,0.0); //8888, specCol, smoothness	- dummy data
#else //cubemap and light_combine passes
	_float4 color = texture_unit0.sample(sampler0, input.v_texcoord);
	_float3 skyCol = RGBMtoRGB( color);	
	_float skyLum = getLum(skyCol);
	_float skyBoost = 1.0;// + powr(max(0.0, skyLum - 0.4), 10.0);
	_float3 result_color = skyCol * skyBoost; //HACK: boost sky colors where too dark
		
#ifdef LIGHTCOMBINE_PASS
	output.frag_color = RGBtoRGBD_lightcombine(result_color); //fp16
#else
	output.frag_color = RGBtoRGBM_cubemap(result_color); //fp16
#endif
#endif
	return output;
}

#endif
