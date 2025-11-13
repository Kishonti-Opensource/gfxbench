/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_analyzer.h"

#include <assert.h>

void  KSLAnalyzer::CollectReflectionInfo()
{
	ast->in_attributes.clear();
	ast->out_attributes.clear();

	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vardef = (KSLVariableDefinitionsNode*)n;

			KSLVariable &v = ast->variables[vardef->variables[0].variable_id];

			if (vardef->storage_type == KSL_STORAGE_IN)
			{
				assert(vardef->variables.size() == 1);
				ast->in_attributes.push_back(v);
			}
			else if (vardef->storage_type == KSL_STORAGE_OUT)
			{
				assert(vardef->variables.size() == 1);
				ast->out_attributes.push_back(v);
			}
			else if (vardef->storage_type == KSL_STORAGE_UNIFORM)
			{
				assert(vardef->variables.size() == 1);
				ast->uniforms.push_back(vardef);

				if (v.type.IsArray())
				{
					if (!EvalArraySize(vardef->variables[0].size_expression, vardef->variables[0].array_size))
					{
						ErrorUnableToEvalArraySize(vardef);
						vardef->variables[0].array_size = -1;
					}
				}
			}
			else if (vardef->storage_type == KSL_STORAGE_BUFFER)
			{
				assert(vardef->variables.size() == 1);
				ast->buffers.push_back(vardef);
			}
		}
	}

	for (size_t i = 0; i < ast->buffers.size(); i++)
	{
		KSLVariableDefinitionsNode *vardef = ast->buffers[i];
		if (vardef->bool_attribs.find(KSL_ATTRIB_QUALIFIER_READONLY) != vardef->bool_attribs.end())
		{
			ast->readonly_buffers.push_back(vardef);
		}
	}

	std::vector<KSLImageDefinitionNode*> images;
	ast->CollectImageDefinitions(images, false);

	for (size_t i = 0; i < images.size(); i++)
	{
		KSLImageDefinitionNode *img = images[i];

		if (img->bool_attribs.find(KSL_ATTRIB_QUALIFIER_READONLY) != img->bool_attribs.end())
		{
			ast->readonly_images.push_back(img);
		}
	}
}


bool KSLAnalyzer::CreateReflection(NGL_shader_source_descriptor &ssd)
{
	if (ast == NULL) return false;

	if (m_shader_type == NGL_VERTEX_SHADER)
	{
		for (size_t i = 0; i < ast->in_attributes.size(); i++)
		{
			KSLVariable &v = ast->in_attributes[i];

			NGL_vertex_attrib sra;

			sra.m_semantic = v.orig_name;
			switch (v.type.id)
			{
				case KSL_TYPE_FLOAT: sra.m_format = NGL_R32_FLOAT; break;
				case KSL_TYPE_VEC2:  sra.m_format = NGL_R32_G32_FLOAT; break;
				case KSL_TYPE_VEC3:  sra.m_format = NGL_R32_G32_B32_FLOAT; break;
				case KSL_TYPE_VEC4:  sra.m_format = NGL_R32_G32_B32_A32_FLOAT; break;
				default:
					assert(0);
					break;
			}
			ssd.m_used_vertex_attribs.push_back(sra);
		}
	}

	for (size_t i = 0; i < ast->uniforms.size(); i++)
	{
		KSLVariableDefinitionsNode *vardef = ast->uniforms[i];
		KSLVariable &v = ast->variables[vardef->variables[0].variable_id];

		NGL_shader_uniform su;

		su.m_name = v.orig_name;

		uint32_t type_id = KSL_TYPE_INVALID;
		if (v.type.IsArray())
		{
			type_id = v.type.GetBaseType().id;
			su.m_size = (uint32_t)vardef->variables[0].array_size;
		}
		else
		{
			type_id = v.type.id;
			su.m_size = 1;
		}

		switch (type_id)
		{
		case KSL_TYPE_FLOAT: su.m_format = NGL_FLOAT; break;
		case KSL_TYPE_VEC2:  su.m_format = NGL_FLOAT2; break;
		case KSL_TYPE_VEC4:  su.m_format = NGL_FLOAT4; break;
		case KSL_TYPE_MAT4:  su.m_format = NGL_FLOAT16; break;

		case KSL_TYPE_INT:   su.m_format = NGL_INT; break;
		case KSL_TYPE_INT2:  su.m_format = NGL_INT2; break;
		case KSL_TYPE_INT4:  su.m_format = NGL_INT4; break;

		case KSL_TYPE_UINT:  su.m_format = NGL_UINT; break;
		case KSL_TYPE_UINT2: su.m_format = NGL_UINT2; break;
		case KSL_TYPE_UINT4: su.m_format = NGL_UINT4; break;

		// samplers
		case KSL_TYPE_SAMPLER_2D:
		case KSL_TYPE_SAMPLER_2D_ARRAY:
		case KSL_TYPE_SAMPLER_CUBE:
		case KSL_TYPE_SAMPLER_CUBE_ARRAY:

		//shadow samplers
		case KSL_TYPE_SAMPLER_2D_SHADOW:
		case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW:
		case KSL_TYPE_SAMPLER_CUBE_SHADOW:
		{
			su.m_format = NGL_TEXTURE;
			break;
		}

		case KSL_TYPE_SUBPASS_INPUT:
		{
			continue;
		}

		default:
			printf("shader reflection: unknown uniform type\n");
			assert(0);
		}

		if (v.type.IsSampler())
		{
			ssd.m_used_uniform_textures.push_back(su);
		}
		else
		{
			ssd.m_used_uniforms.push_back(su);
		}	
	}

	for (size_t i = 0; i < ast->buffers.size(); i++)
	{
		KSLVariableDefinitionsNode *vardef = ast->buffers[i];
		KSLVariable &v = ast->variables[vardef->variables[0].variable_id];

		NGL_shader_uniform su;

		su.m_name = v.orig_name;
		su.m_size = 1;
		su.m_format = NGL_BUFFER;
		ssd.m_used_uniform_buffers.push_back(su);
	}

	for (size_t i = 0; i < ast->readonly_buffers.size(); i++)
	{
		KSLVariableDefinitionsNode *vardef = ast->readonly_buffers[i];
		KSLVariable &v = ast->variables[vardef->variables[0].variable_id];

		NGL_shader_uniform su;

		su.m_name = v.orig_name;
		su.m_size = 1;
		su.m_format = NGL_BUFFER;
		ssd.m_used_readonly_buffers.push_back(su);
	}

	for (size_t i = 0; i < ast->readonly_images.size(); i++)
	{
		KSLImageDefinitionNode *img = ast->readonly_images[i];
		KSLVariable &v = ast->variables[img->variable_id];

		NGL_shader_uniform su;

		su.m_name = v.orig_name;
		su.m_size = 1;
		su.m_format = NGL_TEXTURE;
		ssd.m_used_readonly_images.push_back(su);
	}

	return true;
}


bool KSLAnalyzer::EvalArraySize(KSLExpressionNode* e, int64_t &v)
{
	v = -1;
	switch (e->node_type)
	{
	case KSL_NODE_LITERAL_EXPRESSION:
	{
		KSLLiteralExpressionNode* le = (KSLLiteralExpressionNode*)e;
		if ( (le->type.id != KSL_TYPE_INT) && (le->type.id != KSL_TYPE_UINT))
		{
			assert(0);
			return false;
		}
		v = le->int_value;
		return true;
	}

	case KSL_NODE_BINOP_EXPRESSION:
	{
		KSLBinaryExpressionNode* be = (KSLBinaryExpressionNode*)e;
		int64_t e1_v, e2_v;

		if (!EvalArraySize(be->e1, e1_v)) return false;
		if (!EvalArraySize(be->e2, e2_v)) return false;

		switch (be->operation)
		{
		case KSL_BINOP_ADD: v = e1_v + e2_v; break;
		case KSL_BINOP_SUB: v = e1_v - e2_v; break;
		case KSL_BINOP_MUL: v = e1_v * e2_v; break;
		case KSL_BINOP_DIV: v = e1_v / e2_v; break;
		default:
			assert(0);
			return false;
			break;
		}
		return true;
	}

	case KSL_NODE_PARENTHESIS_EXPRESSION:
	{
		KSLParenthesisExpressionNode* pe = (KSLParenthesisExpressionNode*)e;
		return EvalArraySize(pe->expression, v);
	}

	default:
		break;
	}

	assert(0);
	return false;
}

