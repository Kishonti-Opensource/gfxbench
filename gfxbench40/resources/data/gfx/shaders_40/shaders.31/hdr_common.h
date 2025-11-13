/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ubo_frame.h"
#include "ubo_luminance.h"

float getAvgLum()
{
	return adaptive_avg_pad2.x;
}

//=============================================
//
//  HDR Math
//
//=============================================

vec3 tone_map_filmic_u2_func(vec3 x) 
{
	float A = ABCD.x; // ShoulderStrength;
	float B = ABCD.y; // LinearStrength;
	float C = ABCD.z; // LinearAngle;
	float D = ABCD.w; // ToeStrength;
	
	float E = EFW_tau.x; // ToeNumerator;
	float F = EFW_tau.y; // ToeDenominator;
	
	x = ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;	
	return x ;
}

vec3 tone_map_filmic_u2(vec3 x) 
{
	float W = EFW_tau.z; //LinearWhite
	
	vec3 m = tone_map_filmic_u2_func(x) ;
	vec3 n = vec3(1.0)/tone_map_filmic_u2_func(vec3(W));
	
	return n*m;
}

vec3 expose(vec3 c, float avg_lum, float threshold) 
{
	float val = avg_lum + 1.0;
	float log10val = log(val) / log(10.0);
	float middleGrey = 1.03 - (2.0 / (2.0 + log10val));
    middleGrey *= exposure_bloomthreshold_minmax_lum.x;
	float linExposure = middleGrey / avg_lum;
	float exposure = log2(max(linExposure, 0.0001));
 
	exposure -= threshold;    
	return exp2(exposure) * c;
}

vec3 tone_map(vec3 color, float threshold) 
{
	float avg_lum = getAvgLum();   
  
    avg_lum = max(avg_lum, exposure_bloomthreshold_minmax_lum.z);
    avg_lum = min(avg_lum, exposure_bloomthreshold_minmax_lum.w);
	
	color = expose(color, avg_lum, threshold);	
	color = tone_map_filmic_u2(color);
	
	return color;
}

vec3 bright_pass(vec3 color) 
{
	return tone_map(color, exposure_bloomthreshold_minmax_lum.y);
}

vec3 compose(vec3 color, vec3 b1, vec3 b2, vec3 b3, vec3 b4) 
{
	float threshold = 0.0;
	vec3 ctm = tone_map(color * 4.0, threshold) ;
	vec3 bloom = (b1 + b2 + b3 + b4) * 0.25; //0.05 * b1 + 0.1 * b2 + 0.2 * b3 + 0.4 * b4; //TODO: expose bloom magnitude setting
	
	vec3 res = ctm;
	res += pow(bloom, vec3(2.0)) * 50.0;
 
    //return ctm; //Disable bloom
	return res;
}
