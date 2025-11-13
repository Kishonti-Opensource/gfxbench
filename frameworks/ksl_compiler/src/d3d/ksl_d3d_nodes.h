/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_D3D_NODES__
#define __KSL_D3D_NODES__

#include "ksl_ast_tree.h"


enum KSLD3DNodeTypes
{
	KSL_NODE_D3D_UNIFORM_INTERFACE = KSL_NUM_NODES,
	KSL_NODE_D3D_INPUT_INTERFACE,
	KSL_NODE_D3D_OUTPUT_INTERFACE,
	KSL_NODE_D3D_MAIN_FUNCTION,
	KSL_NODE_D3D_OUTPUT_VARIABLE_DEFINITION
};


struct KSLD3DUniformInterfaceNode : public KSLStructDefinitionNode
{
	KSLD3DUniformInterfaceNode()
	{
		node_type = KSL_NODE_D3D_UNIFORM_INTERFACE;
		binding_point = 0;
	}

	uint32_t binding_point;
};


struct KSLD3DInputOutputIntefaceNode : public KSLStructDefinitionNode
{
	KSLD3DInputOutputIntefaceNode(uint32_t type)
	{
		node_type = type;
	}

	std::map<std::string,std::string> qualifiers;
};


#endif  // __KSL_D3D_NODES__

