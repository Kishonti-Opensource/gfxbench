/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_VK_NODES__
#define __KSL_VK_NODES__

#include "ksl_ast_tree.h"
#include "../gl/ksl_gl_translator.h"


enum KSLVulkanAttribQualifiers
{
	KSL_ATTRIB_QUALIFIER_VK_LOCATION = KSL_NUM_GL_ATTRIB_QUALIFIER,
	KSL_ATTRIB_QUALIFIER_VK_INPUT_ATTACHMENT_ID,
	KSL_ATTRIB_QUALIFIER_VK_GROUP_ID,
	KSL_ATTRIB_QUALIFIER_VK_OFFSET
};


enum KSLVulkanNodeTypes
{
	KSL_NODE_VULKAN_INTERFACE_BLOCK = KSL_NUM_NODES
};


enum KSLVulkanUniformInterfaceType
{
	KSL_VK_UNIFORM_INTERFACE_PER_DRAW = 0,
	KSL_VK_UNIFORM_INTERFACE_RENDERER_CHANGE,
	KSL_VK_UNIFORM_INTERFACE_MANUAL,
	KSL_VK_UNIFORM_INTERFACE_PUSH_CONSTANT,
	KSL_VK_NUM_UNIFORM_INTERFACE,
	KSL_VK_UNIFORM_INTERFACE_INVALID = KSL_VK_NUM_UNIFORM_INTERFACE
};


struct KSLVulkanInterfaceNode : public KSLASTNode
{
	KSLVulkanInterfaceNode()
	{
		node_type = KSL_NODE_VULKAN_INTERFACE_BLOCK;
		m_binding_point = 0;
		m_group_id = KSL_INT32_MAX;
		m_group_type = KSL_VK_UNIFORM_INTERFACE_INVALID;
	}

	~KSLVulkanInterfaceNode()
	{
		for (size_t i = 0; i < members.size(); i++)
		{
			delete members[i];
		}
	}

	std::vector<KSLVariableDefinitionsNode*> members;
	uint32_t m_binding_point;
	uint32_t m_group_id;
	uint32_t m_group_type;
};


#endif // __KSL_VK_NODES__

