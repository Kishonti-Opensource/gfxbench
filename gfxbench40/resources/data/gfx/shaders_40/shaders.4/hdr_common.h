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

highp vec3 tone_map_filmic_u2_func(highp vec3 x) 
{
	float A = ABCD.x; // ShoulderStrength;
	float B = ABCD.y; // LinearStrength;
	float C = ABCD.z; // LinearAngle;
	float D = ABCD.w; // ToeStrength;
	
	float E = EFW_tau.x; // ToeNumerator;
	float F = EFW_tau.y; // ToeDenominator;
	
	x = ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;	
	return x;
}

highp vec3 tone_map_filmic_u2(highp vec3 x)
{
	
	highp vec3 m = tone_map_filmic_u2_func(x);
	highp float n = 1.0 / exposure_bloomthreshold_tone_map_white_pad.z;

	return n*m;
}

vec3 expose(vec3 c, float threshold) 
{
	
	return exp2(adaptive_avg_pad2.z-threshold) * c;
}


highp vec3 tone_map(highp vec3 color, float threshold)
{
	color = expose(color, threshold) ;
	color = tone_map_filmic_u2(color) ;
	
	return color ;
}

highp vec3 bright_pass(highp vec3 color)
{
/*	float threshold = 1.06;
	float diff = dot(vec3(0.212671, 0.71516, 0.072169), tone_map(color)) - threshold;
	
	if(diff < 0.0) return vec3(0.0);
*/
	

	return tone_map(/*vec3(dot(vec3(0.212671, 0.71516, 0.072169),*/color/*))*/, exposure_bloomthreshold_tone_map_white_pad.y);
}


uniform bool bloom_enabled ;


highp vec3 compose(highp vec3 color, vec3 bloom)
{
	float threshold = 0.0;
	highp vec3 ctm = tone_map(color, threshold);
	
	highp vec3 res = ctm;
	//res += tone_map(bloom);
	//res += bloom * 4.0;
	res += bloom;

	return res;
}
