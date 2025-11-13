/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __MTL_TYPES_31_H__
#define __MTL_TYPES_31_H__


struct UBOFrame
{
    KCL::Matrix4x4 shadow_matrix0;
    KCL::Vector4D time_dt_pad2; // time, delta time in seconds
    
    //Post process constants
    KCL::Vector4D ABCD; //ShoulderStrength, LinearStrength, LinearAngle, ToeStrength
    KCL::Vector4D EFW_tau; //ToeNumerator, ToeDenominator, LinearWhite, AdaptationRange
    KCL::Vector4D exposure_bloomthreshold_pad2;
};


#endif // __MTL_TYPES_31_H__

