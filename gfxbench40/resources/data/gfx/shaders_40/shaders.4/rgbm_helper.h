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


vec3 RGBMtoRGB_cubemap( vec4 rgbm ) 
{
	
#ifdef CUBEMAP_FP_RENDERTARGET_ENABLED
	return rgbm.xyz ;
#endif

	vec3 dec = MAX_RANGE_CUBEMAP * rgbm.rgb * rgbm.a;
	
#if CUBEMAP_GAMMA_CORRECTION
	return dec*dec;
#else
	return dec; //from "gamma" to linear
#endif

}


vec4 RGBtoRGBM_cubemap( vec3 rgb )
{
	
#ifdef CUBEMAP_FP_RENDERTARGET_ENABLED
	return vec4( rgb,1.0 ) ;
#endif

#if CUBEMAP_GAMMA_CORRECTION
	vec3 color = vec3( sqrt(rgb.r), sqrt(rgb.g), sqrt(rgb.b) ) ;
#else
   vec3 color = vec3( rgb.r, rgb.g, rgb.b ) ;
#endif

	color *= 1.0 / MAX_RANGE_CUBEMAP ;
	
	float alpha = clamp( ( max( max( rgb.r, rgb.g ), max( rgb.b, 1e-6 ) ) ), 0.0, 1.0);
	
	alpha = float(ceil( alpha * 255.0 )) / 255.0;
	
	color *= 1.0 / alpha;
	
	vec4 res = vec4( color.x, color.y, color.z, alpha) ;
		
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


vec3 RGBDtoRGB_lightcombine( vec4 rgbm ) 
{
#ifdef LIGHTCOMBINE_FP_RENDERTARGET_ENABLED
	return rgbm.xyz ;
#endif

	vec3 dec = rgbm.rgb * ((RGBD_MAX_RANGE / MAX_BYTE_RGBD) / rgbm.a);
	
#if LIGHTCOMBINE_GAMMA_CORRECTION
	return dec*dec; //from "gamma" to linear
#else
	return dec; //from "gamma" to linear
#endif
}

/*highp*/ vec4 RGBtoRGBD_lightcombine(vec3 rgb )
{
#ifdef LIGHTCOMBINE_FP_RENDERTARGET_ENABLED
	return vec4( rgb , 1.0) ;
#endif

#if LIGHTCOMBINE_GAMMA_CORRECTION
	vec3 color = vec3( sqrt(rgb.r), sqrt(rgb.g), sqrt(rgb.b) ) ;
#else
	vec3 color = vec3( rgb.r, rgb.g, rgb.b ) ;
#endif
		
		
	/*highp*/ vec4 res;
	float maxRGB = max(color.x,max(color.g,color.b));
    /*highp*/ float D = max( RGBD_MAX_RANGE / maxRGB, 1.0);
    D                 = clamp(ceil(D) / MAX_BYTE_RGBD, 0.0, 1.0);
	
	D *= RGBD_ALPHA_RANGE ;
	
    res = vec4(color.rgb * (D * (MAX_BYTE_RGBD / RGBD_MAX_RANGE)), D);
	
	//return sat( res );
	
	return res ;
}

vec4 RGBD_transparent_encode(/*highp*/ vec4 rgba)
{
#ifdef LIGHTCOMBINE_FP_RENDERTARGET_ENABLED
	return vec4( rgba.xyz , rgba.a) ;
#else
	return vec4( pow(rgba.xyz * rgba.a, vec3(1.0/2.2)), rgba.a) ; //so source data has more precision in the dark when passing through the G-buffer
#endif
}

