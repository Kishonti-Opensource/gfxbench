/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "hdr_consts40.h"

_float getAvgLum(constant hfloat4 * adaptive_avg_pad2)
{
	return adaptive_avg_pad2->x;
}

//=============================================
//
//  HDR Math
//
//=============================================

hfloat3 tone_map_filmic_u2_func(hfloat3 x, constant HDRConsts * hdr_consts)
{
	_float A = hdr_consts->ABCD.x; // ShoulderStrength;
	_float B = hdr_consts->ABCD.y; // LinearStrength;
	_float C = hdr_consts->ABCD.z; // LinearAngle;
	_float D = hdr_consts->ABCD.w; // ToeStrength;
	
	_float E = hdr_consts->EFW_tau.x; // ToeNumerator;
	_float F = hdr_consts->EFW_tau.y; // ToeDenominator;
	
	x = ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;	
	return x;
}

hfloat3 tone_map_filmic_u2(hfloat3 x, constant HDRConsts * hdr_consts)
{
	
	hfloat3 m = tone_map_filmic_u2_func(x, hdr_consts);
	hfloat n = 1.0 / hdr_consts->exposure_bloomthreshold_tone_map_white_pad.z;

	return n*m;
}

_float3 expose(_float3 c, _float threshold, constant hfloat4 * adaptive_avg_pad2)
{
	
	return exp2(adaptive_avg_pad2->z-threshold) * c;
}


hfloat3 tone_map(hfloat3 color, _float threshold, constant HDRConsts * hdr_consts, constant hfloat4 * adaptive_avg_pad2)
{
	color = hfloat3(expose(_float3(color), threshold, adaptive_avg_pad2));
	color = tone_map_filmic_u2(color, hdr_consts) ;
	
	return color ;
}

hfloat3 bright_pass(hfloat3 color, constant HDRConsts * hdr_consts, constant hfloat4 * adaptive_avg_pad2)
{
	return tone_map(color, hdr_consts->exposure_bloomthreshold_tone_map_white_pad.y, hdr_consts, adaptive_avg_pad2);
}



hfloat3 compose(hfloat3 color, hfloat3 bloom, constant HDRConsts * hdr_consts, constant hfloat4 * adaptive_avg_pad2)
{
	_float threshold = 0.0;
	hfloat3 ctm = tone_map(color, threshold, hdr_consts, adaptive_avg_pad2);
	
	hfloat3 res = ctm;
	//res += tone_map(bloom);
	//res += bloom * 4.0;
	res += bloom;

	return res;
}
