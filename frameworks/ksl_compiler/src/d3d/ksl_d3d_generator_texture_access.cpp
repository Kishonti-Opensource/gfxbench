/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_d3d_generator.h"

#include <assert.h>


const std::string KSL_D3D_SAMPLE_2D_DEPTH_NAME = "__ksl_sample_2d_depth__";
const std::string KSL_D3D_SAMPLE_CUBE_DEPTH_NAME = "__ksl_sample_cube_depth__";


bool KSLD3DGenerator::Print_texture(KSLFunctionCallExpressionNode *fce)
{
	KSLVariableExpressionNode* sampler_exp = dynamic_cast<KSLVariableExpressionNode*>(fce->arguments[0]);
	assert(sampler_exp != NULL);

	KSLExpressionNode* texcoord_exp = fce->arguments[1];

	switch (sampler_exp->type.id)
	{
	case KSL_TYPE_SAMPLER_2D:
	case KSL_TYPE_SAMPLER_CUBE:
		VisitExpression(sampler_exp);
		m_result << ".Sample(";
		VisitExpression(sampler_exp);
		m_result << KSL_D3D_SAMPLER_SUFFIX << ", ";
		VisitExpression(texcoord_exp);
		m_result << ")";
		break;

	case KSL_TYPE_SAMPLER_2D_ARRAY: assert(0);
	case KSL_TYPE_SAMPLER_CUBE_ARRAY: assert(0);

		// shadow sampler
	case KSL_TYPE_SAMPLER_2D_SHADOW:
		m_result << KSL_D3D_SAMPLE_2D_DEPTH_NAME << "(";
		VisitExpression(sampler_exp);
		m_result << ", ";
		VisitExpression(sampler_exp);
		m_result << KSL_D3D_SAMPLER_SUFFIX << ", ";
		VisitExpression(texcoord_exp);
		m_result << ")";
		break;
		
	case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW: assert(0);
	case KSL_TYPE_SAMPLER_CUBE_SHADOW:
		m_result << KSL_D3D_SAMPLE_CUBE_DEPTH_NAME << "(";
		VisitExpression(sampler_exp);
		m_result << ", ";
		VisitExpression(sampler_exp);
		m_result << KSL_D3D_SAMPLER_SUFFIX << ", ";
		VisitExpression(texcoord_exp);
		m_result << ")";
		break;

	default:
		assert(0);
		break;
	}

	return true;
}


bool KSLD3DGenerator::Print_textureGather(KSLFunctionCallExpressionNode *fce)
{
	KSLVariableExpressionNode* sampler_exp = dynamic_cast<KSLVariableExpressionNode*>(fce->arguments[0]);
	assert(sampler_exp != NULL);

	KSLExpressionNode* texcoord_exp = fce->arguments[1];

	switch (sampler_exp->type.id)
	{
	case KSL_TYPE_SAMPLER_2D:
		VisitExpression(sampler_exp);
		m_result << ".Gather(";
		VisitExpression(sampler_exp);
		m_result << KSL_D3D_SAMPLER_SUFFIX << ", ";
		VisitExpression(texcoord_exp);
		m_result << ")";
		break;

	case KSL_TYPE_SAMPLER_CUBE: assert(0);
	case KSL_TYPE_SAMPLER_2D_ARRAY: assert(0);
	case KSL_TYPE_SAMPLER_CUBE_ARRAY: assert(0);

		// shadow sampler
	case KSL_TYPE_SAMPLER_2D_SHADOW:
		VisitExpression(sampler_exp);
		m_result << ".GatherCmp(";
		VisitExpression(sampler_exp);
		m_result << KSL_D3D_SAMPLER_SUFFIX << ", ";
		VisitExpression(texcoord_exp);
		m_result << ", ";
		VisitExpression(fce->arguments[2]);
		m_result << ")";
		break;

	case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW: assert(0);
	case KSL_TYPE_SAMPLER_CUBE_SHADOW: assert(0);

	default:
		assert(0);
		break;
	}

	return true;
}



bool KSLD3DGenerator::Print_texelFetch(KSLFunctionCallExpressionNode* fce)
{
	KSLVariableExpressionNode* sampler_exp = dynamic_cast<KSLVariableExpressionNode*>(fce->arguments[0]);
	assert(sampler_exp != NULL);

	KSLExpressionNode* texcoord_exp = fce->arguments[1];

	switch (sampler_exp->type.id)
	{
	case KSL_TYPE_SAMPLER_2D:
		VisitExpression(sampler_exp);
		m_result << ".Load(int3(";
		VisitExpression(texcoord_exp);
		m_result << ", ";
		VisitExpression(fce->arguments[2]);
		m_result << "))";
		break;
	case KSL_TYPE_SAMPLER_2D_ARRAY: assert(0);
	case KSL_TYPE_SAMPLER_CUBE_ARRAY: assert(0);

		// shadow sampler
	case KSL_TYPE_SAMPLER_2D_SHADOW: assert(0);
	case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW: assert(0);
	case KSL_TYPE_SAMPLER_CUBE_SHADOW: assert(0);

	default:
		assert(0);
		break;
	}

	return true;
}


bool KSLD3DGenerator::Print_textureLod(KSLFunctionCallExpressionNode *fce)
{
	KSLVariableExpressionNode* sampler_exp = dynamic_cast<KSLVariableExpressionNode*>(fce->arguments[0]);
	assert(sampler_exp != NULL);

	KSLExpressionNode* texcoord_exp = fce->arguments[1];
	KSLExpressionNode* lod_exp = fce->arguments[2];

	switch (sampler_exp->type.id)
	{
	case KSL_TYPE_SAMPLER_2D:
	case KSL_TYPE_SAMPLER_CUBE:
		VisitExpression(sampler_exp);
		m_result << ".SampleLevel(";
		VisitExpression(sampler_exp);
		m_result << KSL_D3D_SAMPLER_SUFFIX << ", ";
		VisitExpression(texcoord_exp);
		m_result << ", ";
		VisitExpression(lod_exp);
		m_result << ")";
		break;

	case KSL_TYPE_SAMPLER_2D_ARRAY: assert(0);
	case KSL_TYPE_SAMPLER_CUBE_ARRAY: assert(0);

		// shadow sampler
	case KSL_TYPE_SAMPLER_2D_SHADOW: assert(0);
	case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW: assert(0);
	case KSL_TYPE_SAMPLER_CUBE_SHADOW: assert(0);

	default:
		assert(0);
		break;
	}

	return true;
}


bool KSLD3DGenerator::Print_textureLodOffset(KSLFunctionCallExpressionNode *fce)
{
	KSLVariableExpressionNode* sampler_exp = dynamic_cast<KSLVariableExpressionNode*>(fce->arguments[0]);
	assert(sampler_exp != NULL);

	KSLExpressionNode* texcoord_exp = fce->arguments[1];
	KSLExpressionNode* lod_exp = fce->arguments[2];
	KSLExpressionNode* offset_exp = fce->arguments[3];

	switch (sampler_exp->type.id)
	{
	case KSL_TYPE_SAMPLER_2D:
		VisitExpression(sampler_exp);
		m_result << ".SampleLevel(";
		VisitExpression(sampler_exp);
		m_result << KSL_D3D_SAMPLER_SUFFIX << ", ";
		VisitExpression(texcoord_exp);
		m_result << ", ";
		VisitExpression(lod_exp);
		m_result << ", ";
		VisitExpression(offset_exp);
		m_result << ")";
		break;

	case KSL_TYPE_SAMPLER_CUBE: assert(0);
	case KSL_TYPE_SAMPLER_2D_ARRAY: assert(0);
	case KSL_TYPE_SAMPLER_CUBE_ARRAY: assert(0);

		// shadow sampler
	case KSL_TYPE_SAMPLER_2D_SHADOW: assert(0);
	case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW: assert(0);
	case KSL_TYPE_SAMPLER_CUBE_SHADOW: assert(0);

	default:
		assert(0);
		break;
	}

	return true;
}


bool KSLD3DGenerator::PrintTextureAccess(KSLFunctionCallExpressionNode* fce)
{
	bool add_precision_cast = false;
	add_precision_cast |= fce->name == "texture";
	add_precision_cast |= fce->name == "textureFetch";
	add_precision_cast |= fce->name == "textureLod";
	add_precision_cast |= fce->name == "textureLodOffset";
	add_precision_cast &= fce->type.precision != KSL_PRECISION_HIGH;

	if (add_precision_cast)
	{
		m_result << "((" << TypeToString(KSLType(KSL_TYPE_VEC4, fce->type.precision)) << ")(";
	}

	bool s = false;
	if (fce->name == "texture")
	{
		s = Print_texture(fce);
	}
	else if (fce->name == "textureGather")
	{
		return Print_textureGather(fce);
	}
	else if (fce->name == "texelFetch")
	{
		s = Print_texelFetch(fce);
	}
	else if (fce->name == "textureLod")
	{
		s = Print_textureLod(fce);
	}
	else if (fce->name == "textureLodOffset")
	{
		s = Print_textureLodOffset(fce);
	}

	if (s && add_precision_cast)
	{
		m_result << "))";
	}

	return s;
}


void KSLD3DGenerator::PrintSamplingMethods()
{
	m_result << "float " << KSL_D3D_SAMPLE_2D_DEPTH_NAME << "(Texture2D t, SamplerComparisonState s, float3 tc)"; NewLine();
	m_result << "{"; NewLine();
	m_result << "   return t.SampleCmp(s, tc.xy, tc.z); "; NewLine();
	m_result << "}"; NewLine();
	NewLine();

	m_result << "float " << KSL_D3D_SAMPLE_CUBE_DEPTH_NAME << "(TextureCube t, SamplerComparisonState s, float4 tc)"; NewLine();
	m_result << "{"; NewLine();
	m_result << "   return t.SampleCmp(s, tc.xyz, tc.w); "; NewLine();
	m_result << "}"; NewLine();
	NewLine();
}

