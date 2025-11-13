/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#include "pp2.h"

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

constexpr sampler lin_sampler(coord::normalized, filter::linear, address::clamp_to_edge);

fragment _float4 shader_main(PPVertexOutput in [[stage_in]],
							constant FrameConsts* frameConst [[buffer(10)]],
                            constant DepthOfFieldConsts* dofConst [[buffer(9)]],
							texture2d<_float> unit0 [[ texture(PP2_MAIN_COLOR_TEX_SLOT)   ]],
							texture2d<hfloat> unit2 [[ texture(PP2_DEPTH_TEX_SLOT)        ]],
							texture2d<_float> unit3 [[ texture(PP2_BLURED_COLOR_TEX_SLOT) ]])


{	
	_float3 final;
	_float3 c0 = unit0.sample(lin_sampler, hfloat2(in.texcoord0)).xyz;

#define DOF_ENABLED 1

#if DOF_ENABLED
	_float3 c3 = unit3.sample(lin_sampler, hfloat2(in.texcoord0)).xyz;
	
	_float f = Dof(unit2, lin_sampler,
                  in.texcoord0,
                  _float4(frameConst[0].depth_parameters),
                  dofConst[0].dof_strengthX_camera_focusY_pad.y,
                  dofConst[0].dof_strengthX_camera_focusY_pad.x);
	
	final = mix(c0,c3, clamp( _float(2.0)*f, _float(0.0), _float(1.0) ) );
#else
	final = c0;
#endif
	
	return _float4(final, _float(1.0)) ;
}


