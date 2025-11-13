/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#include "hdr_consts.h"

// .x - adaptive luminance
// .y - current frame average luminance
// .zw - padding
// hfloat4 adaptive_avg_pad2;

_float getAvgLum(constant hfloat4 *adaptive_avg_pad2)
{
	return adaptive_avg_pad2->x;
}

//=============================================
//
//  HDR Math
//
//=============================================

_float3 tone_map_filmic_u2_func(_float3 x, constant HDRConsts* hdr_consts) 
{
	_float A = hdr_consts->ABCD.x; // ShoulderStrength;
	_float B = hdr_consts->ABCD.y; // LinearStrength;
	_float C = hdr_consts->ABCD.z; // LinearAngle;
	_float D = hdr_consts->ABCD.w; // ToeStrength;
	
	_float E = hdr_consts->EFW_tau.x; // ToeNumerator;
	_float F = hdr_consts->EFW_tau.y; // ToeDenominator;
	
	x = ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;	
	return x ;
}

_float3 tone_map_filmic_u2(_float3 x, constant HDRConsts* hdr_consts) 
{
	_float W = hdr_consts->EFW_tau.z; //LinearWhite
	
	_float3 m = tone_map_filmic_u2_func(x, hdr_consts) ;
	_float3 n = _float3(1.0)/tone_map_filmic_u2_func(_float3(W), hdr_consts);
	
	return n*m;
}

_float3 expose(_float3 c, _float avg_lum, _float threshold, constant HDRConsts* hdr_consts) 
{
	_float val = avg_lum + _float(1.0);
	_float log10val = log(val) / log( _float(10.0) );
	_float middleGrey = _float(1.03) - ( _float(2.0) / ( _float(2.0) + log10val));
    middleGrey *= hdr_consts->exposure_bloomthreshold_minmax_lum.x;
	_float linExposure = middleGrey / avg_lum;
	_float exposure = log2(max(linExposure, _float(0.0001) ));
 
	exposure -= threshold;    
	return exp2(exposure) * c;
}

_float3 tone_map(_float3 color, _float threshold, constant HDRConsts* hdr_consts, constant hfloat4 *adaptive_avg_pad2) 
{
	_float avg_lum = getAvgLum(adaptive_avg_pad2);   
  
    avg_lum = max(avg_lum, _float(hdr_consts->exposure_bloomthreshold_minmax_lum.z) );
    avg_lum = min(avg_lum, _float(hdr_consts->exposure_bloomthreshold_minmax_lum.w) );
	
	color = expose(color, avg_lum, threshold, hdr_consts);	
	color = tone_map_filmic_u2(color, hdr_consts);
	
	return color;
}

_float3 bright_pass(_float3 color, constant HDRConsts* hdr_consts, constant hfloat4 *adaptive_avg_pad2) 
{
	return tone_map(color, hdr_consts->exposure_bloomthreshold_minmax_lum.y, hdr_consts, adaptive_avg_pad2);
}

_float3 compose(_float3 color, _float3 b1, _float3 b2, _float3 b3, _float3 b4, constant HDRConsts* hdr_consts, constant hfloat4 *adaptive_avg_pad2) 
{
	_float threshold = 0.0;
	_float3 ctm = tone_map( RGBA10A2_DECODE(color), threshold, hdr_consts, adaptive_avg_pad2) ;
	_float3 bloom = (b1 + b2 + b3 + b4) * _float(0.25); //0.05 * b1 + 0.1 * b2 + 0.2 * b3 + 0.4 * b4; //TODO: expose bloom magnitude setting
	
	_float3 res = ctm;
	res += powr(bloom, _float3(2.0)) * _float(50.0);
 
    //return ctm; //Disable bloom
	return res;
}
