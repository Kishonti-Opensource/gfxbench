/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_vk_generator.h"

#include<assert.h>


const std::string KSL_VK_PUSH_CONTANT_UBO_NAME = "ksl_push_constant_";


KSLVKGenerator::KSLVKGenerator(bool vk_use_subpass)
	: m_vk_translator(NULL)
	, m_vk_use_subpass(vk_use_subpass)
{

}


KSLVKGenerator::~KSLVKGenerator()
{

}


bool KSLVKGenerator::Generate()
{
	PrintHeader();

	return KSLGLGeneratorBase::Generate();
}


void KSLVKGenerator::SetTranslator(KSLTranslator* translator)
{
	m_vk_translator = dynamic_cast<KSLVKTranslator*>(translator);
}


bool KSLVKGenerator::VisitVulkanInterfaceNode(KSLVulkanInterfaceNode* vin)
{
	m_result << "layout(";

	switch (vin->m_group_type)
	{
	case KSL_VK_UNIFORM_INTERFACE_PER_DRAW:
	case KSL_VK_UNIFORM_INTERFACE_RENDERER_CHANGE:
	case KSL_VK_UNIFORM_INTERFACE_MANUAL:
		m_result << "std140, binding = " << vin->m_binding_point << ", ";
		m_result << "set = " << vin->m_group_id;
		m_result << ") uniform uniformObject" << ast->shader_type << "_" << vin->m_group_id;
		break;

	case KSL_VK_UNIFORM_INTERFACE_PUSH_CONSTANT:
		m_result << "push_constant) uniform uniformObject";
		break;

	default:
		assert(0);
		break;
	}

	NewLine();
	m_result << "{";
	m_indent++;
	NewLine();

	m_do_syncline = false;
	for (size_t i = 0; i < vin->members.size(); i++)
	{
		VisitVariableDefinitions(vin->members[i]);
		if (i + 1 != vin->members.size()) NewLine();
	}
	m_do_syncline = true;

	m_indent--;
	NewLine();

	switch (vin->m_group_type)
	{
	case KSL_VK_UNIFORM_INTERFACE_PER_DRAW:
	case KSL_VK_UNIFORM_INTERFACE_RENDERER_CHANGE:
	case KSL_VK_UNIFORM_INTERFACE_MANUAL:
		m_result << "};";
		break;

	case KSL_VK_UNIFORM_INTERFACE_PUSH_CONSTANT:
		m_result << "} " << KSL_VK_PUSH_CONTANT_UBO_NAME << ";";
		break;

	default:
		assert(0);
		break;
	}
	
	NewLine();
	return true;
}


bool KSLVKGenerator::VisitVariableExpression(KSLVariableExpressionNode *ve)
{
	uint32_t v_id = ve->variable_id;

	std::map<uint32_t, KSLVulkanUniformInterfaceType>::const_iterator it = m_vk_translator->m_variable_group_table.find(v_id);
	if (it != m_vk_translator->m_variable_group_table.end())
	{
		switch (it->second)
		{
		case KSL_VK_UNIFORM_INTERFACE_PER_DRAW:
		case KSL_VK_UNIFORM_INTERFACE_RENDERER_CHANGE:
		case KSL_VK_UNIFORM_INTERFACE_MANUAL:
			break;

		case KSL_VK_UNIFORM_INTERFACE_PUSH_CONSTANT:
			m_result << KSL_VK_PUSH_CONTANT_UBO_NAME<<".";
			break;

		default:
			assert(0);
			break;
		}
	}

	return KSLGenerator::VisitVariableExpression(ve);
}


bool KSLVKGenerator::VisitGlobalSpaceNode(KSLASTNode* n)
{
	switch (n->node_type)
	{
	case KSL_NODE_VULKAN_INTERFACE_BLOCK:
		if (!VisitVulkanInterfaceNode(dynamic_cast<KSLVulkanInterfaceNode*>(n))) return false;
		break;

	default:
		return KSLGenerator::VisitGlobalSpaceNode(n);
		break;
	}

	return true;
}


bool KSLVKGenerator::VisitFunctionCallExpression(KSLFunctionCallExpressionNode* fce)
{
	if (!m_vk_use_subpass && (fce->name == "subpassLoad"))
	{
		m_result << "texelFetch(";
		VisitExpression(fce->arguments[0]);
		m_result << ", ivec2(gl_FragCoord.xy), 0)";
		return true;
	}

	return KSLGLGeneratorBase::VisitFunctionCallExpression(fce);
}


std::string KSLVKGenerator::TypeToString(KSLType type)
{
	switch (type.id)
	{
	// subpass input
	case KSL_TYPE_SUBPASS_INPUT: return PrecisionToPrefix(type.precision) + ((m_vk_use_subpass)?"subpassInput":"sampler2D");

	default:
		return KSLGLGeneratorBase::TypeToString(type);
	}

	return "";
}


std::string KSLVKGenerator::TranslateAttribQualifer(uint32_t qualifier)
{
	switch (qualifier)
	{
	case KSL_ATTRIB_QUALIFIER_VK_LOCATION: return "location";
	case KSL_ATTRIB_QUALIFIER_VK_INPUT_ATTACHMENT_ID: return "input_attachment_index";
	case KSL_ATTRIB_QUALIFIER_VK_GROUP_ID: return "set";
	case KSL_ATTRIB_QUALIFIER_VK_OFFSET: return "offset";

	default:
		break;
	}

	return KSLGLGeneratorBase::TranslateAttribQualifer(qualifier);
}


void KSLVKGenerator::PrintHeader()
{
	if (m_vk_translator->ast->shader_type == NGL_VERTEX_SHADER)
	{
		m_result << "out gl_PerVertex"; NewLine();
		m_result << "{"; NewLine();
		m_result << "  vec4 gl_Position;"; NewLine();
		m_result << "};"; NewLine();
		NewLine();
	}

	if (ast->shader_type == NGL_FRAGMENT_SHADER)
	{
		if (!ast->has_discard)
		{
			m_result << "layout(early_fragment_tests) in;"; NewLine();
			NewLine();
		}

		//m_result << "layout(depth_unchanged) out float gl_FragDepth;"; NewLine();
		NewLine();
	}
}

