/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_MTL_NODES__
#define __KSL_MTL_NODES__

#include "ksl_ast_tree.h"

enum KSLMetalNodeTypes
{
	KSL_NODE_METAL_UNIFORM_INTERFACE = KSL_NUM_NODES,
	KSL_NODE_METAL_INPUT_INTERFACE,
	KSL_NODE_METAL_OUTPUT_INTERFACE,
	KSL_NODE_METAL_MAIN_FUNCTION
};


struct KSLMetalUniformInterfaceNode : public KSLStructDefinitionNode
{
	KSLMetalUniformInterfaceNode()
	{
		node_type = KSL_NODE_METAL_UNIFORM_INTERFACE;
		binding_point = 0;
	}

	uint32_t binding_point;
};


struct KSLMetalInputOutputIntefaceNode : public KSLStructDefinitionNode
{
	KSLMetalInputOutputIntefaceNode(uint32_t type)
	{
		node_type = type;
	}

	std::vector<std::string> qualifiers;
};


struct KSLMetalMainFunctionNode : public KSLFunctionNode
{
	KSLMetalMainFunctionNode()
	{
		node_type = KSL_NODE_METAL_MAIN_FUNCTION;
	}

	~KSLMetalMainFunctionNode()
	{
		for (size_t i = 0; i < samplers.size(); i++) delete samplers[i];
		for (size_t i = 0; i < buffers.size(); i++)  delete buffers[i];
		for (size_t i = 0; i < images.size(); i++)   delete images[i];
		for (size_t i = 0; i < subpass_inputs.size(); i++) delete subpass_inputs[i];
	}

	std::vector<KSLVariableDefinitionsNode*> samplers;
	std::vector<KSLVariableDefinitionsNode*> buffers;
	std::vector<KSLVariableDefinitionsNode*> subpass_inputs;
	std::vector<KSLImageDefinitionNode*> images;
};


#endif // __KSL_MTL_NODES__

