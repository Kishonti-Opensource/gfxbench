/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#define HDR_REDUCTION_PASS1_UNIFORMS_BFR_SLOT  0
#define HDR_REDUCTION_PASS1_OUTPUT_BFR_SLOT    1

#define HDR_REDUCTION_PASS1_INPUT_TEX_SLOT     0
#define HDR_REDUCTION_PASS1_INPUT_SAMPLER_SLOT 0


struct hdr_reduction_pass1_uniforms
{
	hfloat2 texel_center;  // Half texel size of the original texture
	hfloat2 step_uv;       // Step between the samples
	int2    sample_count;  // Count of samples in a tile
};


#define HDR_REDUCTION_PASS2_UNIFORMS_BFR_SLOT 0
#define HDR_REDUCTION_PASS2_INPUT_BFR_SLOT    1
#define HDR_REDUCTION_PASS2_OUTPUT_BFR_SLOT   2


struct hdr_reduction_pass2_uniforms
{
	hfloat4 time_dt_pad2; // time, delta time in seconds
    hfloat4 EFW_tau;      //ToeNumerator, ToeDenominator, LinearWhite, AdaptationRange
    
	hfloat texture_samples_inv;
    int adaptation_mode; // 0: adaptive, 1 - disabled, 2 - predefined
    hfloat predefined_luminance;
    
    hfloat _pad1 ;
};


