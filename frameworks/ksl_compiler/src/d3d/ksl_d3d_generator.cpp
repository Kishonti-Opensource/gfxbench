/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_d3d_generator.h"

#include <assert.h>

KSLD3DGenerator::KSLD3DGenerator()
	: KSLGenerator()
	, m_d3d_translator(NULL)
	, m_visiting_main(false)
	, m_visiting_global_vdn(false)
	, m_next_srv_binding(0)
	, m_next_uav_binding(0)
{
}


KSLD3DGenerator::~KSLD3DGenerator()
{
}


bool KSLD3DGenerator::VisitBinaryExpression(KSLBinaryExpressionNode *be)
{
	SyncLine(be);

	if (be->operation == KSL_BINOP_MUL)
	{
		assert(be->e1->type.id != KSL_TYPE_INVALID);
		assert(be->e2->type.id != KSL_TYPE_INVALID);

		bool generate_mul = false;
		generate_mul |= be->e1->type.IsMatrix() && be->e2->type.IsMatrix();
		generate_mul |= be->e1->type.IsMatrix() && be->e2->type.IsVector();
		generate_mul |= be->e1->type.IsVector() && be->e2->type.IsMatrix();

		if (generate_mul)
		{
			m_result << "mul(";
			VisitExpression(be->e1);
			m_result << ",";
			VisitExpression(be->e2);
			m_result << ")";
			return true;
		}
	}

	return KSLGenerator::VisitBinaryExpression(be);
}


bool KSLD3DGenerator::VisitD3DUniformInterfaceNode(KSLD3DUniformInterfaceNode* d3duin)
{
	m_result << "cbuffer cb"<< d3duin->binding_point << " : register(b" << d3duin->binding_point << ")";
	NewLine();
	m_result << "{";
	m_indent++;
	NewLine();

	bool temp = m_do_syncline;
	m_do_syncline = false;
	for (size_t i = 0; i < d3duin->members.size(); i++)
	{
		VisitVariableDefinitions(d3duin->members[i]);
		if (i + 1 != d3duin->members.size()) NewLine();
	}
	m_do_syncline = temp;

	m_indent--;
	NewLine();
	m_result << "};";
	NewLine();

	return true;
}


bool KSLD3DGenerator::VisitD3DInputOutputInterfaceNode(KSLD3DInputOutputIntefaceNode* d3dioin)
{
	SyncLine(d3dioin);

	m_result << "struct " << d3dioin->name; NewLine();
	m_result << "{";
	m_indent++;

	for (size_t i = 0; i < d3dioin->members.size(); i++)
	{
		NewLine();

		KSLVariableDefinitionsNode* vardef = d3dioin->members[i];
		assert(vardef->variables.size() == 1);

		m_result << TypeToString(vardef->variable_type) << " ";

		KSLVariableDefinitionsNode::InnerNode &dn = vardef->variables[0];
		KSLVariable &v = ast->variables[dn.variable_id];
		m_result << v.new_name;

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

		assert(dn.init_expressions.size() == 0);

		std::string &q = d3dioin->qualifiers.at(v.orig_name);
		assert(q != "");
		m_result << " : " << q << ";";
	}

	m_indent--;
	NewLine();
	m_result << "};"; NewLine();

	return true;
}


bool KSLD3DGenerator::VisitD3DMainFunctionNode(KSLFunctionNode* d3dmfn)
{
	if (m_d3d_translator->ast->shader_type == NGL_COMPUTE_SHADER)
	{
		KSLNumThreadNode* ntn = m_d3d_translator->m_num_thread_node;
		m_result << "[numthreads(" << ntn->x << ", " << ntn->y << ", " << ntn->z << ")]";
		NewLine();
	}


	// function name
	{
		m_result << (m_d3d_translator->m_has_output_interface?KSL_D3D_OUTPUT_LAYOUT_TYPE_NAME:"void");

		switch (ast->shader_type)
		{
		case NGL_VERTEX_SHADER:   m_result << " vertex_main("; break;
		case NGL_FRAGMENT_SHADER: m_result << " fragment_main("; break;
		case NGL_COMPUTE_SHADER:  m_result << " compute_main("; break;
		default:
			assert(0);
			break;
		}
	}

	m_indent++;
	bool need_comma = false;

	// input
	if (m_d3d_translator->m_has_input_interface)
	{
		if (need_comma) m_result << ",";
		NewLine();
		m_result << KSL_D3D_INPUT_LAYOUT_TYPE_NAME << " " << KSL_D3D_INPUT_NAME;
		need_comma = true;
	}

	if (ast->shader_type == NGL_FRAGMENT_SHADER)
	{
		if (need_comma) m_result << ","; NewLine();
		m_result << "float4 gl_FragCoord : SV_POSITION,"; NewLine();
		m_result << "bool gl_FrontFacing : SV_IsFrontFace";
		need_comma = true;
	}

	else if (ast->shader_type == NGL_VERTEX_SHADER)
	{
		if (need_comma) m_result << ","; NewLine();
		m_result << "uint gl_VertexID : SV_VertexID";
		need_comma = true;
	}

	else if (ast->shader_type == NGL_COMPUTE_SHADER)
	{
		if (need_comma) m_result << ","; NewLine();
		m_result << "uint3 gl_WorkGroupID : SV_GroupID,"; NewLine();
		m_result << "uint3 gl_LocalInvocationID : SV_GroupThreadID,"; NewLine();
		m_result << "uint3 gl_GlobalInvocationID : SV_DispatchThreadID,"; NewLine();
		m_result << "uint  gl_LocalInvocationIndex : SV_GroupIndex";
		need_comma = true;
	}

	m_indent--; NewLine();
	m_result << ")"; NewLine();

	m_visiting_main = true;
	VisitBlockStatement(d3dmfn->body);
	m_visiting_main = false;

	return true;
}


bool KSLD3DGenerator::VisitVariableExpression(KSLVariableExpressionNode *ve)
{
	KSLVariable &v = ast->variables[ve->variable_id];

	switch (v.storage_type)
	{
	case KSL_STORAGE_IN: m_result << KSL_D3D_INPUT_NAME << "."; break;
	case KSL_STORAGE_OUT: m_result << KSL_D3D_OUTPUT_NAME << "."; break;

	case KSL_STORAGE_UNIFORM:
	case KSL_STORAGE_BUFFER:
	case KSL_STORAGE_SHARED:
	case KSL_STORAGE_DEFAULT:
	case KSL_STORAGE_CONST:
		break;
	default:
		assert(0);
		return false;
	}

	bool s = KSLGenerator::VisitVariableExpression(ve);

	if ((v.storage_type == KSL_STORAGE_BUFFER) && !v.type.IsArray())
	{
		m_result << "[0]";
	}

	return s;
}


bool KSLD3DGenerator::VisitStatementOrVariableDefinitions(KSLASTNode *n)
{
	if (n->node_type == KSL_NODE_D3D_OUTPUT_VARIABLE_DEFINITION)
	{
		m_result << KSL_D3D_OUTPUT_LAYOUT_TYPE_NAME << " " << KSL_D3D_OUTPUT_NAME << " = (" << KSL_D3D_OUTPUT_LAYOUT_TYPE_NAME << ")0;";
		return true;
	}
	return KSLGenerator::VisitStatementOrVariableDefinitions(n);
}


bool KSLD3DGenerator::VisitReturnStatement(KSLReturnStatementNode* rs)
{
	SyncLine(rs);

	if (m_visiting_main && m_d3d_translator->m_has_output_interface)
	{
		assert(rs->expression == NULL);
		m_result << "return " << KSL_D3D_OUTPUT_NAME << ";";
		return true;
	}

	return KSLGenerator::VisitReturnStatement(rs);
}


bool KSLD3DGenerator::VisitVariableDefinitions(KSLVariableDefinitionsNode *vardef)
{
	SyncLine(vardef);

	KSLVariable &v = ast->variables[vardef->variables[0].variable_id];

	CHECK_FORCE_HIGHP(v.type);

	if (vardef->storage_type == KSL_STORAGE_BUFFER)
	{
		assert(vardef->variables.size() == 1);

		bool is_ubo = (vardef->bool_attribs.find(KSL_ATTRIB_QUALIFIER_SSBO) == vardef->bool_attribs.end());
		bool is_readonly = (vardef->bool_attribs.find(KSL_ATTRIB_QUALIFIER_READONLY) != vardef->bool_attribs.end());
		bool is_readwrite_ssbo = !is_ubo && !is_readonly;

		m_result << ((is_readwrite_ssbo)?"RWStructuredBuffer":"StructuredBuffer") ;
		KSLType &type = v.type.IsArray() ? v.type.GetBaseType() : v.type;
		m_result << "<" << TypeToString(type) << "> " << v.orig_name << " : register(";

		if (is_readwrite_ssbo)
		{
			m_result << "u" << m_next_uav_binding;
			m_next_uav_binding++;
		}
		else
		{
			m_result << "t" << m_next_srv_binding;
			m_next_srv_binding++;
		}

		m_result << ");";
		NewLine();
		return true;
	}

	if (v.type.IsSampler() || v.type.IsSubpassInput())
	{
		assert(vardef->variables.size() == 1);

		m_result << TypeToString(v.type) << " " << v.orig_name << " : register(t" << m_next_srv_binding << ");";
		NewLine();
		m_result << GetSamplerStateType(v.type) <<" " << v.orig_name << KSL_D3D_SAMPLER_SUFFIX << " : register(s" << m_next_srv_binding << ");";
		NewLine();
		m_next_srv_binding++;
		return true;
	}

	return KSLGenerator::VisitVariableDefinitions(vardef);
}


bool KSLD3DGenerator::VisitFunction(KSLFunctionNode* fn)
{
	SyncLine(fn);

	KSLFunction &function = ast->functions[fn->function_id];

	m_result << TypeToString(function.return_type) << " " << function.function_name << "(";

	for (size_t i = 0; i < fn->attribs.size(); i++)
	{
		KSLFunctionNode::AttribNode &an = fn->attribs[i];

		KSLVariable &v = ast->variables[an.variable_id];

		switch (function.attrib_access[i])
		{
		case KSL_ATTRIB_ACCESS_IN: break;
		case KSL_ATTRIB_ACCESS_OUT: m_result << "out "; break;
		case KSL_ATTRIB_ACCESS_INOUT: m_result << "inout "; break;

		default: assert(0); break;
		}

		if (!v.type.IsArray())
		{
			m_result << TypeToString(v.type) << " ";
			m_result << v.new_name;
		}
		else
		{
			m_result << TypeToString(v.type.GetBaseType()) << " ";
			m_result << v.new_name;

			m_result << "[";
			VisitExpression(an.size_expression);
			m_result << "]";
		}


		if (v.type.IsSampler())
		{
			m_result << ", " << GetSamplerStateType(v.type) << " " << v.new_name << KSL_D3D_SAMPLER_SUFFIX;
		}

		if (i + 1 < fn->attribs.size()) m_result << ", ";
	}

	// print global arguments
	{
		KSLGlobalUsageInfo &gui = m_d3d_translator->m_global_usage[function.function_name];
		bool need_comma = fn->attribs.size() > 0;

		if (gui.use_in_attribs || gui.use_out_attribs || gui.use_in_attribs) NewLine();

		if (gui.use_in_attribs)
		{
			if (need_comma) m_result << ", ";
			m_result << "inout " << KSL_D3D_INPUT_LAYOUT_TYPE_NAME << " " << KSL_D3D_INPUT_NAME;
			need_comma = true;
		}


		if (gui.use_out_attribs)
		{
			if (need_comma) m_result << ", ";
			m_result << "inout " << KSL_D3D_OUTPUT_LAYOUT_TYPE_NAME << " " << KSL_D3D_OUTPUT_NAME;
			need_comma = true;
		}


		// inbuilts
		{
			if (gui.used_inbuilts.size() > 0) NewLine();
			std::set<uint32_t>::iterator inbuilt_it = gui.used_inbuilts.begin();
			for (; inbuilt_it != gui.used_inbuilts.end(); inbuilt_it++)
			{
				if (need_comma) m_result << ", ";

				KSLVariable &v = ast->variables[*inbuilt_it];

				m_result << TypeToString(v.type) << " " << v.new_name;

				need_comma = true;
			}
		}
	}

	m_result << ")";
	NewLine();

	VisitBlockStatement(fn->body);

	return true;
}


bool KSLD3DGenerator::PrintImageStore(KSLFunctionCallExpressionNode* fce)
{
	if (fce->name == "imageStore")
	{
		KSLExpressionNode* texture_exp = fce->arguments[0];
		KSLExpressionNode* texcoord_exp = fce->arguments[1];
		KSLExpressionNode* value_exp = fce->arguments[2];

		VisitExpression(texture_exp);
		m_result << "[";
		VisitExpression(texcoord_exp);
		m_result << "] = ";
		VisitExpression(value_exp);
		return true;
	}

	return false;
}


bool KSLD3DGenerator::PrintSubpassLoad(KSLFunctionCallExpressionNode* fce)
{
	if (fce->name == "subpassLoad")
	{
		VisitExpression(fce->arguments[0]);
		m_result << ".Load(int3(gl_FragCoord.xy,0))";
		return true;
	}

	return false;
}


bool KSLD3DGenerator::PrintBarrier(KSLFunctionCallExpressionNode* fce)
{
	if (fce->name == "workgroupMemoryBarrierAll")
	{
		m_result << "AllMemoryBarrierWithGroupSync()";
		return true;
	}
	else if (fce->name == "workgroupMemoryBarrierGlobal")
	{
		m_result << "DeviceMemoryBarrierWithGroupSync()";
		return true;
	}
	else if (fce->name == "workgroupMemoryBarrierShared")
	{
		m_result << "GroupMemoryBarrierWithGroupSync()";
		return true;
	}

	return false;
}


bool KSLD3DGenerator::VisitFunctionCallExpression(KSLFunctionCallExpressionNode* fce)
{
	SyncLine(fce);

	if (PrintTextureAccess(fce)) return true;
	if (PrintSubpassLoad(fce)) return true;
	if (PrintBarrier(fce)) return true;
	if (PrintImageStore(fce)) return true;

	if (fce->name == "fract")
	{
		m_result << "frac";
	}
	else if (fce->name == "mix")
	{
		m_result << "lerp";
	}
	else
	{
		m_result << fce->name;
	}

	m_result << "(";
	for (size_t i = 0; i < fce->arguments.size(); i++)
	{
		KSLExpressionNode* ae = fce->arguments[i];
		VisitExpression(ae);

		if ((ae->node_type == KSL_NODE_VARIABLE_EXPRESSION) && ae->type.IsSampler())
		{
			m_result << ", ";
			VisitExpression(ae);
			m_result << KSL_D3D_SAMPLER_SUFFIX;
		}

		if (i + 1 < fce->arguments.size()) m_result << ",";
	}

	// print global arguments
	{
		KSLGlobalUsageInfo &gui = m_d3d_translator->m_global_usage[fce->name];
		bool need_comma = fce->arguments.size() > 0;

		if (gui.use_in_attribs)
		{
			if (need_comma) m_result << ", ";
			m_result << KSL_D3D_INPUT_NAME;
			need_comma = true;
		}

		if (gui.use_out_attribs)
		{
			if (need_comma) m_result << ", ";
			m_result << KSL_D3D_OUTPUT_NAME;
			need_comma = true;
		}

		// inbuilts
		{
			std::set<uint32_t>::iterator inbuilt_it = gui.used_inbuilts.begin();
			for (; inbuilt_it != gui.used_inbuilts.end(); inbuilt_it++)
			{
				if (need_comma) m_result << ", ";
				KSLVariable &v = ast->variables[*inbuilt_it];
				m_result << v.new_name;
				need_comma = true;
			}
		}
	}


	m_result << ")";
	
	return true;
}


bool KSLD3DGenerator::VisitSelectorExpression(KSLSelectorExpressionNode *se)
{
	SyncLine(se);

	if (se->expression->type.IsMatrix() && (se->node_type == KSL_NODE_ARRAY_ACCESS_EXPRESSION))
	{
		m_result << "transpose(";
		VisitExpression(se->expression);
		m_result << ")[";
		VisitExpression(se->id_expression);
		m_result << "]";
		return true;
	}

	return KSLGenerator::VisitSelectorExpression(se);
}


bool KSLD3DGenerator::VisitConstructorExpression(KSLConstructorExpressionNode *ce)
{
	SyncLine(ce);

	if (ce->initializers.size() == 1)
	{
		m_result << "((" << TypeToString(ce->constructor_type) << ")(";
		VisitExpression(ce->initializers[0]);
		m_result << "))";
		return true;
	}

	if (ce->type.IsMatrix())
	{
		m_result << "transpose(";
		bool s = KSLGenerator::VisitConstructorExpression(ce);
		m_result << ")";
		return s;
	}

	return KSLGenerator::VisitConstructorExpression(ce);
}


bool KSLD3DGenerator::VisitLiteralExpression(KSLLiteralExpressionNode *le)
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


bool  KSLD3DGenerator::VisitForStatement(KSLForStatementNode *fs)
{
	SyncLine(fs);

	if (fs->is_loop)
	{
		m_result << "[ loop ]";
		NewLine();
	}

	return KSLGenerator::VisitForStatement(fs);
}


bool KSLD3DGenerator::VisitImageDefinition(KSLImageDefinitionNode* idn)
{
	SyncLine(idn);

	KSLVariable &v = ast->variables[idn->variable_id];

	CHECK_FORCE_HIGHP(v.type);

	switch (v.access)
	{
	case KSL_ACCESS_READ_ONLY: m_result << "Texture2D "; break;
	case KSL_ACCESS_WRITE_ONLY: m_result << "RWTexture2D<unorm float4> "; break; // we only bind unorm textures this way in Aztec
	default: assert(0);
	}

	m_result << v.new_name << " : register(u" << m_next_uav_binding << ");";
	m_next_uav_binding++;

	return true;
}


bool KSLD3DGenerator::VisitGlobalSpaceNode(KSLASTNode* n)
{
	switch (n->node_type)
	{
	case KSL_NODE_D3D_UNIFORM_INTERFACE:
		return VisitD3DUniformInterfaceNode(dynamic_cast<KSLD3DUniformInterfaceNode*>(n));
		break;

	case KSL_NODE_D3D_INPUT_INTERFACE:
	case KSL_NODE_D3D_OUTPUT_INTERFACE:
		return VisitD3DInputOutputInterfaceNode(dynamic_cast<KSLD3DInputOutputIntefaceNode*>(n));

	case KSL_NODE_D3D_MAIN_FUNCTION:
		return VisitD3DMainFunctionNode(dynamic_cast<KSLFunctionNode*>(n));

	case KSL_NODE_VARIABLE_DEFINITIONS:
	{
		m_visiting_global_vdn = true;
		bool s = VisitVariableDefinitions(dynamic_cast<KSLVariableDefinitionsNode*>(n));
		m_visiting_global_vdn = false;
		return s;
		break;
	}
		
	default:
		return KSLGenerator::VisitGlobalSpaceNode(n);
		break;
	}

	return true;
}


bool KSLD3DGenerator::Generate()
{
	PrintSamplingMethods();

	return KSLGenerator::Generate();
}


std::string KSLD3DGenerator::TypeToString(KSLType type)
{
	CHECK_FORCE_HIGHP(type);

	std::string fprefix, iprefix, uiprefix;

	switch (type.precision)
	{
	case KSL_PRECISION_HIGH:
		fprefix = "float";
		iprefix = "int";
		uiprefix = "uint";
		break;
	case KSL_PRECISION_MEDIUM:
		fprefix = "min16float";
		iprefix = "min16int";
		uiprefix = "min16uint";
		break;
	case KSL_PRECISION_LOW:
		fprefix = "min10float";
		iprefix = "min12int";
		uiprefix = "min16uint";
		break;
	case KSL_PRECISION_NONE:
		fprefix = "float_noprec";
		iprefix = "int_noprec";
		uiprefix = "uint_noprec";
		break;
	default:
		assert(0);
		break;
	}

	switch (type.id)
	{
		// float
	case KSL_TYPE_FLOAT: return fprefix;

		// vectors
	case KSL_TYPE_VEC4:  return fprefix + "4";
	case KSL_TYPE_VEC3:  return fprefix + "3";
	case KSL_TYPE_VEC2:  return fprefix + "2";

		// matrices
	case KSL_TYPE_MAT4:  return fprefix + "4x4";
	case KSL_TYPE_MAT3:  return fprefix + "3x3";
	case KSL_TYPE_MAT2:  return fprefix + "2x2";

	case KSL_TYPE_INT:   return iprefix;
	case KSL_TYPE_UINT:  return uiprefix;

		// Integer vectors
	case KSL_TYPE_INT4:  return iprefix + "4";
	case KSL_TYPE_INT3:  return iprefix + "3";
	case KSL_TYPE_INT2:  return iprefix + "2";

	case KSL_TYPE_UINT4: return uiprefix + "4";
	case KSL_TYPE_UINT3: return uiprefix + "3";
	case KSL_TYPE_UINT2: return uiprefix + "2";

		// samplers
	case KSL_TYPE_SAMPLER_2D:
	case KSL_TYPE_SAMPLER_2D_SHADOW:
	case KSL_TYPE_SUBPASS_INPUT:
		return "Texture2D";
	
	case KSL_TYPE_SAMPLER_2D_ARRAY:
	case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW:
		return "Texture2DArray";
	
	case KSL_TYPE_SAMPLER_CUBE:
	case KSL_TYPE_SAMPLER_CUBE_SHADOW:
		return "TextureCube";

	case KSL_TYPE_SAMPLER_CUBE_ARRAY:
		return "TextureCubeArray";

	default:
		break;
	}

	return KSLGenerator::TypeToString(type);
}


std::string KSLD3DGenerator::StorageQualifierToString(KSLStorageQualifier qualifier)
{
	switch (qualifier)
	{
	case KSL_STORAGE_SHARED:  return "groupshared ";
	case KSL_STORAGE_CONST: return m_visiting_global_vdn ? "static const " : "const ";
	case KSL_STORAGE_IN:
	case KSL_STORAGE_OUT:
	case KSL_STORAGE_UNIFORM:
	case KSL_STORAGE_BUFFER:
	case KSL_STORAGE_DEFAULT: return "";
	default: assert(0); return "error_qualifer";
	}
}


void KSLD3DGenerator::SetTranslator(KSLTranslator* translator)
{
	m_d3d_translator = dynamic_cast<KSLD3DTranslator*>(translator);
}


std::string KSLD3DGenerator::GetSamplerStateType(const KSLType &type)
{
	switch (type.id)
	{
	case KSL_TYPE_SAMPLER_2D:
	case KSL_TYPE_SUBPASS_INPUT:
	case KSL_TYPE_SAMPLER_2D_ARRAY:
	case KSL_TYPE_SAMPLER_CUBE_ARRAY:
	case KSL_TYPE_SAMPLER_CUBE:
		return "SamplerState";

	case KSL_TYPE_SAMPLER_2D_SHADOW:
	case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW:
	case KSL_TYPE_SAMPLER_CUBE_SHADOW:
		return "SamplerComparisonState";

	default:
		assert(0);
		break;
	}

	return "error_SamplerState";
}

