/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
layout(std140) uniform frameConsts
{
    uniform highp mat4 shadow_matrix0;
    uniform mediump vec4 time_dt_pad2; // time, delta time in seconds
    
     //Post process constants
    uniform highp vec4 ABCD; //ShoulderStrength, LinearStrength, LinearAngle, ToeStrength
    uniform highp vec4 EFW_tau; //ToeNumerator, ToeDenominator, LinearWhite, AdaptationRange
    uniform mediump vec4 exposure_bloomthreshold_minmax_lum;
};