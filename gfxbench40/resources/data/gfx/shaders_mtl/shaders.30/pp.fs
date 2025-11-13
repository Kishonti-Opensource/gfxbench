/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

struct DepthOfFieldConsts
{
    hfloat4 dof_strengthX_camera_focusY_pad;
};

_float GetLinearDepth(texture2d<hfloat> depthtexture, sampler sam, _float2 texcoords, _float4 depth_parameters)
{
	hfloat d = depthtexture.sample(sam, hfloat2(texcoords) ).x;
	
	d = depth_parameters.y / (d - depth_parameters.x);
	
	return d;
}

_float Dof(texture2d<hfloat> depthtexture, sampler sam, _float2 texcoords, _float4 depth_parameters, _float focus, _float dof_strength)
{
	_float d_s = (dof_strength * _float(0.1));
	
	_float depth = GetLinearDepth(depthtexture, sam, texcoords, depth_parameters);
	
	_float dof = clamp(fabs(log(depth/focus)) / d_s, _float(0.0), _float(1.0) );
	
	dof = powr( dof, _float(0.6) );
	
	return dof;
}

fragment _float4 shader_main(PPVertexOutput in [[stage_in]],
							constant FrameConsts* frameConst [[buffer(10)]],
                            constant DepthOfFieldConsts* dofConst [[buffer(9)]],
							texture2d<_float> unit0 [[texture(0)]],
							texture2d<_float> unit1 [[texture(1)]],
							texture2d<hfloat> unit2 [[texture(2)]],
							sampler sampler0 [[sampler(0)]],
							sampler sampler1 [[sampler(1)]],
							sampler sampler2 [[sampler(2)]])


{
	_float3 c1 = unit1.sample(sampler1, hfloat2(in.texcoord0) ).xyz;
	
	_float f = Dof(unit2, sampler2,
                  in.texcoord0,
                  _float4(frameConst[0].depth_parameters),
                  dofConst[0].dof_strengthX_camera_focusY_pad.y,
                  dofConst[0].dof_strengthX_camera_focusY_pad.x);

	f *= _float(3.0);

	_float3 final = unit0.sample(sampler0, hfloat2(in.texcoord0), bias(f)).xyz;
	final += c1;
	
	return _float4(final, _float(1.0)) ;
}


