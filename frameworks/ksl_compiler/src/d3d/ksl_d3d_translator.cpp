/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_d3d_translator.h"

#include "ksl_d3d_nodes.h"

#include <sstream>
#include <algorithm>
#include <assert.h>


KSLD3DTranslator::KSLD3DTranslator()
	: m_num_thread_node(NULL)
	, m_has_input_interface(false)
	, m_has_output_interface(false)
{
}


KSLD3DTranslator::~KSLD3DTranslator()
{
	delete m_num_thread_node;
}


bool KSLD3DTranslator::Translate()
{
	if (ast == NULL) return false;

	bool s = true;

	if (s) s &= CreateGlobalUsageInfo();

	if (s) s &= CollectAttributes();

	if (s) s &= CreateUniformInterfaceBlock();

	if (s) s &= CreateMainFunction();

	if (s) s &= CollectNumThread();

	return s;
}


static uint32_t NGLGroupToUniformIntefaceBinding(NGL_shader_uniform_group ngl_group)
{
	switch (ngl_group)
	{
	case NGL_GROUP_PER_DRAW: return 0;
	case NGL_GROUP_PER_RENDERER_CHANGE: return 1;
	case NGL_GROUP_MANUAL: return 2;
	default:
		break;
	}

	return KSL_INT32_MAX;
}


bool KSLD3DTranslator::CreateUniformInterfaceBlock()
{
	if (m_uniform_info == NULL)
	{
		ErrorUniformInfoNotSet();
		return false;
	}

	std::map<std::string, uint32_t> m_group_mapping;
	for (size_t i = 0; i < m_uniform_info->size(); i++)
	{
		uint32_t t = NGLGroupToUniformIntefaceBinding(m_uniform_info->at(i).m_group);
		m_group_mapping[m_uniform_info->at(i).m_name] = t;
	}

	const uint32_t NUM_D3D_INTERFACE_NODES = 3;
	std::vector<KSLASTNode*> new_nodes;

	KSLD3DUniformInterfaceNode* d3di_nodes[NUM_D3D_INTERFACE_NODES];
	for (size_t i = 0; i < NUM_D3D_INTERFACE_NODES; i++)
	{
		d3di_nodes[i] = new KSLD3DUniformInterfaceNode;
		d3di_nodes[i]->binding_point = (uint32_t)i;
	}

	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vardef = (KSLVariableDefinitionsNode*)n;

			KSLType bt = vardef->variable_type;

			bool to_uniform_interface = true;
			to_uniform_interface &= (vardef->storage_type == KSL_STORAGE_UNIFORM);
			to_uniform_interface &= bt.IsNumeric() || bt.IsBool();

			assert(vardef->variables.size() == 1);
			KSLVariable &v = ast->variables[vardef->variables[0].variable_id];

			if (to_uniform_interface)
			{
				vardef->start_line = 0;
				vardef->storage_type = KSL_STORAGE_DEFAULT;
				assert(vardef->variables.size() == 1);

				if (m_group_mapping.find(v.orig_name) != m_group_mapping.end())
				{
					uint32_t g_id = m_group_mapping.at(v.orig_name);
					d3di_nodes[g_id]->members.push_back(vardef);
				}
				else
				{
					ErrorUniformInfoNotSetFor(v.orig_name);
				}
			}
			else
			{
				new_nodes.push_back(n);
			}
		}
		else
		{
			new_nodes.push_back(n);
		}
	}

	for (size_t i = 0; i < NUM_D3D_INTERFACE_NODES; i++)
	{
		// push back in reverse order to front
		size_t vki_id = NUM_D3D_INTERFACE_NODES - 1 - i;
		KSLD3DUniformInterfaceNode* d3din = d3di_nodes[vki_id];

		if (d3din->members.size() > 0)
		{
			new_nodes.insert(new_nodes.begin(), d3din);
		}
		else
		{
			delete d3din;
		}
	}

	ast->root->nodes = new_nodes;

	return true;
}


class VardefComparator
{
public:
	VardefComparator(KSLProgramAST* ast)
		: m_ast(ast)
	{
	}

	bool operator()(const KSLVariableDefinitionsNode* vdn1, const KSLVariableDefinitionsNode* vdn2)
	{
		KSLVariable &v1 = m_ast->variables[vdn1->variables[0].variable_id];
		KSLVariable &v2 = m_ast->variables[vdn2->variables[0].variable_id];

		return v1.orig_name < v2.orig_name;
	}

private:
	KSLProgramAST* m_ast;
};


bool KSLD3DTranslator::CollectAttributes()
{
	if (m_shader_type == NGL_COMPUTE_SHADER) return true;

	std::vector<KSLASTNode*> new_nodes;

	KSLD3DInputOutputIntefaceNode *d3dii = new KSLD3DInputOutputIntefaceNode(KSL_NODE_D3D_INPUT_INTERFACE);
	d3dii->name = KSL_D3D_INPUT_LAYOUT_TYPE_NAME;

	KSLD3DInputOutputIntefaceNode *d3doi = new KSLD3DInputOutputIntefaceNode(KSL_NODE_D3D_OUTPUT_INTERFACE);
	d3doi->name = KSL_D3D_OUTPUT_LAYOUT_TYPE_NAME;

	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vardef = (KSLVariableDefinitionsNode*)n;

			KSLVariable v = ast->variables[vardef->variables[0].variable_id];
			std::stringstream sstream;

			if (vardef->storage_type == KSL_STORAGE_IN)
			{
				vardef->start_line = 0;
				d3dii->members.push_back(vardef);
				d3dii->qualifiers[v.orig_name] = v.orig_name;
			}
			else if (vardef->storage_type == KSL_STORAGE_OUT)
			{
				vardef->start_line = 0;
				d3doi->members.push_back(vardef);

				if (m_shader_type == NGL_FRAGMENT_SHADER)
				{
					sstream << "SV_TARGET" << vardef->int_attribs.at(KSL_ATTRIB_QUALIFIER_COLOR);
				}
				else
				{
					sstream << v.orig_name;
				}

				d3doi->qualifiers[v.orig_name] = sstream.str();;
			}
			else
			{
				new_nodes.push_back(n);
			}
		}
		else
		{
			new_nodes.push_back(n);
		}
	}

	VardefComparator vdnc(ast);
	std::stable_sort(d3dii->members.begin(), d3dii->members.end(), vdnc);
	std::stable_sort(d3doi->members.begin(), d3doi->members.end(), vdnc);

	if (m_shader_type == NGL_VERTEX_SHADER)
	{
		d3doi->members.push_back(CreatePositionVardef());
		d3doi->qualifiers["gl_Position"] = ("SV_POSITION");
	}

	if (d3doi->members.size() > 0)
	{
		m_has_output_interface = true;
		new_nodes.insert(new_nodes.begin(), d3doi);
	}
	else
	{
		delete d3doi;
	}


	if (d3dii->members.size() > 0)
	{
		m_has_input_interface = true;
		new_nodes.insert(new_nodes.begin(), d3dii);
	}
	else
	{
		delete d3dii;
	}

	ast->root->nodes = new_nodes;

	return true;
}


bool KSLD3DTranslator::CreateMainFunction()
{
	uint32_t main_id = ast->GetMainFunctionNodeId();
	assert(main_id != KSL_UINT32_MAX);

	KSLFunctionNode* main_node = (KSLFunctionNode*)ast->root->nodes[main_id];
	main_node->node_type = KSL_NODE_D3D_MAIN_FUNCTION;

	std::vector<KSLASTNode*> &body_nodes = main_node->body->nodes;

	if (m_has_output_interface)
	{
		// Add output define node
		{
			KSLASTNode* define_output_done = new KSLASTNode();
			define_output_done->node_type = KSL_NODE_D3D_OUTPUT_VARIABLE_DEFINITION;
			body_nodes.insert(body_nodes.begin(), define_output_done);
		}

		// Add return statement
		{

			KSLReturnStatementNode* out_return = new KSLReturnStatementNode();
			body_nodes.push_back(out_return);
		}
	}

	return true;
}


bool KSLD3DTranslator::CollectNumThread()
{
	if (m_shader_type != NGL_COMPUTE_SHADER) return true;

	for (std::vector<KSLASTNode*>::iterator it = ast->root->nodes.begin(); it != ast->root->nodes.end() ; it++)
	{
		if ((*it)->node_type == KSL_NODE_NUMTHREAD)
		{
			m_num_thread_node = dynamic_cast<KSLNumThreadNode*>(*it);
			ast->root->nodes.erase(it);
			return true;
		}
	}

	return false;
}

