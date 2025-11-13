/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_gl_generator_base.h"

#include "ksl_gl_translator.h"

#include <assert.h>


KSLGLGeneratorBase::KSLGLGeneratorBase()
{
	Clear();
}


KSLGLGeneratorBase::~KSLGLGeneratorBase()
{
	Clear();
}


void KSLGLGeneratorBase::Clear()
{
}


bool KSLGLGeneratorBase::PrintDefinitionLayout(const std::set<uint32_t> &bool_attribs, const std::map<uint32_t, uint32_t> int_attribs)
{
	size_t attrib_count = bool_attribs.size() + int_attribs.size();

	if (attrib_count > 0)
	{
		m_result << "layout( ";

		size_t i = 0;

		std::set<uint32_t>::const_iterator b_it = bool_attribs.begin();
		for (; b_it != bool_attribs.end(); b_it++)
		{
			m_result << TranslateAttribQualifer(*b_it);
			i++;
			if (i < attrib_count) m_result << ", ";
		}

		std::map<uint32_t, uint32_t>::const_iterator i_it = int_attribs.begin();
		for (; i_it != int_attribs.end(); i_it++)
		{
			m_result << TranslateAttribQualifer(i_it->first) << " = " << i_it->second;
			i++;
			if (i < attrib_count) m_result << ", ";
		}

		m_result << " ) ";
	}
	return true;
}


bool KSLGLGeneratorBase::PrintBufferDefinition(KSLVariableDefinitionsNode *vardef)
{
	SyncLine(vardef);

	if (vardef->variables.size() > 1)
	{
		assert(0);
		return false;
	}
	
	KSLVariableDefinitionsNode::InnerNode &dn = vardef->variables[0];
	KSLVariable &v = ast->variables[dn.variable_id];

	if (dn.init_expressions.size() != 0)
	{
		assert(0);
		return false;
	}

	// print buffer layout
	bool is_ubo = (vardef->bool_attribs.find(KSL_ATTRIB_QUALIFIER_SSBO) == vardef->bool_attribs.end());
	vardef->bool_attribs.erase(KSL_ATTRIB_QUALIFIER_SSBO);

	// readonly
	bool is_readonly = (vardef->bool_attribs.find(KSL_ATTRIB_QUALIFIER_READONLY) != vardef->bool_attribs.end());
	vardef->bool_attribs.erase(KSL_ATTRIB_QUALIFIER_READONLY);

	vardef->bool_attribs.insert(is_ubo ? KSL_ATTRIB_QUALIFIER_GL_STD140 : KSL_ATTRIB_QUALIFIER_GL_STD430);

	PrintDefinitionLayout(vardef->bool_attribs, vardef->int_attribs);

	if (is_readonly) m_result << "readonly ";

	m_result << (is_ubo ? "uniform " : "buffer ") << v.orig_name;
	NewLine();

	m_result << "{";
	m_indent++;
	NewLine();

	m_result << TypeToString(vardef->variable_type) << " " << v.new_name;

	if (v.type.IsArray())
	{
		if ((dn.size_expression == NULL) && (vardef->storage_type != KSL_STORAGE_BUFFER))
		{
			assert(0);
			return false;
		}

		m_result << "[";
		if (dn.size_expression != NULL) VisitExpression(dn.size_expression);
		m_result << "]";
	}
	m_result << ";";

	m_indent--;
	NewLine();
	m_result << "};";
	NewLine();

	return true;
}


bool KSLGLGeneratorBase::VisitVariableDefinitions(KSLVariableDefinitionsNode *vardef)
{
	SyncLine(vardef);

	if (vardef->storage_type == KSL_STORAGE_BUFFER)
	{
		return PrintBufferDefinition(vardef);
	}

	PrintDefinitionLayout(vardef->bool_attribs, vardef->int_attribs);

	m_result << StorageQualifierToString(vardef->storage_type) << TypeToString(vardef->variable_type) << " ";

	for (size_t i = 0; i < vardef->variables.size(); i++)
	{
		KSLVariableDefinitionsNode::InnerNode &dn = vardef->variables[i];
		KSLVariable &v = ast->variables[dn.variable_id];
		m_result << v.new_name;

		if (v.type.IsArray())
		{
			m_result << "[";

			if (dn.size_expression == NULL)
			{
				if (vardef->storage_type != KSL_STORAGE_BUFFER)
				{
					if (dn.init_expressions.size() == 0)
					{
						assert(0);
						return false;
					}
					
					m_result << dn.init_expressions.size();
				}
			}
			else
			{
				VisitExpression(dn.size_expression);
			}
			
			m_result << "]";
		}

		if (dn.init_expressions.size() != 0)
		{
			m_result << " = ";

			if (v.type.IsArray())
			{
				m_result << TypeToStringWithOutPrecision(vardef->variable_type) << "[";
				m_result << dn.init_expressions.size();
				m_result << "](";

				for (size_t i = 0; i < dn.init_expressions.size(); i++)
				{
					VisitExpression(dn.init_expressions[i]);
					if (i + 1 < dn.init_expressions.size()) m_result << ", ";
				}

				m_result << ")";
			}
			else
			{
				assert(dn.init_expressions.size() == 1);
				VisitExpression(dn.init_expressions[0]);
			}
		}

		if (i + 1 < vardef->variables.size()) m_result << ", ";
	}

	m_result << ";";

	return true;
}


bool KSLGLGeneratorBase::VisitImageDefinition(KSLImageDefinitionNode* idn)
{
	SyncLine(idn);

	KSLVariable &v = ast->variables[idn->variable_id];

	CHECK_FORCE_HIGHP(v.type);

	idn->bool_attribs.erase(KSL_ATTRIB_QUALIFIER_READONLY);
	idn->bool_attribs.erase(KSL_ATTRIB_QUALIFIER_WRITEONLY);

	PrintDefinitionLayout(idn->bool_attribs, idn->int_attribs);
	m_result << " uniform ";
	
	switch (v.access)
	{
	case KSL_ACCESS_READ_ONLY: m_result << "readonly "; break;
	case KSL_ACCESS_WRITE_ONLY: m_result << "writeonly "; break;
	default: assert(0);
	}

	switch (v.type.precision)
	{
	case KSL_PRECISION_HIGH:   m_result << "highp "; break;
	case KSL_PRECISION_MEDIUM: m_result << "mediump "; break;
	case KSL_PRECISION_LOW:    m_result << "lowp "; break;
	default: assert(0);
	}

	m_result << "image2D " << v.new_name << ";";
	return true;
}


bool KSLGLGeneratorBase::VisitConstructorExpression(KSLConstructorExpressionNode *ce)
{
	SyncLine(ce);

	m_result << TypeToStringWithOutPrecision(ce->constructor_type) << "(";

	for (size_t i = 0; i < ce->initializers.size(); i++)
	{
		VisitExpression(ce->initializers[i]);
		if (i + 1 < ce->initializers.size()) m_result << ",";
	}

	m_result << ")";
	return true;
}


bool KSLGLGeneratorBase::VisitNumThreads(KSLNumThreadNode* ntn)
{
	SyncLine(ntn);
	m_result << "layout(local_size_x = " << ntn->x << ", local_size_y = " << ntn->y << ", local_size_z = " << ntn->z << ") in;";
	NewLine();
	return true;
}


bool KSLGLGeneratorBase::VisitFunctionCallExpression(KSLFunctionCallExpressionNode * fce)
{
	if ((fce->name == "workgroupMemoryBarrierAll") || (fce->name == "workgroupMemoryBarrierGlobal"))
	{
		m_result << "groupMemoryBarrier();"; NewLine();
		m_result << "barrier()";
		return true;
	}
	else if (fce->name == "workgroupMemoryBarrierShared")
	{
		m_result << "memoryBarrierShared();"; NewLine();
		m_result << "barrier()";
		return true;
	}

	KSLGenerator::VisitFunctionCallExpression(fce);
	return true;
}


bool KSLGLGeneratorBase::VisitLiteralExpression(KSLLiteralExpressionNode *le)
{
	CHECK_FORCE_HIGHP(le->type);

	switch (le->type.id)
	{
	case KSL_TYPE_FLOAT:
		PrintFloatLiteral(le, true);
		break;
	default:
		return KSLGenerator::VisitLiteralExpression(le);
	}

	return true;
}


std::string KSLGLGeneratorBase::TranslateAttribQualifer(uint32_t qualifier)
{
	switch (qualifier)
	{
		case KSL_ATTRIB_QUALIFIER_COLOR: return "location";
		case KSL_ATTRIB_QUALIFIER_GL_BINDING: return "binding";
		case KSL_ATTRIB_QUALIFIER_GL_STD140: return "std140";
		case KSL_ATTRIB_QUALIFIER_GL_STD430: return "std430";

		default:
			break;
	}

	return KSLGenerator::TranslateAttribQualifer(qualifier);
}


std::string KSLGLGeneratorBase::PrecisionToPrefix(KSLPrecision p)
{
	switch (p)
	{
	case KSL_PRECISION_HIGH:   return "highp ";
	case KSL_PRECISION_MEDIUM: return "mediump ";
	case KSL_PRECISION_LOW:    return "lowp ";
	case KSL_PRECISION_NONE:   return "";
	default: break;
	}

	assert(0);
	return "error_prec ";
}


std::string KSLGLGeneratorBase::TypeToString(KSLType type)
{
	CHECK_FORCE_HIGHP(type);
	return PrecisionToPrefix(type.precision) + TypeToStringWithOutPrecision(type);
}


std::string KSLGLGeneratorBase::TypeToStringWithOutPrecision(KSLType type)
{
	switch (type.id)
	{
	case KSL_TYPE_VOID:
		return "void";
		
	// float
	case KSL_TYPE_FLOAT: return "float";

	// vectors
	case KSL_TYPE_VEC4: return "vec4";
	case KSL_TYPE_VEC3: return "vec3";
	case KSL_TYPE_VEC2: return "vec2";

	// matrices
	case KSL_TYPE_MAT4: return "mat4";
	case KSL_TYPE_MAT3: return "mat3";
	case KSL_TYPE_MAT2: return "mat2";

	case KSL_TYPE_INT:   return "int";
	case KSL_TYPE_UINT:  return "uint";

		// Integer vectors
	case KSL_TYPE_INT4:  return "ivec4";
	case KSL_TYPE_INT3:  return "ivec3";
	case KSL_TYPE_INT2:  return "ivec2";

	case KSL_TYPE_UINT4: return "uvec4";
	case KSL_TYPE_UINT3: return "uvec3";
	case KSL_TYPE_UINT2: return "uvec2";

	case KSL_TYPE_BOOL:  return "bool";
	case KSL_TYPE_BOOL2: return "bool2";
	case KSL_TYPE_BOOL3: return "bool3";
	case KSL_TYPE_BOOL4: return "bool4";

	// samplers
	case KSL_TYPE_SAMPLER_2D: return "sampler2D";
	case KSL_TYPE_SAMPLER_2D_ARRAY: return "sampler2DArray";
	case KSL_TYPE_SAMPLER_CUBE: return "samplerCube";
	case KSL_TYPE_SAMPLER_CUBE_ARRAY: return "samplerCubeArray";

	// shadow samplers
	case KSL_TYPE_SAMPLER_2D_SHADOW: return "sampler2DShadow";
	case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW: return "sampler2DArrayShadow";
	case KSL_TYPE_SAMPLER_CUBE_SHADOW: return "samplerCubeShadow";

	// subpass input
	case KSL_TYPE_SUBPASS_INPUT: return "sampler2D";

	default:
		return KSLGenerator::TypeToString(type);
	}

	return "";
}


