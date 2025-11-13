/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_mtl_translator.h"

#include "ksl_mtl_nodes.h"

#include <sstream>
#include <assert.h>
#include <algorithm>


KSLMetalTranslator::KSLMetalTranslator(bool mtl_use_subpass)
{
	m_has_input_interface = false;
	m_has_output_interface = false;
	m_out_base_type = NULL;
	m_mtl_use_subpass = mtl_use_subpass;

	m_has_uniform_interface[NGL_GROUP_PER_DRAW] = false;
	m_has_uniform_interface[NGL_GROUP_PER_RENDERER_CHANGE] = false;
	m_has_uniform_interface[NGL_GROUP_MANUAL] = false;
}


KSLMetalTranslator::~KSLMetalTranslator()
{
	delete m_out_base_type;
}


bool KSLMetalTranslator::Translate()
{
	if (ast == NULL) return false;

	bool s = true;

	// The call order may be relevant!
	if (s) s &= ShrinkFragmentOutput();
	
	if (s) s &= CollectAttributes();

	if (s) s &= CollectUniforms();

	if (s) s &= CreateGlobalUsageInfo();

	if (s) s &= CreateMainFunction();

	return s;
}


bool KSLMetalTranslator::CreateMainFunction()
{
	KSLMetalMainFunctionNode *mmfn = new KSLMetalMainFunctionNode();
	std::vector<KSLVariableDefinitionsNode*> shareds;

	ast->CollectSamplerDefinitions(mmfn->samplers, true);
	ast->CollectBufferDefinitions(mmfn->buffers, true);
	ast->CollectImageDefinitions(mmfn->images, true);
	ast->CollectSubpassDefinitions(mmfn->subpass_inputs, true);
	if (ast->shader_type == NGL_COMPUTE_SHADER)
	{
		ast->CollectSharedDefinitions(shareds, true);
	}

	uint32_t main_id = ast->GetMainFunctionNodeId();

	assert(main_id != KSL_UINT32_MAX);

	KSLFunctionNode* orig_main = (KSLFunctionNode*)ast->root->nodes[main_id];

	mmfn->body = orig_main->body;
	orig_main->body = NULL;

	delete orig_main;
	ast->root->nodes[main_id] = mmfn;


	std::vector<KSLASTNode*> &body_nodes = mmfn->body->nodes;

	if (ast->shader_type != NGL_COMPUTE_SHADER)
	{
		// Create output variable definition
		uint32_t out_variable_id = KSL_UINT32_MAX;
		
		if (m_has_output_interface)
		{
			m_out_base_type = new KSLStructDefinitionNode();
			m_out_base_type->name = KSL_METAL_OUTPUT_LAYOUT_TYPE_NAME;
			uint32_t out_base_type_id = (uint32_t)ast->user_types.size() + KSL_NUM_INBUILT_TYPES;
			ast->user_types.push_back(m_out_base_type);

			KSLType out_type(out_base_type_id, KSL_PRECISION_HIGH);

			KSLVariable out_variable(KSL_METAL_OUTPUT_NAME, out_type);
			out_variable_id = (uint32_t)ast->variables.size();
			ast->variables.push_back(out_variable);

			KSLVariableDefinitionsNode::InnerNode vardef_node;
			vardef_node.variable_id = out_variable_id;

			KSLVariableDefinitionsNode* out_vardef = new KSLVariableDefinitionsNode();
			out_vardef->variable_type = out_type;
			out_vardef->variables.push_back(vardef_node);

			body_nodes.insert(body_nodes.begin(), out_vardef);
		}

		// Add return statement
		{

			KSLReturnStatementNode* out_return = new KSLReturnStatementNode();
			body_nodes.push_back(out_return);
		}
	}
	else
	{
		for (size_t i = 0; i < shareds.size(); i++)
		{
			body_nodes.insert(body_nodes.begin(), shareds[i]);
		}
	}


	return true;
}


static uint32_t NGLGroupToUniformIntefaceId(NGL_shader_uniform_group ngl_group)
{
	switch (ngl_group)
	{
	case NGL_GROUP_PER_DRAW: return 0;
	case NGL_GROUP_PER_RENDERER_CHANGE: return 1;
	case NGL_GROUP_MANUAL: return 2;
	default:
		break;
	}

	assert(0);
	return KSL_INT32_MAX;
}


bool KSLMetalTranslator::CollectUniforms()
{
	if (m_uniform_info == NULL)
	{
		ErrorUniformInfoNotSet();
		return false;
	}

	for (size_t i = 0; i < m_uniform_info->size(); i++)
	{
		NGL_shader_uniform_group t = m_uniform_info->at(i).m_group;
		m_uniform_variable_name_to_group[m_uniform_info->at(i).m_name] = t;
	}

	for (uint32_t i = 0; i < ast->variables.size(); i++)
	{
		KSLVariable &v = ast->variables[i];
		
		if (v.storage_type == KSL_STORAGE_UNIFORM)
		{
			auto it = m_uniform_variable_name_to_group.find(v.orig_name);

			if (it != m_uniform_variable_name_to_group.end())
			{
				m_uniform_variable_id_to_group[i] = it->second;
			}
			else
			{
				ErrorUniformInfoNotSetFor(v.orig_name);
			}
		}
	}

	const uint32_t NUM_METAL_INTERFACE_NODES = 3;
	std::vector<KSLASTNode*> new_nodes;

	KSLMetalUniformInterfaceNode* mtli_nodes[NUM_METAL_INTERFACE_NODES];
	NGL_shader_uniform_group ngl_groups[NUM_METAL_INTERFACE_NODES] = { NGL_GROUP_PER_DRAW, NGL_GROUP_PER_RENDERER_CHANGE, NGL_GROUP_MANUAL };

	for (size_t i = 0; i < NUM_METAL_INTERFACE_NODES; i++)
	{
		mtli_nodes[i] = new KSLMetalUniformInterfaceNode;
		mtli_nodes[i]->binding_point = (uint32_t)i;
		mtli_nodes[i]->name = NGLGroupToUniformIntefaceTypeName(ngl_groups[i]);
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

			if (to_uniform_interface)
			{
				vardef->start_line = 0;
				vardef->storage_type = KSL_STORAGE_DEFAULT;
				assert(vardef->variables.size() == 1);

				auto it = m_uniform_variable_id_to_group.find(vardef->variables[0].variable_id);

				if (it != m_uniform_variable_id_to_group.end())
				{
					mtli_nodes[NGLGroupToUniformIntefaceId(it->second)]->members.push_back(vardef);
					m_has_uniform_interface[it->second] = true;
				}
				else
				{
					KSLVariable &v = ast->variables[vardef->variables[0].variable_id];
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

	for (size_t i = 0; i < NUM_METAL_INTERFACE_NODES; i++)
	{
		// push back in reverse order to front
		size_t vki_id = NUM_METAL_INTERFACE_NODES - 1 - i;
		KSLMetalUniformInterfaceNode* mtlin = mtli_nodes[vki_id];

		if (mtlin->members.size() > 0)
		{
			new_nodes.insert(new_nodes.begin(), mtlin);
		}
		else
		{
			delete mtlin;
		}
	}

	ast->root->nodes = new_nodes;

	return true;
}


bool KSLMetalTranslator::CollectAttributes()
{
	if (m_shader_type == NGL_COMPUTE_SHADER) return true;

	// create input attrib mapping
	std::map<std::string, uint32_t> in_mapping;
	if (m_shader_type == NGL_VERTEX_SHADER)
	{
		for (size_t i = 0; i < ast->in_attributes.size(); i++)
		{
			in_mapping[ast->in_attributes.at(i).orig_name] = (uint32_t)i;
		}
	}

	std::vector<KSLASTNode*> new_nodes;

	KSLMetalInputOutputIntefaceNode *mtlii = new KSLMetalInputOutputIntefaceNode(KSL_NODE_METAL_INPUT_INTERFACE);
	mtlii->name = KSL_METAL_INPUT_LAYOUT_TYPE_NAME;
	
	KSLMetalInputOutputIntefaceNode *mtloi = new KSLMetalInputOutputIntefaceNode(KSL_NODE_METAL_OUTPUT_INTERFACE);
	mtloi->name = KSL_METAL_OUTPUT_LAYOUT_TYPE_NAME;
	if (m_shader_type == NGL_VERTEX_SHADER)
	{
		mtloi->members.push_back(CreatePositionVardef());
		mtloi->qualifiers.push_back("position");
	}

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
				mtlii->members.push_back(vardef);

				if (m_shader_type == NGL_VERTEX_SHADER)
				{
					sstream << "attribute(" << in_mapping.at(v.new_name) << ")";
				}
				else
				{
					sstream << "user(" << v.new_name << ")";
				}
				
				mtlii->qualifiers.push_back(sstream.str());
			}
			else if (vardef->storage_type == KSL_STORAGE_OUT)
			{
				vardef->start_line = 0;
				mtloi->members.push_back(vardef);

				if (m_shader_type == NGL_FRAGMENT_SHADER)
				{
					sstream << "color(" << vardef->int_attribs.at(KSL_ATTRIB_QUALIFIER_COLOR) << ")";
				}
				else
				{
					sstream << "user(" << v.new_name << ")";
				}

				mtloi->qualifiers.push_back(sstream.str());
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


	if (mtloi->members.size() > 0)
	{
		m_has_output_interface = true;
		new_nodes.insert(new_nodes.begin(), mtloi);
	}
	else
	{
		delete mtloi;
	}


	if (mtlii->members.size() > 0)
	{
		m_has_input_interface = true;
		new_nodes.insert(new_nodes.begin(), mtlii);
	}
	else
	{
		delete mtlii;
	}

	ast->root->nodes = new_nodes;

	return true;
}


bool KSLMetalTranslator::ShrinkFragmentOutput()
{
	if (m_shader_type != NGL_FRAGMENT_SHADER) return true;
	
	if (m_mtl_use_subpass) return true;
	
	std::vector<KSLVariableDefinitionsNode*> fragment_out;
	ast->CollectVariableDefinitions(fragment_out, false, KSL_STORAGE_OUT, KSL_TYPECLASS_ALL);
	
	std::vector<uint32_t> ids;
	
	for(size_t i = 0; i < fragment_out.size(); i++)
	{
		ids.push_back(fragment_out[i]->int_attribs.at(KSL_ATTRIB_QUALIFIER_COLOR));
	}
	
	std::sort(ids.begin(),ids.end());
	
	for (size_t i = 0; i < ids.size();i++)
	{
		for(size_t j = 0; j < fragment_out.size(); j++)
		{
			if (fragment_out[j]->int_attribs.at(KSL_ATTRIB_QUALIFIER_COLOR) == ids[i])
			{
				fragment_out[j]->int_attribs[KSL_ATTRIB_QUALIFIER_COLOR] = (uint32_t)i;
			}
		}
	}
	
	return true;
}


std::string KSLMetalTranslator::NGLGroupToUniformIntefaceTypeName(NGL_shader_uniform_group ngl_group)
{
	switch (ngl_group)
	{
	case NGL_GROUP_PER_DRAW: return "ksl_mtl_per_draw_uniforms_type__";
	case NGL_GROUP_PER_RENDERER_CHANGE: return "ksl_mtl_per_renderer_change_uniforms_type__";
	case NGL_GROUP_MANUAL: return "ksl_mtl_manual_uniforms_type__";
	default:
		break;
	}

	assert(0);
	return "error_uniform_type";
}


std::string KSLMetalTranslator::NGLGroupToUniformIntefaceVariableName(NGL_shader_uniform_group ngl_group)
{
	switch (ngl_group)
	{
	case NGL_GROUP_PER_DRAW: return "_ksl_mtl_per_draw_uniforms_";
	case NGL_GROUP_PER_RENDERER_CHANGE: return "_ksl_mtl_per_renderer_change_uniforms_";
	case NGL_GROUP_MANUAL: return "_ksl_mtl_manual_uniforms_";
	default:
		break;
	}

	assert(0);
	return "error_uniform_variable";
}


uint32_t KSLMetalTranslator::NGLGroupToUniformIntefaceBinding(NGL_shader_uniform_group ngl_group)
{
	switch (ngl_group)
	{
	case NGL_GROUP_PER_DRAW: return 1;
	case NGL_GROUP_PER_RENDERER_CHANGE: return 2;
	case NGL_GROUP_MANUAL: return 3;
	default:
		break;
	}

	assert(0);
	return KSL_INT32_MAX;
}

