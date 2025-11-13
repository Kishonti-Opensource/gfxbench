/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_VK_TRANSLATOR__
#define __KSL_VK_TRANSLATOR__

#include "../gl/ksl_gl_translator.h"
#include "ksl_vk_nodes.h"


class KSLVKTranslator : public KSLGLTranslator
{
public:
	KSLVKTranslator(bool vk_use_subpass);
	virtual ~KSLVKTranslator();

	virtual bool Translate();
	static bool ProcessStageInfo(std::vector<KSLTranslator*> translators);
	
	std::map<uint32_t, KSLVulkanUniformInterfaceType> m_variable_group_table;
	static uint32_t m_pust_constant_count;
	
protected:

	bool m_has_uniform_interface[KSL_VK_NUM_UNIFORM_INTERFACE];
	uint32_t m_num_images_per_group[KSL_VK_NUM_UNIFORM_INTERFACE];
	uint32_t m_num_buffers_per_group[KSL_VK_NUM_UNIFORM_INTERFACE];
	uint32_t m_num_samplers_per_group[KSL_VK_NUM_UNIFORM_INTERFACE];
	uint32_t m_num_subpasses_per_group[KSL_VK_NUM_UNIFORM_INTERFACE];

	uint32_t m_uniform_interface_block_binding[KSL_VK_NUM_UNIFORM_INTERFACE];
	
	uint32_t m_first_sampler_binding[KSL_VK_NUM_UNIFORM_INTERFACE];
	uint32_t m_first_buffer_binding[KSL_VK_NUM_UNIFORM_INTERFACE];
	uint32_t m_first_image_binding[KSL_VK_NUM_UNIFORM_INTERFACE];
	uint32_t m_first_subpass_binding[KSL_VK_NUM_UNIFORM_INTERFACE];

	std::set<uint32_t> m_interface_variables;
	std::set<KSLVulkanUniformInterfaceType> m_group_usage;
	std::map<KSLVulkanUniformInterfaceType, uint32_t> m_group_mapping;
	std::vector<KSLVariable> m_in_attrib_mapping;

	std::map<uint32_t, KSLVulkanUniformInterfaceType> m_initial_variable_group_table;
	class PushConstantGroupComparator;

	bool InitGroupUsage();
	bool CollectPushContants(KSLVulkanUniformInterfaceType vuit, uint64_t &remaining_push_constants);
	bool SetPushConstantOffsets(uint32_t &offset);
	bool DetectGroupUsage();

	bool NumerateInterfaceVariables();
	bool CreateUniformInterfaceBlock();
	bool SetResourceAttribs();
	bool SetAttribLocations();
	virtual bool TranslateSubpassInput();

	uint64_t GetVariableSize(const KSLVariableDefinitionsNode::InnerNode &vardef_node) const;
	void GetUniformSize(std::vector<uint64_t> &uniform_sizes) const;
	uint64_t GetUniformSize(KSLVulkanUniformInterfaceType vui_type) const;
	std::vector<KSLVariableDefinitionsNode*> m_push_contants;

	static KSLVulkanUniformInterfaceType NGLGroupToKSLUniformInterfaceType(NGL_shader_uniform_group ngl_group);
	static uint32_t GetTypeSize(KSLType t);

	bool m_vk_use_subpass;

	void DumpUniformUsage() const;
};


#endif // __KSL_VK_TRANSLATOR__

