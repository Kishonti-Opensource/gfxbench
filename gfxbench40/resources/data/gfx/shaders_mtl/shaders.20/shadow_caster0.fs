/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
_float3 EncodeFloatRGB( _float v ) 
{
	_float3 enc = _float3(1.0, 255.0, 65025.0) * v;
	enc = fract(enc);
	//enc -= enc.yzz * _float3( _float(1.0)/_float(255.0), _float(1.0)/_float(255.0), 0.0);
	enc.xy -= enc.yz * (_float(1.0)/_float(255.0));
	return enc;
}


fragment _float4 shader_main(ShadowCaster0InOut  inFrag    [[ stage_in ]] )
{	
	_float4 result ;
	
    result = _float4( 0.3);
	
	_float4 pos = _float4(inFrag.out_position);

#if defined RGB_ENCODED
	//float d = depth.x / depth.y;
	
	//d = d * _float(0.5) + _float(0.5);	
	
	//gl_FragColor.xyz = EncodeFloatRGB( d);
    result.xyz = EncodeFloatRGB( pos.z / pos.w );
	
#endif

	return result ;
}

