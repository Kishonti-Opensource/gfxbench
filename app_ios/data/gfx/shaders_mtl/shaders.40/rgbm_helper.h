/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//
//	Dynamic cubemap encoding
//
//


#define MAX_RANGE_CUBEMAP 6.0
#define CUBEMAP_GAMMA_CORRECTION 0 


_float3 RGBMtoRGB_cubemap( _float4 rgbm )
{
	
#ifdef CUBEMAP_FP_RENDERTARGET_ENABLED
	return rgbm.xyz ;
#endif

	_float3 dec = MAX_RANGE_CUBEMAP * rgbm.rgb * rgbm.a;
	
#if CUBEMAP_GAMMA_CORRECTION
	return dec*dec;
#else
	return dec; //from "gamma" to linear
#endif

}


_float4 RGBtoRGBM_cubemap( _float3 rgb )
{
	
#ifdef CUBEMAP_FP_RENDERTARGET_ENABLED
	return _float4( rgb,1.0 ) ;
#endif

#if CUBEMAP_GAMMA_CORRECTION
	_float3 color = _float3( sqrt(rgb.r), sqrt(rgb.g), sqrt(rgb.b) ) ;
#else
   _float3 color = _float3( rgb.r, rgb.g, rgb.b ) ;
#endif

	color *= 1.0 / MAX_RANGE_CUBEMAP ;
	
	_float alpha = clamp( ( max( max( rgb.r, rgb.g ), max( rgb.b, _float(1e-6) ) ) ), _float(0.0), _float(1.0));
	
	alpha = _float(ceil( alpha * 255.0 )) / 255.0;
	
	color *= 1.0 / alpha;
	
	_float4 res = _float4( color.x, color.y, color.z, alpha) ;
		
	//return sat(res) ;
		
	return res ;	
}


//
//
//	Blendable RGBD encode for lightcombine
//
//

#define LIGHTCOMBINE_GAMMA_CORRECTION 1 

#define RGBD_MAX_RANGE 22.0
#define RGBD_ALPHA_RANGE 0.9

#define MAX_BYTE_RGBD 255.0


_float3 RGBDtoRGB_lightcombine( _float4 rgbm )
{
#ifdef LIGHTCOMBINE_FP_RENDERTARGET_ENABLED
	return rgbm.xyz ;
#endif

	_float3 dec = rgbm.rgb * ((RGBD_MAX_RANGE / MAX_BYTE_RGBD) / rgbm.a);
	
#if LIGHTCOMBINE_GAMMA_CORRECTION
	return dec*dec; //from "gamma" to linear
#else
	return dec; //from "gamma" to linear
#endif
}

/*highp*/ _float4 RGBtoRGBD_lightcombine(_float3 rgb )
{
#ifdef LIGHTCOMBINE_FP_RENDERTARGET_ENABLED
	return _float4( rgb , 1.0) ;
#endif

#if LIGHTCOMBINE_GAMMA_CORRECTION
	_float3 color = _float3( sqrt(rgb.r), sqrt(rgb.g), sqrt(rgb.b) ) ;
#else
	_float3 color = _float3( rgb.r, rgb.g, rgb.b ) ;
#endif
		
		
	/*highp*/ _float4 res;
	hfloat maxRGB = hfloat(max(color.x,max(color.g,color.b)));
    /*highp*/ hfloat D = max( RGBD_MAX_RANGE / maxRGB, 1.0);
    D                 = clamp(ceil(D) / MAX_BYTE_RGBD, 0.0, 1.0);
	
	D *= RGBD_ALPHA_RANGE ;
	
    res = _float4(color.rgb * (D * (MAX_BYTE_RGBD / RGBD_MAX_RANGE)), D);
	
	//return sat( res );
	
	return res ;
}

_float4 RGBD_transparent_encode(/*highp*/ _float4 rgba)
{
#ifdef LIGHTCOMBINE_FP_RENDERTARGET_ENABLED
	return _float4( rgba.xyz , rgba.a) ;
#else
	return _float4( powr(rgba.xyz * rgba.a, _float3(1.0/2.2)), rgba.a) ; //so source data has more precision in the dark when passing through the G-buffer
#endif
}

