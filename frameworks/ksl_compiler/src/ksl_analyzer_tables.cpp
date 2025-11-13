/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_analyzer.h"

void KSLAnalyzer::RegisterBuiltInFunctions()
{
	// abs
	RegisterFunctionWithUniformPrecisions("abs", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("abs", KSL_TYPE_VEC2, KSL_TYPE_VEC2);
	RegisterFunctionWithUniformPrecisions("abs", KSL_TYPE_VEC3, KSL_TYPE_VEC3);

	RegisterFunctionWithUniformPrecisions("ceil", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	
	// clamp
	RegisterFunctionWithUniformPrecisions("clamp", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT, KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("clamp", KSL_TYPE_VEC3, KSL_TYPE_VEC3, KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("clamp", KSL_TYPE_VEC4, KSL_TYPE_VEC4, KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("clamp", KSL_TYPE_VEC3, KSL_TYPE_VEC3, KSL_TYPE_VEC3, KSL_TYPE_VEC3);
	RegisterFunctionWithUniformPrecisions("clamp", KSL_TYPE_VEC4, KSL_TYPE_VEC4, KSL_TYPE_VEC4, KSL_TYPE_VEC4);

	RegisterFunctionWithUniformPrecisions("clamp", KSL_TYPE_INT, KSL_TYPE_INT, KSL_TYPE_INT, KSL_TYPE_INT);


	RegisterFunctionWithUniformPrecisions("cos", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("cross", KSL_TYPE_VEC3, KSL_TYPE_VEC3, KSL_TYPE_VEC3);
	
	// dFdx
	RegisterFunctionWithUniformPrecisions("dFdx", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("dFdx", KSL_TYPE_VEC2, KSL_TYPE_VEC2);
	RegisterFunctionWithUniformPrecisions("dFdx", KSL_TYPE_VEC3, KSL_TYPE_VEC3);
	
	// dFdy
	RegisterFunctionWithUniformPrecisions("dFdy", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("dFdy", KSL_TYPE_VEC2, KSL_TYPE_VEC2);
	RegisterFunctionWithUniformPrecisions("dFdy", KSL_TYPE_VEC3, KSL_TYPE_VEC3);
	
	// dot
	RegisterFunctionWithUniformPrecisions("dot", KSL_TYPE_FLOAT, KSL_TYPE_VEC2, KSL_TYPE_VEC2);
	RegisterFunctionWithUniformPrecisions("dot", KSL_TYPE_FLOAT, KSL_TYPE_VEC3, KSL_TYPE_VEC3);
	RegisterFunctionWithUniformPrecisions("dot", KSL_TYPE_FLOAT, KSL_TYPE_VEC4, KSL_TYPE_VEC4);
	
	RegisterFunctionWithUniformPrecisions("exp", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("exp2", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);

	// floor
	RegisterFunctionWithUniformPrecisions("floor", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("floor", KSL_TYPE_VEC2, KSL_TYPE_VEC2);
	
	// fract
	RegisterFunctionWithUniformPrecisions("fract", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("fract", KSL_TYPE_VEC2, KSL_TYPE_VEC2);
	RegisterFunctionWithUniformPrecisions("fract", KSL_TYPE_VEC3, KSL_TYPE_VEC3);
	RegisterFunctionWithUniformPrecisions("fract", KSL_TYPE_VEC4, KSL_TYPE_VEC4);
	
    ast->functions.push_back(KSLFunction("isinf",      KSLType(KSL_TYPE_BOOL,KSL_PRECISION_NONE), KSLType(KSL_TYPE_FLOAT,KSL_PRECISION_HIGH)));
    ast->functions.push_back(KSLFunction("isnan",      KSLType(KSL_TYPE_BOOL,KSL_PRECISION_NONE), KSLType(KSL_TYPE_FLOAT,KSL_PRECISION_HIGH)));
	
	// length
	RegisterFunctionWithUniformPrecisions("length", KSL_TYPE_FLOAT, KSL_TYPE_VEC2);
	RegisterFunctionWithUniformPrecisions("length", KSL_TYPE_FLOAT, KSL_TYPE_VEC3);
	RegisterFunctionWithUniformPrecisions("length", KSL_TYPE_FLOAT, KSL_TYPE_VEC4);
	
	RegisterFunctionWithUniformPrecisions("log", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("log2", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	
	// max
	RegisterFunctionWithUniformPrecisions("max", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("max", KSL_TYPE_VEC2, KSL_TYPE_VEC2, KSL_TYPE_VEC2);
	RegisterFunctionWithUniformPrecisions("max", KSL_TYPE_VEC3, KSL_TYPE_VEC3, KSL_TYPE_VEC3);
	
	RegisterFunctionWithUniformPrecisions("min", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("min", KSL_TYPE_VEC2, KSL_TYPE_VEC2, KSL_TYPE_VEC2);
	RegisterFunctionWithUniformPrecisions("min", KSL_TYPE_VEC3, KSL_TYPE_VEC3, KSL_TYPE_VEC3);
	
	// mix
	RegisterFunctionWithUniformPrecisions("mix", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT, KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("mix", KSL_TYPE_VEC3, KSL_TYPE_VEC3, KSL_TYPE_VEC3, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("mix", KSL_TYPE_VEC4, KSL_TYPE_VEC4, KSL_TYPE_VEC4, KSL_TYPE_FLOAT);
	
	// normalize
	RegisterFunctionWithUniformPrecisions("normalize", KSL_TYPE_VEC3, KSL_TYPE_VEC3);
	RegisterFunctionWithUniformPrecisions("normalize", KSL_TYPE_VEC4, KSL_TYPE_VEC4);
	
	// pow
	RegisterFunctionWithUniformPrecisions("pow", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("pow", KSL_TYPE_VEC2, KSL_TYPE_VEC2, KSL_TYPE_VEC2);
	RegisterFunctionWithUniformPrecisions("pow", KSL_TYPE_VEC3, KSL_TYPE_VEC3, KSL_TYPE_VEC3);
	RegisterFunctionWithUniformPrecisions("pow", KSL_TYPE_VEC4, KSL_TYPE_VEC4, KSL_TYPE_VEC3);
	
	// smoothstep
	RegisterFunctionWithUniformPrecisions("smoothstep", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT, KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("smoothstep", KSL_TYPE_VEC4, KSL_TYPE_VEC4, KSL_TYPE_VEC4, KSL_TYPE_VEC4);
	
	RegisterFunctionWithUniformPrecisions("sin", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("sqrt", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("reflect", KSL_TYPE_VEC3, KSL_TYPE_VEC3, KSL_TYPE_VEC3);
	RegisterFunctionWithUniformPrecisions("round", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	
	// transpose
	RegisterFunctionWithUniformPrecisions("normalize", KSL_TYPE_MAT3, KSL_TYPE_MAT3);
	RegisterFunctionWithUniformPrecisions("normalize", KSL_TYPE_MAT4, KSL_TYPE_MAT4);

	// trunc
	RegisterFunctionWithUniformPrecisions("trunc", KSL_TYPE_FLOAT, KSL_TYPE_FLOAT);
	RegisterFunctionWithUniformPrecisions("trunc", KSL_TYPE_VEC2, KSL_TYPE_VEC2);
	RegisterFunctionWithUniformPrecisions("trunc", KSL_TYPE_VEC3, KSL_TYPE_VEC3);
	RegisterFunctionWithUniformPrecisions("trunc", KSL_TYPE_VEC4, KSL_TYPE_VEC4);

	if (m_shader_type == NGL_FRAGMENT_SHADER)
	{
		RegisterFunctionWithUniformPrecisions("subpassLoad", KSL_TYPE_VEC4, KSL_TYPE_SUBPASS_INPUT);
	}

	if (m_shader_type == NGL_COMPUTE_SHADER)
	{
		KSLPrecision precisions[3] = { KSL_PRECISION_HIGH, KSL_PRECISION_MEDIUM, KSL_PRECISION_LOW };
		for (uint32_t i = 0; i < 3; i++)
		{
			ast->functions.push_back(KSLFunction("imageStore", KSLType(KSL_TYPE_VOID, KSL_PRECISION_NONE), KSLType(KSL_TYPE_IMAGE2D, precisions[i]),
				KSLType(KSL_TYPE_INT2, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_VEC4, precisions[i])));
		}

		ast->functions.push_back(KSLFunction("workgroupMemoryBarrierAll", KSLType(KSL_TYPE_VOID, KSL_PRECISION_NONE)));
		ast->functions.push_back(KSLFunction("workgroupMemoryBarrierGlobal", KSLType(KSL_TYPE_VOID, KSL_PRECISION_NONE)));
		ast->functions.push_back(KSLFunction("workgroupMemoryBarrierShared", KSLType(KSL_TYPE_VOID, KSL_PRECISION_NONE)));
	}

	RegisterTextureFunctions();
}


void KSLAnalyzer::RegisterTextureFunctions()
{
	KSLPrecision p[3] = { KSL_PRECISION_HIGH, KSL_PRECISION_MEDIUM, KSL_PRECISION_LOW };

	// color samplers: high, half, low precisions
	for (uint32_t i = 0; i < 3; i++)
	{
		// texelFetch
		ast->functions.push_back(KSLFunction("texelFetch", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_2D, p[i]), KSLType(KSL_TYPE_INT2, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_INT, KSL_PRECISION_HIGH)));

		ast->functions.push_back(KSLFunction("texelFetchOffset", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_2D, p[i]), KSLType(KSL_TYPE_INT2, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_INT, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_VEC2, KSL_PRECISION_HIGH)));

		// texture sampling without lod level only allowed in fragment shader
		if (m_shader_type == NGL_FRAGMENT_SHADER)
		{
			// texture; 2D color
			ast->functions.push_back(KSLFunction("texture", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_2D, p[i]), KSLType(KSL_TYPE_VEC2, KSL_PRECISION_HIGH)));
			ast->functions.push_back(KSLFunction("texture", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_2D, p[i]), KSLType(KSL_TYPE_VEC2, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_FLOAT, KSL_PRECISION_HIGH)));

			// texture; 2D array color
			ast->functions.push_back(KSLFunction("texture", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_2D_ARRAY, p[i]), KSLType(KSL_TYPE_VEC3, KSL_PRECISION_HIGH)));
			ast->functions.push_back(KSLFunction("texture", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_2D_ARRAY, p[i]), KSLType(KSL_TYPE_VEC3, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_FLOAT, KSL_PRECISION_HIGH)));

			// texture; cube color
			ast->functions.push_back(KSLFunction("texture", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_CUBE, p[i]), KSLType(KSL_TYPE_VEC3, KSL_PRECISION_HIGH)));
			ast->functions.push_back(KSLFunction("texture", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_CUBE, p[i]), KSLType(KSL_TYPE_VEC3, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_FLOAT, KSL_PRECISION_HIGH)));
		}

		// texture gather
		ast->functions.push_back(KSLFunction("textureGather", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_2D, p[i]), KSLType(KSL_TYPE_VEC2, KSL_PRECISION_HIGH)));
		ast->functions.push_back(KSLFunction("textureGatherOffset", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_2D, p[i]), KSLType(KSL_TYPE_VEC2, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_INT2, KSL_PRECISION_HIGH)));
		ast->functions.push_back(KSLFunction("textureGatherOffsets", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_2D, p[i]), KSLType(KSL_TYPE_VEC2, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_INT2, KSL_PRECISION_HIGH)));

		// textureLod
		ast->functions.push_back(KSLFunction("textureLod", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_2D, p[i]), KSLType(KSL_TYPE_VEC2, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_FLOAT, KSL_PRECISION_HIGH)));
		ast->functions.push_back(KSLFunction("textureLod", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_CUBE, p[i]), KSLType(KSL_TYPE_VEC3, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_FLOAT, KSL_PRECISION_HIGH)));

		ast->functions.push_back(KSLFunction("textureLodOffset", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_2D, p[i]), KSLType(KSL_TYPE_VEC2, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_FLOAT, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_INT2, KSL_PRECISION_HIGH)));
	}

	// shadow samplers: high, half precisions
	for (uint32_t i = 0; i < 2; i++)
	{
		// texture sampling without lod level only allowed in fragment shader
		if (m_shader_type == NGL_FRAGMENT_SHADER)
		{
			// texture shadow
			ast->functions.push_back(KSLFunction("texture", KSLType(KSL_TYPE_FLOAT, p[i]), KSLType(KSL_TYPE_SAMPLER_2D_SHADOW, p[i]), KSLType(KSL_TYPE_VEC3, KSL_PRECISION_HIGH)));
			ast->functions.push_back(KSLFunction("texture", KSLType(KSL_TYPE_FLOAT, p[i]), KSLType(KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW, p[i]), KSLType(KSL_TYPE_VEC4, KSL_PRECISION_HIGH)));
			ast->functions.push_back(KSLFunction("texture", KSLType(KSL_TYPE_FLOAT, p[i]), KSLType(KSL_TYPE_SAMPLER_CUBE_SHADOW, p[i]), KSLType(KSL_TYPE_VEC4, KSL_PRECISION_HIGH)));
		}

		ast->functions.push_back(KSLFunction("textureGather", KSLType(KSL_TYPE_VEC4, p[i]), KSLType(KSL_TYPE_SAMPLER_2D_SHADOW, p[i]), KSLType(KSL_TYPE_VEC2, KSL_PRECISION_HIGH), KSLType(KSL_TYPE_FLOAT, KSL_PRECISION_HIGH)));
	}
}


void KSLAnalyzer::RegisterBuiltInVariables()
{
	if (m_shader_type == NGL_VERTEX_SHADER)
	{
		ast->inbuilt_vertex_position_variable_id = RegisterBuiltInVariable(KSLVariable("gl_Position", KSLType(KSL_TYPE_VEC4,KSL_PRECISION_HIGH)));
		ast->inbuilt_vertex_id_variable_id = RegisterBuiltInVariable(KSLVariable("gl_VertexID", KSLType(KSL_TYPE_INT, KSL_PRECISION_HIGH)));
	}

	if (m_shader_type == NGL_COMPUTE_SHADER)
	{
		ast->inbuilt_compute_globalinvocationid_id   = RegisterBuiltInVariable(KSLVariable("gl_GlobalInvocationID", KSLType(KSL_TYPE_UINT3,KSL_PRECISION_HIGH)));
		ast->inbuilt_compute_localinvocationindex_id = RegisterBuiltInVariable(KSLVariable("gl_LocalInvocationIndex", KSLType(KSL_TYPE_UINT, KSL_PRECISION_HIGH)));
		ast->inbuilt_compute_localinvocationid_id    = RegisterBuiltInVariable(KSLVariable("gl_LocalInvocationID", KSLType(KSL_TYPE_UINT3, KSL_PRECISION_HIGH)));
		ast->inbuilt_compute_workgroupid_id          = RegisterBuiltInVariable(KSLVariable("gl_WorkGroupID", KSLType(KSL_TYPE_UINT3,KSL_PRECISION_HIGH)));
	}

	if (m_shader_type == NGL_FRAGMENT_SHADER)
	{
		ast->inbuilt_fragment_fragcoord   = RegisterBuiltInVariable(KSLVariable("gl_FragCoord", KSLType(KSL_TYPE_VEC4, KSL_PRECISION_HIGH)));
		ast->inbuilt_fragment_frontfacing = RegisterBuiltInVariable(KSLVariable("gl_FrontFacing", KSLType(KSL_TYPE_BOOL, KSL_PRECISION_NONE)));
	}

	for (uint32_t i = 0; i < ast->variables.size(); i++)
	{
		m_active_variable_ids.push_back(i);
	}
}


void KSLAnalyzer::RegisterMultiplications()
{
	for (size_t i = 0; i < KSL_NUM_INBUILT_TYPES; i++)
	{
		memset(m_multiplication_table[i], KSL_TYPE_INVALID, sizeof(KSLBaseType)*KSL_NUM_INBUILT_TYPES);
	}

	m_multiplication_table[KSL_TYPE_MAT2][KSL_TYPE_VEC2] = KSL_TYPE_VEC2;
	m_multiplication_table[KSL_TYPE_VEC2][KSL_TYPE_MAT2] = KSL_TYPE_VEC2;

	m_multiplication_table[KSL_TYPE_MAT3][KSL_TYPE_VEC3] = KSL_TYPE_VEC3;
	m_multiplication_table[KSL_TYPE_VEC3][KSL_TYPE_MAT3] = KSL_TYPE_VEC3;

	m_multiplication_table[KSL_TYPE_MAT4][KSL_TYPE_VEC4] = KSL_TYPE_VEC4;
	m_multiplication_table[KSL_TYPE_VEC4][KSL_TYPE_MAT4] = KSL_TYPE_VEC4;
}


void KSLAnalyzer::RegisterBuiltInTypes()
{
	m_types["void"] = KSLType(KSL_TYPE_VOID, KSL_PRECISION_NONE);

	// Floats
	m_types["float"] = KSLType(KSL_TYPE_FLOAT, KSL_PRECISION_HIGH);
	m_types["half"] = KSLType(KSL_TYPE_FLOAT, KSL_PRECISION_MEDIUM);
	m_types["lowp"] = KSLType(KSL_TYPE_FLOAT, KSL_PRECISION_LOW);

	// Vectors
	m_types["float4"] = KSLType(KSL_TYPE_VEC4, KSL_PRECISION_HIGH);
	m_types["half4"] = KSLType(KSL_TYPE_VEC4, KSL_PRECISION_MEDIUM);
	m_types["lowp4"] = KSLType(KSL_TYPE_VEC4, KSL_PRECISION_LOW);

	m_types["float3"] = KSLType(KSL_TYPE_VEC3, KSL_PRECISION_HIGH);
	m_types["half3"] = KSLType(KSL_TYPE_VEC3, KSL_PRECISION_MEDIUM);
	m_types["lowp3"] = KSLType(KSL_TYPE_VEC3, KSL_PRECISION_LOW);

	m_types["float2"] = KSLType(KSL_TYPE_VEC2, KSL_PRECISION_HIGH);
	m_types["half2"] = KSLType(KSL_TYPE_VEC2, KSL_PRECISION_MEDIUM);
	m_types["lowp2"] = KSLType(KSL_TYPE_VEC2, KSL_PRECISION_LOW);

	// Matrices
	m_types["float4x4"] = KSLType(KSL_TYPE_MAT4, KSL_PRECISION_HIGH);
	m_types["half4x4"] = KSLType(KSL_TYPE_MAT4, KSL_PRECISION_MEDIUM);
	m_types["lowp4x4"] = KSLType(KSL_TYPE_MAT4, KSL_PRECISION_LOW);

	m_types["float3x3"] = KSLType(KSL_TYPE_MAT3, KSL_PRECISION_HIGH);
	m_types["half3x3"] = KSLType(KSL_TYPE_MAT3, KSL_PRECISION_MEDIUM);
	m_types["lowp3x3"] = KSLType(KSL_TYPE_MAT3, KSL_PRECISION_LOW);

	m_types["float2x2"] = KSLType(KSL_TYPE_MAT2, KSL_PRECISION_HIGH);
	m_types["half2x2"] = KSLType(KSL_TYPE_MAT2, KSL_PRECISION_MEDIUM);
	m_types["lowp2x2"] = KSLType(KSL_TYPE_MAT2, KSL_PRECISION_LOW);

	// Integer
	m_types["int"] = KSLType(KSL_TYPE_INT, KSL_PRECISION_HIGH);
	m_types["uint"] = KSLType(KSL_TYPE_UINT, KSL_PRECISION_HIGH);

	m_types["short"] = KSLType(KSL_TYPE_INT, KSL_PRECISION_MEDIUM);
	m_types["ushort"] = KSLType(KSL_TYPE_UINT, KSL_PRECISION_MEDIUM);

	m_types["byte"] = KSLType(KSL_TYPE_INT, KSL_PRECISION_LOW);
	m_types["ubyte"] = KSLType(KSL_TYPE_UINT, KSL_PRECISION_LOW);

	// Integer vectors
	m_types["int4"] = KSLType(KSL_TYPE_INT4, KSL_PRECISION_HIGH);
	m_types["int3"] = KSLType(KSL_TYPE_INT3, KSL_PRECISION_HIGH);
	m_types["int2"] = KSLType(KSL_TYPE_INT2, KSL_PRECISION_HIGH);

	m_types["uint4"] = KSLType(KSL_TYPE_UINT4, KSL_PRECISION_HIGH);
	m_types["uint3"] = KSLType(KSL_TYPE_UINT3, KSL_PRECISION_HIGH);
	m_types["uint2"] = KSLType(KSL_TYPE_UINT2, KSL_PRECISION_HIGH);

	// Short vectors
	m_types["short4"] = KSLType(KSL_TYPE_INT4, KSL_PRECISION_MEDIUM);
	m_types["short3"] = KSLType(KSL_TYPE_INT3, KSL_PRECISION_MEDIUM);
	m_types["short2"] = KSLType(KSL_TYPE_INT2, KSL_PRECISION_MEDIUM);

	m_types["ushort4"] = KSLType(KSL_TYPE_UINT4, KSL_PRECISION_MEDIUM);
	m_types["ushort3"] = KSLType(KSL_TYPE_UINT3, KSL_PRECISION_MEDIUM);
	m_types["ushort2"] = KSLType(KSL_TYPE_UINT2, KSL_PRECISION_MEDIUM);

	// Byte vectors
	m_types["byte4"] = KSLType(KSL_TYPE_INT4, KSL_PRECISION_LOW);
	m_types["byte3"] = KSLType(KSL_TYPE_INT3, KSL_PRECISION_LOW);
	m_types["byte2"] = KSLType(KSL_TYPE_INT2, KSL_PRECISION_LOW);

	m_types["ubyte4"] = KSLType(KSL_TYPE_UINT4, KSL_PRECISION_LOW);
	m_types["ubyte3"] = KSLType(KSL_TYPE_UINT3, KSL_PRECISION_LOW);
	m_types["ubyte2"] = KSLType(KSL_TYPE_UINT2, KSL_PRECISION_LOW);

	// Bool
	m_types["bool"] = KSLType(KSL_TYPE_BOOL, KSL_PRECISION_NONE);
	m_types["bool2"] = KSLType(KSL_TYPE_BOOL2, KSL_PRECISION_NONE);
	m_types["bool3"] = KSLType(KSL_TYPE_BOOL3, KSL_PRECISION_NONE);
	m_types["bool4"] = KSLType(KSL_TYPE_BOOL4, KSL_PRECISION_NONE);

	// samplers
	m_types["sampler2D"] = KSLType(KSL_TYPE_SAMPLER_2D, KSL_PRECISION_NONE);
	m_types["sampler2DArray"] = KSLType(KSL_TYPE_SAMPLER_2D_ARRAY, KSL_PRECISION_NONE);
	m_types["samplerCube"] = KSLType(KSL_TYPE_SAMPLER_CUBE, KSL_PRECISION_NONE);
	m_types["samplerCubeArray"] = KSLType(KSL_TYPE_SAMPLER_CUBE_ARRAY, KSL_PRECISION_NONE);

	// shadow sampler
	m_types["sampler2DShadow"] = KSLType(KSL_TYPE_SAMPLER_2D_SHADOW, KSL_PRECISION_NONE);
	m_types["sampler2DArrayShadow"] = KSLType(KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW, KSL_PRECISION_NONE);
	m_types["samplerCubeShadow"] = KSLType(KSL_TYPE_SAMPLER_CUBE_SHADOW, KSL_PRECISION_NONE);

	// subpass input
	m_types["subpassInput"] = KSLType(KSL_TYPE_SUBPASS_INPUT, KSL_PRECISION_NONE);
}


void KSLAnalyzer::RegisterBuiltInQualifiers()
{
	m_builtin_qualifiers["color"] = KSL_ATTRIB_QUALIFIER_COLOR;
	m_builtin_qualifiers["depth"] = KSL_ATTRIB_QUALIFIER_DEPTH;
	m_builtin_qualifiers["ssbo"] = KSL_ATTRIB_QUALIFIER_SSBO;

	m_builtin_qualifiers["rgba8"] = KSL_ATTRIB_QUALIFIER_RGBA8;
	m_builtin_qualifiers["rgba16f"] = KSL_ATTRIB_QUALIFIER_RGBA16F;
}


uint32_t KSLAnalyzer::RegisterBuiltInVariable(const KSLVariable &v)
{
	uint32_t r = (uint32_t)ast->variables.size();
	ast->variables.push_back(v);
	return r;
}


void KSLAnalyzer::RegisterFunctionWithUniformPrecisions(const std::string &name, const KSLBaseType rt, const KSLBaseType at)
{
	KSLPrecision precisions[3] = { KSL_PRECISION_HIGH, KSL_PRECISION_MEDIUM, KSL_PRECISION_LOW };
	for (uint32_t i = 0; i < 3; i++)
	{
		ast->functions.push_back(KSLFunction(name, KSLType(rt, precisions[i]), KSLType(at, precisions[i])));
	}
}


void KSLAnalyzer::RegisterFunctionWithUniformPrecisions(const std::string &name, const KSLBaseType rt, const KSLBaseType at1, const KSLBaseType at2)
{
	KSLPrecision p[3] = { KSL_PRECISION_HIGH, KSL_PRECISION_MEDIUM, KSL_PRECISION_LOW };
	for (uint32_t i = 0; i < 3; i++)
	{
		ast->functions.push_back(KSLFunction(name, KSLType(rt, p[i]), KSLType(at1, p[i]), KSLType(at2, p[i])));
	}
}


void KSLAnalyzer::RegisterFunctionWithUniformPrecisions(const std::string &name, const KSLBaseType rt, const KSLBaseType at1, const KSLBaseType at2, const KSLBaseType at3)
{
	KSLPrecision p[3] = { KSL_PRECISION_HIGH, KSL_PRECISION_MEDIUM, KSL_PRECISION_LOW };
	for (uint32_t i = 0; i < 3; i++)
	{
		ast->functions.push_back(KSLFunction(name, KSLType(rt, p[i]), KSLType(at1, p[i]), KSLType(at2, p[i]), KSLType(at3, p[i])));
	}
}

