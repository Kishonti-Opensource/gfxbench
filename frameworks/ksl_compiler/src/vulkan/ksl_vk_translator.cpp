/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_vk_translator.h"

#include "ksl_vk_nodes.h"
#include "ngl.h"

#include <sstream>
#include <algorithm>
#include <assert.h>


#define DUMP_PUSH_CONSTANT_INFO 0
#define BIND_SSBO_AS_UBO 1

const uint32_t DEFAULT_PUSH_CONSTANT_BYTES = 128;

uint32_t KSLVKTranslator::m_pust_constant_count = DEFAULT_PUSH_CONSTANT_BYTES;


KSLVKTranslator::KSLVKTranslator(bool vk_use_subpass)
{
	for (size_t i = 0; i < KSL_VK_NUM_UNIFORM_INTERFACE; i++)
	{
		m_has_uniform_interface[i] = false;
		m_num_buffers_per_group[i] = 0;
		m_num_samplers_per_group[i] = 0;
		m_num_images_per_group[i] = 0;
		m_num_subpasses_per_group[i] = 0;
	}
	m_uniform_info = NULL;
	m_group_usage.clear();
	m_interface_variables.clear();
	m_push_contants.clear();
	m_vk_use_subpass = vk_use_subpass;
}


KSLVKTranslator::~KSLVKTranslator()
{

}


KSLVulkanUniformInterfaceType KSLVKTranslator::NGLGroupToKSLUniformInterfaceType(NGL_shader_uniform_group ngl_group)
{
	switch (ngl_group)
	{
	case NGL_GROUP_PER_DRAW: return KSL_VK_UNIFORM_INTERFACE_PER_DRAW;
	case NGL_GROUP_PER_RENDERER_CHANGE: return KSL_VK_UNIFORM_INTERFACE_RENDERER_CHANGE;
	case NGL_GROUP_MANUAL: return KSL_VK_UNIFORM_INTERFACE_MANUAL;
	default:
		break;
	}

	return KSL_VK_UNIFORM_INTERFACE_INVALID;
}


uint32_t KSLVKTranslator::GetTypeSize(KSLType t)
{
	if (t.precision == KSL_PRECISION_HIGH)
	{
		switch (t.id)
		{
		// float
		case KSL_TYPE_FLOAT: return 16; // 4;
		case KSL_TYPE_VEC2:  return 16; // 8;
		case KSL_TYPE_VEC4:  return 16;

		// matrices
		case KSL_TYPE_MAT4: return 64;

		// integers
		case KSL_TYPE_UINT4: return 16;

		case KSL_TYPE_UINT:
		case KSL_TYPE_INT: return 16; // 4;

		default:
			break;
		}
	}
	
	assert(0);
	return KSL_INT32_MAX;
}


uint64_t KSLVKTranslator::GetVariableSize(const KSLVariableDefinitionsNode::InnerNode &vardef_node) const
{
	uint32_t v_id = vardef_node.variable_id;
	KSLVariable &v = ast->variables[v_id];

	if (v.type.IsArray())
	{
		return vardef_node.array_size * GetTypeSize(v.type.GetBaseType());
	}
	else
	{
		return GetTypeSize(v.type);
	}
}


void KSLVKTranslator::GetUniformSize(std::vector<uint64_t> &uniform_sizes) const
{
	uniform_sizes.resize(KSL_VK_NUM_UNIFORM_INTERFACE, 0);

	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];
		KSLVariable *v = NULL;

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vardef = (KSLVariableDefinitionsNode*)n;
			if (!(vardef->storage_type == KSL_STORAGE_UNIFORM)) continue;

			assert(vardef->variables.size() == 1);

			uint32_t v_id = vardef->variables[0].variable_id;
			v = &ast->variables[v_id];

			{
				KSLType &bt = v->type.IsArray() ? v->type.GetBaseType() : v->type;
				if (!(bt.IsBool() || bt.IsNumeric())) continue;
			}
			
			KSLVulkanUniformInterfaceType uit = m_variable_group_table.at(v_id);

			uniform_sizes[uit] += GetVariableSize(vardef->variables[0]);
		}
	}
}


uint64_t KSLVKTranslator::GetUniformSize(KSLVulkanUniformInterfaceType vui_type) const
{
	std::vector<uint64_t> uniform_sizes;
	GetUniformSize(uniform_sizes);
	return uniform_sizes[vui_type];
}


bool KSLVKTranslator::InitGroupUsage()
{
	if (m_uniform_info == NULL)
	{
		ErrorUniformInfoNotSet();
		return false;
	}


	// collect interface variables
	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];
		uint32_t v_id = KSL_INT32_MAX;

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vardef = (KSLVariableDefinitionsNode*)n;

			bool is_interface_variable = (vardef->storage_type == KSL_STORAGE_BUFFER) || (vardef->storage_type == KSL_STORAGE_UNIFORM);
				
			if (!is_interface_variable) continue;

			assert(vardef->variables.size() == 1);
			v_id = vardef->variables[0].variable_id;
		}
		else if (n->node_type == KSL_NODE_IMAGE_DEFINITION)
		{
			KSLImageDefinitionNode* idn = (KSLImageDefinitionNode*)n;
			v_id = idn->variable_id;
		}

		if (v_id != KSL_INT32_MAX)
		{
			m_interface_variables.insert(v_id);
		}
	}


	// initalize variable group table
	for (std::set<uint32_t>::iterator it = m_interface_variables.begin(); it != m_interface_variables.end(); it++)
	{
		size_t i = 0;
		for (; i < m_uniform_info->size(); i++)
		{
			if (ast->variables[*it].orig_name == m_uniform_info->at(i).m_name) break;
		}

		if (i == m_uniform_info->size())
		{
			ErrorUniformInfoNotSetFor(ast->variables[*it].orig_name);
			continue;
		}

		KSLVulkanUniformInterfaceType uit = NGLGroupToKSLUniformInterfaceType(m_uniform_info->at(i).m_group);
		m_variable_group_table[*it] = uit;
	}

	m_initial_variable_group_table = m_variable_group_table;

	return true;
}


bool KSLVKTranslator::CollectPushContants(KSLVulkanUniformInterfaceType vuit, uint64_t &remaining_push_constants)
{
	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];

		// TODO: find the best fit next variable

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vardef = (KSLVariableDefinitionsNode*)n;
			if (!(vardef->storage_type == KSL_STORAGE_UNIFORM)) continue;

			assert(vardef->variables.size() == 1);

			uint32_t v_id = vardef->variables[0].variable_id;

			const KSLVariable &v = ast->variables[v_id];
			if (v.type.IsArray()) continue;

			{
				KSLType &bt = vardef->variable_type;
				if (!(bt.IsBool() || bt.IsNumeric())) continue;
			}

			if (m_variable_group_table.at(v_id) != vuit) continue;

			const uint64_t variable_size = GetVariableSize(vardef->variables[0]);

			if (variable_size <= remaining_push_constants)
			{
				m_variable_group_table[v_id] = KSL_VK_UNIFORM_INTERFACE_PUSH_CONSTANT;
				remaining_push_constants -= variable_size;

				m_push_contants.push_back(vardef);

				// array push constants not allowed;
				assert(!v.type.IsArray());
			}
		}
	}

	return true;
}


bool KSLVKTranslator::SetPushConstantOffsets(uint32_t &offset)
{
	for (size_t i = 0; i < m_push_contants.size(); i++)
	{
		m_push_contants[i]->int_attribs[KSL_ATTRIB_QUALIFIER_VK_OFFSET] = offset;
		offset += (uint32_t)GetVariableSize(m_push_contants[i]->variables[0]);
	}

	return true;
}


bool KSLVKTranslator::DetectGroupUsage()
{
	std::map<uint32_t, KSLVulkanUniformInterfaceType>::const_iterator it = m_variable_group_table.begin(); 
	for (; it != m_variable_group_table.end(); it++)
	{
		m_group_usage.insert(it->second);
	}

	return true;
}


bool KSLVKTranslator::Translate()
{
	if (ast == NULL) return false;

	bool s = true;

	if (s) s &= m_errors.size() == 0;

	if (s) s &= RenameBufferInnerData();

	if (s) s &= CreateUniformInterfaceBlock();

	if (s) s &= SetResourceAttribs();

	if (s) s &= SetAttribLocations();

	if (s) s &= TranslateSubpassInput();

	if (ast->shader_type == NGL_VERTEX_SHADER)
	{
		ast->variables[ast->inbuilt_vertex_id_variable_id].new_name = "gl_VertexIndex";
	}

	return s;
}


bool KSLVKTranslator::ProcessStageInfo(std::vector<KSLTranslator*> translators)
{
	std::vector<KSLVKTranslator*> vk_translators(translators.size(), NULL);

	// get used groups per stage
	for (size_t i = 0; i < NGL_NUM_SHADER_TYPES; i++)
	{
		if (translators[i] == NULL) continue;
		vk_translators[i] = dynamic_cast<KSLVKTranslator*>(translators[i]);
		bool s = vk_translators[i]->InitGroupUsage();

		vk_translators[i]->DumpUniformUsage();

		if (!s) return false;
	}


	uint64_t remaining_push_costants = m_pust_constant_count;

	const uint32_t UNIFORM_TYPES_COUNT = 3;
	KSLVulkanUniformInterfaceType it_order[UNIFORM_TYPES_COUNT] = { KSL_VK_UNIFORM_INTERFACE_PER_DRAW, KSL_VK_UNIFORM_INTERFACE_RENDERER_CHANGE, KSL_VK_UNIFORM_INTERFACE_MANUAL };

	const uint32_t NUM_RENDER_STAGES = 5;
	NGL_shader_type stage_order[NUM_RENDER_STAGES] = { NGL_FRAGMENT_SHADER, NGL_VERTEX_SHADER, NGL_TESS_EVALUATION_SHADER, NGL_TESS_CONTROL_SHADER, NGL_GEOMETRY_SHADER };

	if (translators[NGL_COMPUTE_SHADER] == NULL)
	{
		for (size_t i = 0; i < UNIFORM_TYPES_COUNT; i++)
		{
			for (size_t j = 0; j < NUM_RENDER_STAGES; j++)
			{
				KSLVKTranslator* t = vk_translators[stage_order[j]];
				if (t == NULL) continue;

				t->CollectPushContants(it_order[i], remaining_push_costants);
			}
		}
	}
	else
	{
		for (size_t i = 0; i < UNIFORM_TYPES_COUNT; i++)
		{
			vk_translators[NGL_COMPUTE_SHADER]->CollectPushContants(it_order[i], remaining_push_costants);
		}
	}


	uint32_t push_constant_offset = 0;

	if (translators[NGL_COMPUTE_SHADER] == NULL)
	{
		for (size_t j = 0; j < NUM_RENDER_STAGES; j++)
		{
			KSLVKTranslator* t = vk_translators[stage_order[j]];
			if (t == NULL) continue;

			t->SetPushConstantOffsets(push_constant_offset);
		}
	}
	else
	{
		vk_translators[NGL_COMPUTE_SHADER]->SetPushConstantOffsets(push_constant_offset);
	}


	for (size_t i = 0; i < NGL_NUM_SHADER_TYPES; i++)
	{
		if (translators[i] == NULL) continue;
		vk_translators[i]->DetectGroupUsage();
		vk_translators[i]->DumpUniformUsage();
	}

#if DUMP_PUSH_CONSTANT_INFO
	uint32_t used_push_constants = 0;
	for (size_t i = 0; i < NGL_NUM_SHADER_TYPES; i++)
	{
		if (translators[i] == NULL) continue;
		used_push_constants += (uint32_t)vk_translators[i]->GetUniformSize(KSL_VK_UNIFORM_INTERFACE_PUSH_CONSTANT);
	}
	printf("push constant utilization: %d/%d\n\n", used_push_constants, MAX_PUSH_CONSTANT_BYTES);
#endif

	// global group usage
	std::set<KSLVulkanUniformInterfaceType> global_group_usage;
	for (uint32_t i = 0; i < NGL_NUM_SHADER_TYPES; i++)
	{
		if (translators[i] == NULL) continue;
		std::set<KSLVulkanUniformInterfaceType>::const_iterator it = vk_translators[i]->m_group_usage.begin();
		for ( ; it != vk_translators[i]->m_group_usage.end(); it++)
		{
			global_group_usage.insert(*it);
		}
	}

	KSLVulkanUniformInterfaceType group_precedence[KSL_VK_NUM_UNIFORM_INTERFACE] = {
		KSL_VK_UNIFORM_INTERFACE_PER_DRAW,
		KSL_VK_UNIFORM_INTERFACE_RENDERER_CHANGE,
		KSL_VK_UNIFORM_INTERFACE_MANUAL,
		KSL_VK_UNIFORM_INTERFACE_PUSH_CONSTANT
	};

	std::map<KSLVulkanUniformInterfaceType, uint32_t> group_mapping;
	uint32_t next_group_id = 0;
	for (uint32_t i = 0; i < KSL_VK_NUM_UNIFORM_INTERFACE; i++)
	{
		if (global_group_usage.find(group_precedence[i]) != global_group_usage.end())
		{
			group_mapping[group_precedence[i]] = next_group_id;
			next_group_id++;
		}
	}

	// create mapping tables
	for (uint32_t i = 0; i < NGL_NUM_SHADER_TYPES; i++)
	{
		if (translators[i] == NULL) continue;
		vk_translators[i]->m_group_mapping = group_mapping;
		vk_translators[i]->NumerateInterfaceVariables();
	}


	if (translators[NGL_COMPUTE_SHADER] != NULL)
	{
		KSLVKTranslator* vkt = vk_translators[NGL_COMPUTE_SHADER];
		for (size_t i = 0; i < KSL_VK_NUM_UNIFORM_INTERFACE; i++)
		{
			if (vkt->m_has_uniform_interface[i])
			{
				vkt->m_uniform_interface_block_binding[i] = 0;
				vkt->m_first_sampler_binding[i] = 1;
			}
			else
			{
				vkt->m_first_sampler_binding[i] = 0;
			}
			vkt->m_first_buffer_binding[i] = vkt->m_first_sampler_binding[i] + vkt->m_num_samplers_per_group[i];
			vkt->m_first_image_binding[i] = vkt->m_first_buffer_binding[i] + vkt->m_num_buffers_per_group[i];
			vkt->m_first_subpass_binding[i] = vkt->m_first_image_binding[i] + vkt->m_num_images_per_group[i];
		}
	}
	else
	{
		uint32_t m_last_binding_points[KSL_VK_NUM_UNIFORM_INTERFACE];
		for (size_t i = 0; i < KSL_VK_NUM_UNIFORM_INTERFACE; i++)
		{
			m_last_binding_points[i] = 0;
		}

		const uint32_t NUM_RENDER_STAGES = 5;
		NGL_shader_type stage_order[NUM_RENDER_STAGES] = { NGL_VERTEX_SHADER, NGL_TESS_CONTROL_SHADER, NGL_TESS_EVALUATION_SHADER, NGL_GEOMETRY_SHADER, NGL_FRAGMENT_SHADER };

		for (uint32_t j = 0; j < NUM_RENDER_STAGES; j++)
		{
			uint32_t i = stage_order[j];
			if (translators[i] == NULL) continue;

			KSLVKTranslator* vkt = vk_translators[i];
			for (size_t i = 0; i < KSL_VK_NUM_UNIFORM_INTERFACE; i++)
			{
				if (vkt->m_has_uniform_interface[i])
				{
					vkt->m_uniform_interface_block_binding[i] = m_last_binding_points[i];
					vkt->m_first_sampler_binding[i] = vkt->m_uniform_interface_block_binding[i] + 1;
				}
				else
				{
					vkt->m_first_sampler_binding[i] = m_last_binding_points[i];
				}
				vkt->m_first_buffer_binding[i] = vkt->m_first_sampler_binding[i] + vkt->m_num_samplers_per_group[i];
				vkt->m_first_image_binding[i] = vkt->m_first_buffer_binding[i] + vkt->m_num_buffers_per_group[i];
				vkt->m_first_subpass_binding[i] = vkt->m_first_image_binding[i] + vkt->m_num_images_per_group[i];
				m_last_binding_points[i] = vkt->m_first_subpass_binding[i] + vkt->m_num_subpasses_per_group[i];
			}
		}

		
		for (int32_t i = NUM_RENDER_STAGES - 1; i > 0; i--)
		{
			KSLVKTranslator* dst_vkt = vk_translators[stage_order[i]];
			if (dst_vkt == NULL) continue;

			for (int32_t j = i - 1; j >= 0; j--)
			{
				KSLVKTranslator* src_vkt = vk_translators[stage_order[j]];
				if (src_vkt == NULL) continue;

				dst_vkt->m_in_attrib_mapping = src_vkt->ast->out_attributes;
			}
		}
	}
	
	return true;
}


class KSLVKTranslator::PushConstantGroupComparator
{
public:
	PushConstantGroupComparator(KSLVKTranslator* translator)
		: m_translator(translator)
	{
	}

	bool operator()(const KSLVariableDefinitionsNode* vdn1, const KSLVariableDefinitionsNode* vdn2)
	{
		KSLVulkanUniformInterfaceType uit1 = m_translator->m_initial_variable_group_table[vdn1->variables[0].variable_id];
		KSLVulkanUniformInterfaceType uit2 = m_translator->m_initial_variable_group_table[vdn2->variables[0].variable_id];

		return uit1 < uit2;
	}

private:
	KSLVKTranslator* m_translator;
};


bool KSLVKTranslator::CreateUniformInterfaceBlock()
{
	std::vector<KSLASTNode*> new_nodes;

	std::vector<KSLVulkanInterfaceNode *> vki_nodes(KSL_VK_NUM_UNIFORM_INTERFACE);
	for (size_t i = 0; i < KSL_VK_NUM_UNIFORM_INTERFACE; i++)
	{
		if (!m_has_uniform_interface[i]) continue;

		vki_nodes[i] = new KSLVulkanInterfaceNode;
		vki_nodes[i]->m_group_id = m_group_mapping.at((KSLVulkanUniformInterfaceType)i);
		vki_nodes[i]->m_binding_point = m_uniform_interface_block_binding[i];
		vki_nodes[i]->m_group_type = (uint32_t)i;
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
				assert(vardef->variables.size() == 1);

				uint32_t v_id = vardef->variables[0].variable_id;

				std::map<uint32_t, KSLVulkanUniformInterfaceType>::const_iterator it = m_variable_group_table.find(v_id);
				if (it != m_variable_group_table.end())
				{
					// push constant collected at setup push constants but removed at here
					if (it->second != KSL_VK_UNIFORM_INTERFACE_PUSH_CONSTANT)
					{
						vki_nodes[it->second]->members.push_back(vardef);
					}
				}
				else
				{
					KSLVariable &v = ast->variables[v_id];
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

	if (vki_nodes[KSL_VK_UNIFORM_INTERFACE_PUSH_CONSTANT] != NULL)
	{
		vki_nodes[KSL_VK_UNIFORM_INTERFACE_PUSH_CONSTANT]->members = m_push_contants;

		// TODO: remove:
		//std::vector<KSLVariableDefinitionsNode*> &pushconstant_nodes = vki_nodes[KSL_VK_UNIFORM_INTERFACE_PUSH_CONSTANT]->members;
		//std::stable_sort(pushconstant_nodes.begin(), pushconstant_nodes.end(), PushConstantGroupComparator(this));
	}
	
	for (size_t i = 0; i < KSL_VK_NUM_UNIFORM_INTERFACE; i++)
	{
		// push back in reverse order to front
		size_t vki_id = KSL_VK_NUM_UNIFORM_INTERFACE - 1 - i;
		KSLVulkanInterfaceNode* vki = vki_nodes[vki_id];
		
		if (vki != NULL) new_nodes.insert(new_nodes.begin(), vki);
	}

	ast->root->nodes = new_nodes;

	return true;
}


bool KSLVKTranslator::NumerateInterfaceVariables()
{
	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vardef = (KSLVariableDefinitionsNode*)n;
			KSLType bt = vardef->variable_type;

			assert(vardef->variables.size() == 1);
			uint32_t v_id = vardef->variables[0].variable_id;

			bool is_buffer = vardef->storage_type == KSL_STORAGE_BUFFER;
			bool is_uniform = vardef->storage_type == KSL_STORAGE_UNIFORM;
			if (!is_buffer && !is_uniform)
			{
				continue;
			}

			if (m_variable_group_table.find(v_id) == m_variable_group_table.end())
			{
				KSLVariable &v = ast->variables[v_id];
				ErrorUniformInfoNotSetFor(v.orig_name);
				continue;
			}

			size_t group_id = m_variable_group_table.at(v_id);

			if (is_buffer)
			{
				m_num_buffers_per_group[group_id]++;
			}
			else if (is_uniform && bt.IsSampler())
			{
				m_num_samplers_per_group[group_id]++;
			}
			else if (is_uniform && bt.IsSubpassInput())
			{
				m_num_subpasses_per_group[group_id]++;
			}

			bool to_uniform_interface = true;
			to_uniform_interface &= is_uniform;
			to_uniform_interface &= bt.IsNumeric() || bt.IsBool();

			if (to_uniform_interface)
			{
				m_has_uniform_interface[group_id] = true;
			}
		}
		else if (n->node_type == KSL_NODE_IMAGE_DEFINITION)
		{
			KSLImageDefinitionNode* idn = (KSLImageDefinitionNode*)n;
			uint32_t v_id = idn->variable_id;

			size_t group_id = m_variable_group_table.at(v_id);
			m_num_images_per_group[group_id]++;
		}
	}

	return true;
}


bool KSLVKTranslator::SetResourceAttribs()
{
	uint32_t next_sampler_binding[KSL_VK_NUM_UNIFORM_INTERFACE];
	uint32_t next_buffer_binding[KSL_VK_NUM_UNIFORM_INTERFACE];
	uint32_t next_image_binding[KSL_VK_NUM_UNIFORM_INTERFACE];

	for (size_t i = 0; i < KSL_VK_NUM_UNIFORM_INTERFACE; i++)
	{
		next_sampler_binding[i] = m_first_sampler_binding[i];
		next_buffer_binding[i] = m_first_buffer_binding[i];
		next_image_binding[i] = m_first_image_binding[i];
	}

	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vardef = (KSLVariableDefinitionsNode*)n;
			KSLType bt = vardef->variable_type;

			assert(vardef->variables.size() == 1);
			uint32_t v_id = vardef->variables[0].variable_id;

			bool is_buffer = vardef->storage_type == KSL_STORAGE_BUFFER;
			bool is_uniform = vardef->storage_type == KSL_STORAGE_UNIFORM;

			if(!is_buffer && !is_uniform)
			{
				continue;
			}

			size_t group_id = m_variable_group_table.at(v_id);

			if (is_uniform && bt.IsSampler())
			{
				vardef->int_attribs[KSL_ATTRIB_QUALIFIER_GL_BINDING] = next_sampler_binding[group_id]++;
			}

			if (is_buffer)
			{
				vardef->int_attribs[KSL_ATTRIB_QUALIFIER_GL_BINDING] = next_buffer_binding[group_id]++;
#if BIND_SSBO_AS_UBO == 0
				vardef->bool_attribs.insert(KSL_ATTRIB_QUALIFIER_SSBO);
#endif
			}

			if (m_variable_group_table.find(v_id) != m_variable_group_table.end())
			{
				KSLVulkanUniformInterfaceType uit = m_variable_group_table.at(v_id);
				vardef->int_attribs[KSL_ATTRIB_QUALIFIER_VK_GROUP_ID] = m_group_mapping.at(uit);
			}
			else
			{
				KSLVariable &v = ast->variables[v_id];
				ErrorUniformInfoNotSetFor(v.orig_name);
			}
		}
		else if (n->node_type == KSL_NODE_IMAGE_DEFINITION)
		{
			KSLImageDefinitionNode* idn = (KSLImageDefinitionNode*)n;

			uint32_t v_id = idn->variable_id;
			
			if (m_variable_group_table.find(v_id) != m_variable_group_table.end())
			{
				KSLVulkanUniformInterfaceType uit = m_variable_group_table.at(v_id);
				idn->int_attribs[KSL_ATTRIB_QUALIFIER_VK_GROUP_ID] = m_group_mapping.at(uit);
				idn->int_attribs[KSL_ATTRIB_QUALIFIER_GL_BINDING] = next_image_binding[uit]++;
			}
			else
			{
				KSLVariable &v = ast->variables[v_id];
				ErrorUniformInfoNotSetFor(v.orig_name);
			}
		}
	}
	
	return true;
}


bool KSLVKTranslator::SetAttribLocations()
{
	if (m_shader_type == NGL_COMPUTE_SHADER) return true;

	std::map<std::string, uint32_t> in_mapping;
	std::map<std::string, uint32_t> out_mapping;

	// create input attrib mapping
	{
		std::vector<KSLVariable> *in_mapping_vector = (m_shader_type == NGL_VERTEX_SHADER) ? &ast->in_attributes : &m_in_attrib_mapping;

		if ( (ast->in_attributes.size() != 0) && (in_mapping_vector->size() == 0) )
		{
			std::stringstream sstream;
			sstream << "input attribute mapping not set!" << std::endl;
			m_errors.push_back(KSLError(KSL_TRANSLATOR, KSL_ERROR, 0, 0, sstream.str()));

			in_mapping_vector = &ast->in_attributes;
		}

		for (size_t i = 0; i < in_mapping_vector->size(); i++)
		{
			in_mapping[in_mapping_vector->at(i).orig_name] = (uint32_t)i;
		}
	}


	// create output attrib mapping
	{
		for (size_t i = 0; i < ast->out_attributes.size(); i++)
		{
			out_mapping[ast->out_attributes[i].orig_name] = (uint32_t)i;
		}
	}


	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vardef = (KSLVariableDefinitionsNode*)n;

			assert(vardef->variables.size() == 1);

			KSLVariable v = ast->variables[vardef->variables[0].variable_id];

			std::map<std::string, uint32_t>::const_iterator cit;
			if (vardef->storage_type == KSL_STORAGE_IN)
			{
				cit = in_mapping.find(v.orig_name);
				assert(cit != in_mapping.end());
				vardef->int_attribs[KSL_ATTRIB_QUALIFIER_VK_LOCATION] = cit->second;
			}

			// do not set out attrib locations for fragment shader
			if ( (vardef->storage_type == KSL_STORAGE_OUT) && (m_shader_type != NGL_FRAGMENT_SHADER) )
			{
				cit = out_mapping.find(v.orig_name);
				assert(cit != out_mapping.end());
				vardef->int_attribs[KSL_ATTRIB_QUALIFIER_VK_LOCATION] = cit->second;
			}
		}
	}

	return true;
}


bool KSLVKTranslator::TranslateSubpassInput()
{
	uint32_t next_subpass_binding[KSL_VK_NUM_UNIFORM_INTERFACE];
	for (size_t i = 0; i < KSL_VK_NUM_UNIFORM_INTERFACE; i++)
	{
		next_subpass_binding[i] = m_first_subpass_binding[i];
	}

	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vardef = (KSLVariableDefinitionsNode*)n;
			KSLVariable &v = ast->variables[vardef->variables[0].variable_id];
			KSLType &t = v.type;

			if (vardef->storage_type == KSL_STORAGE_UNIFORM)
			{
				if (t.id == KSL_TYPE_SUBPASS_INPUT)
				{
					int id = KSL_UINT32_MAX;
					if (vardef->int_attribs.find(KSL_ATTRIB_QUALIFIER_COLOR) != vardef->int_attribs.end())
					{
						id = vardef->int_attribs[KSL_ATTRIB_QUALIFIER_COLOR];
					    vardef->int_attribs.erase(KSL_ATTRIB_QUALIFIER_COLOR);
					}
					else
					{
						id = vardef->int_attribs[KSL_ATTRIB_QUALIFIER_DEPTH];
						vardef->int_attribs.erase(KSL_ATTRIB_QUALIFIER_DEPTH);
					}

					assert(vardef->variables.size() == 1);
					uint32_t v_id = vardef->variables[0].variable_id;
					size_t group_id = m_variable_group_table.at(v_id);

					vardef->int_attribs[KSL_ATTRIB_QUALIFIER_GL_BINDING] = next_subpass_binding[group_id];

					if (m_vk_use_subpass)
					{
						vardef->int_attribs[KSL_ATTRIB_QUALIFIER_VK_INPUT_ATTACHMENT_ID] = id;
					}

					next_subpass_binding[group_id]++;
				}
			}
		}
	}

	return true;
}


void KSLVKTranslator::DumpUniformUsage() const
{
#if !DUMP_PUSH_CONSTANT_INFO
	return;
#endif

	printf("\n");

	std::vector<uint64_t> uniform_sizes;
	GetUniformSize(uniform_sizes);

	for (size_t i = 0; i < KSL_VK_NUM_UNIFORM_INTERFACE; i++)
	{
		printf("%s ",GetShaderTypeName((NGL_shader_type)m_shader_type).c_str());

		switch (i)
		{
		case KSL_VK_UNIFORM_INTERFACE_PER_DRAW:        printf("per draw:        "); break;
		case KSL_VK_UNIFORM_INTERFACE_RENDERER_CHANGE: printf("renderer change: "); break;
		case KSL_VK_UNIFORM_INTERFACE_MANUAL:          printf("manual:          "); break;
		case KSL_VK_UNIFORM_INTERFACE_PUSH_CONSTANT:   printf("push constant:   "); break;
		default:
			assert(0);
			break;
		}

		printf("%d\n", (uint32_t)uniform_sizes[i]);
	}

	printf("\n");
}

