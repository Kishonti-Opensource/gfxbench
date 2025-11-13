/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <metal_stdlib>

using namespace metal;

//
//
//	float type redefinitons
//
//

#ifndef FORCE_HIGHP
#error please define the float precision
#endif

#ifndef VARYING_HIGHP
#error please define the varying precision
#endif

//
//	32 bit float
//
typedef float    hfloat  ;
typedef float2   hfloat2 ;
typedef float3   hfloat3 ;
typedef float4   hfloat4 ;

typedef float2x2 hfloat2x2 ;
typedef float2x3 hfloat2x3 ;
typedef float2x4 hfloat2x4 ;

typedef float3x2 hfloat3x2 ;
typedef float3x3 hfloat3x3 ;
typedef float3x4 hfloat3x4 ;

typedef float4x2 hfloat4x2 ;
typedef float4x3 hfloat4x3 ;
typedef float4x4 hfloat4x4 ;


//
//	16 bit float
//
typedef half   mfloat  ;
typedef half2  mfloat2 ;
typedef half3  mfloat3 ;
typedef half4  mfloat4 ;

typedef half2x2 mfloat2x2  ;
typedef half2x3 mfloat2x3  ;
typedef half2x4 mfloat2x4  ;

typedef half3x2 mfloat3x2  ;
typedef half3x3 mfloat3x3  ;
typedef half3x4 mfloat3x4  ;

typedef half4x2 mfloat4x2  ;
typedef half4x3 mfloat4x3  ;
typedef half4x4 mfloat4x4  ;


//
//	dynamic type
//
#if FORCE_HIGHP

typedef float    _float  ;
typedef float2   _float2 ;
typedef float3   _float3 ;
typedef float4   _float4 ;

typedef float2x2 _float2x2 ;
typedef float2x3 _float2x3 ;
typedef float2x4 _float2x4 ;

typedef float3x2 _float3x2 ;
typedef float3x3 _float3x3 ;
typedef float3x4 _float3x4 ;

typedef float4x2 _float4x2 ;
typedef float4x3 _float4x3 ;
typedef float4x4 _float4x4 ;

#else

typedef  half    _float  ;
typedef  half2   _float2 ;
typedef  half3   _float3 ;
typedef  half4   _float4 ;

typedef  half2x2 _float2x2 ;
typedef  half2x3 _float2x3 ;
typedef  half2x4 _float2x4 ;

typedef  half3x2 _float3x2 ;
typedef  half3x3 _float3x3 ;
typedef  half3x4 _float3x4 ;

typedef  half4x2 _float4x2 ;
typedef  half4x3 _float4x3 ;
typedef  half4x4 _float4x4 ;

#endif


//
//	dynamic type
//
#if VARYING_HIGHP

typedef float    v_float  ;
typedef float2   v_float2 ;
typedef float3   v_float3 ;
typedef float4   v_float4 ;

typedef float2x2 v_float2x2 ;
typedef float2x3 v_float2x3 ;
typedef float2x4 v_float2x4 ;

typedef float3x2 v_float3x2 ;
typedef float3x3 v_float3x3 ;
typedef float3x4 v_float3x4 ;

typedef float4x2 v_float4x2 ;
typedef float4x3 v_float4x3 ;
typedef float4x4 v_float4x4 ;

#else

typedef  half    v_float  ;
typedef  half2   v_float2 ;
typedef  half3   v_float3 ;
typedef  half4   v_float4 ;

typedef  half2x2 v_float2x2 ;
typedef  half2x3 v_float2x3 ;
typedef  half2x4 v_float2x4 ;

typedef  half3x2 v_float3x2 ;
typedef  half3x3 v_float3x3 ;
typedef  half3x4 v_float3x4 ;

typedef  half4x2 v_float4x2 ;
typedef  half4x3 v_float4x3 ;
typedef  half4x4 v_float4x4 ;

#endif



//
//	error for the inbuilt types
//
 
#define float  PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES 
#define float2 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define float3 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define float4 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES

#define float2x2 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define float2x3 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define float2x4 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES

#define float3x2 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define float3x3 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define float3x4 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES

#define float4x2 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define float4x3 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define float4x4 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES

#define half  PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define half2 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define half3 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define half4 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES

#define half2x2 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define half2x3 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define half2x4 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES

#define half3x2 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define half3x3 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define half3x4 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES

#define half4x2 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define half4x3 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
#define half4x4 PLEASE_USE_ONE_OF_THE_PREDEFINED_TYPES
