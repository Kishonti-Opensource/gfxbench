/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_mtl_generator.h"

#include <assert.h>


bool KSLMetalGenerator::Print_texture(KSLFunctionCallExpressionNode *fce)
{
	KSLVariableExpressionNode* sampler_exp = dynamic_cast<KSLVariableExpressionNode*>(fce->arguments[0]);
	assert(sampler_exp != NULL);
	
	KSLExpressionNode* texcoord_exp = fce->arguments[1];
	
	switch (sampler_exp->type.id)
	{
		case KSL_TYPE_SAMPLER_2D:
			VisitExpression(sampler_exp);
			m_result << ".sample(";
			VisitExpression(sampler_exp);
			m_result<< KSL_METAL_SAMPLER_SUFFIX << ", ";
			VisitExpression(texcoord_exp);
			m_result << ")";
			break;
			
		case KSL_TYPE_SAMPLER_CUBE: assert(0);
		case KSL_TYPE_SAMPLER_2D_ARRAY: assert(0);
		case KSL_TYPE_SAMPLER_CUBE_ARRAY: assert(0);
			
		// shadow sampler
		case KSL_TYPE_SAMPLER_2D_SHADOW:
			m_result << KSL_METAL_SAMPLE_2D_DEPTH_NAME << "(";
			VisitExpression(sampler_exp);
			m_result << ", ";
			VisitExpression(texcoord_exp);
			m_result << ")";
			break;
			
		case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW: assert(0);
		case KSL_TYPE_SAMPLER_CUBE_SHADOW:
			m_result << KSL_METAL_SAMPLE_CUBE_DEPTH_NAME << "(";
			VisitExpression(sampler_exp);
			m_result << ", ";
			VisitExpression(texcoord_exp);
			m_result << ")";
			break;
			
		default:
			assert(0);
			break;
	}

	return true;
}


bool KSLMetalGenerator::Print_textureGather(KSLFunctionCallExpressionNode *fce)
{
	KSLVariableExpressionNode* sampler_exp = dynamic_cast<KSLVariableExpressionNode*>(fce->arguments[0]);
	assert(sampler_exp != NULL);
	
	KSLExpressionNode* texcoord_exp = fce->arguments[1];
	
	switch (sampler_exp->type.id)
	{
		case KSL_TYPE_SAMPLER_2D:
			VisitExpression(sampler_exp);
			m_result << ".gather(";
			VisitExpression(sampler_exp);
			m_result<< KSL_METAL_SAMPLER_SUFFIX << ", ";
			VisitExpression(texcoord_exp);
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


bool KSLMetalGenerator::Print_texelFetch(KSLFunctionCallExpressionNode* fce)
{
    KSLVariableExpressionNode* sampler_exp = dynamic_cast<KSLVariableExpressionNode*>(fce->arguments[0]);
    assert(sampler_exp != NULL);
    
    KSLExpressionNode* texcoord_exp = fce->arguments[1];
    
    switch (sampler_exp->type.id)
    {
        case KSL_TYPE_SAMPLER_2D:
            VisitExpression(sampler_exp);
            m_result << ".read(uint2(";
            VisitExpression(texcoord_exp);
            m_result<<"),";
            VisitExpression(fce->arguments[2]);
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


bool KSLMetalGenerator::Print_textureLod(KSLFunctionCallExpressionNode *fce)
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
			m_result << ".sample(";
			VisitExpression(sampler_exp);
			m_result<< KSL_METAL_SAMPLER_SUFFIX << ", ";
			VisitExpression(texcoord_exp);
			m_result << ", level(";
			VisitExpression(lod_exp);
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


bool KSLMetalGenerator::Print_textureLodOffset(KSLFunctionCallExpressionNode *fce)
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
			m_result << ".sample(";
			VisitExpression(sampler_exp);
			m_result<< KSL_METAL_SAMPLER_SUFFIX << ", ";
			VisitExpression(texcoord_exp);
			m_result << ", level(";
			VisitExpression(lod_exp);
			m_result << "),";
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


bool KSLMetalGenerator::PrintTextureAccess(KSLFunctionCallExpressionNode* fce)
{
	if (fce->name == "texture")
	{
		return Print_texture(fce);
	}
	else if (fce->name == "textureGather")
	{
		return Print_textureGather(fce);
	}
    else if (fce->name == "texelFetch")
    {
        return Print_texelFetch(fce);
    }
	else if (fce->name == "textureLod")
	{
		return Print_textureLod(fce);
	}
	else if (fce->name == "textureLodOffset")
	{
		return Print_textureLodOffset(fce);
	}

	return false;
}

