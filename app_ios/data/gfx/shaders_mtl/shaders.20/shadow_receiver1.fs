/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
_float DecodeFloatRGB( _float3 rgba ) 
{
	return dot( rgba, _float3(1.0, _float(1.0)/_float(255.0), _float(1.0)/_float(65025.0)) );
}


_float shadow(texture2d<_float> shadow_unit0, sampler shadow_sampler0, _float3 out_texcoord0)
{
#if defined RGB_ENCODED
	_float color0 = DecodeFloatRGB( shadow_unit0.sample(shadow_sampler0, hfloat2(out_texcoord0.xy) ).xyz);
#else
	_float color0 = shadow_unit0.sample(shadow_sampler0, hfloat2(out_texcoord0.xy) ).x;
#endif


	if( out_texcoord0.z < _float(1.0) && out_texcoord0.z > _float(0.0) )
	{
		if( color0 < out_texcoord0.z)
		{
			return _float(0.33);
		}
	}
	return 1.0;
}


fragment _float4 shader_main(ShadowReceiver0InOut  inFrag    [[ stage_in ]],
							texture2d<_float> shadow_unit0 [[texture(4)]],
							sampler shadow_sampler0 [[sampler(4)]] )
{
    _float color0 = shadow(shadow_unit0, shadow_sampler0, inFrag.out_texcoord0);
	_float4 color = _float4(color0, color0, color0, 1.0);

	return color;
}

