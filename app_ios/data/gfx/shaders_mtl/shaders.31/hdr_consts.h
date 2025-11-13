/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
struct HDRConsts
{   
     //Post process constants
    hfloat4 ABCD; //ShoulderStrength, LinearStrength, LinearAngle, ToeStrength
    hfloat4 EFW_tau; //ToeNumerator, ToeDenominator, LinearWhite, AdaptationRange
    hfloat4 exposure_bloomthreshold_minmax_lum;
};

