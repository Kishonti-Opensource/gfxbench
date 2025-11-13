/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ubo_bindings.h"
layout(std140, binding = FRAME_CONSTS_BINDING_SLOT) uniform UBlockFrame
{
    uniform mediump vec4 time_dt_pad2; // time, delta time in seconds
    
    uniform mediump vec4 global_light_dir;
    uniform mediump vec4 global_light_color;

    // Motion blur 
    // Min - max velocity values, sfactor is [sample_count / max_velocity] used by pp_motion_blur.shader
    uniform highp vec4 mb_velocity_min_max_sfactor_pad;
    
    //Post process constants
    uniform highp vec4 ABCD; //ShoulderStrength, LinearStrength, LinearAngle, ToeStrength
    uniform highp vec4 EFW_tau; //ToeNumerator, ToeDenominator, LinearWhite, AdaptationRange

    uniform mediump vec4 fogCol;
    uniform mediump vec4 sky_color;
    uniform mediump vec4 ground_color;
    
    uniform highp vec4 exposure_bloomthreshold_tone_map_white_pad;

    uniform mediump vec4 ambient_colors[54];
};
