/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_MTL_TRANSLATOR__
#define __KSL_MTL_TRANSLATOR__

#include "ksl_translator.h"


const std::string KSL_METAL_INPUT_LAYOUT_TYPE_NAME = "ksl_mtl_input_layout_type__";
const std::string KSL_METAL_OUTPUT_LAYOUT_TYPE_NAME = "ksl_mtl_output_layout_type__";

const std::string KSL_METAL_INPUT_NAME = "_ksl_mtl_input_";
const std::string KSL_METAL_OUTPUT_NAME = "_ksl_mtl_output_";


class KSLMetalTranslator : public KSLTranslator
{
public:
	KSLMetalTranslator(bool mtl_use_subpass);
	~KSLMetalTranslator();

	virtual bool Translate();

	bool m_has_input_interface;
	bool m_has_output_interface;
	std::map<NGL_shader_uniform_group, bool> m_has_uniform_interface;

	static uint32_t NGLGroupToUniformIntefaceBinding(NGL_shader_uniform_group ngl_group);
	static std::string NGLGroupToUniformIntefaceTypeName(NGL_shader_uniform_group ngl_group);
	static std::string NGLGroupToUniformIntefaceVariableName(NGL_shader_uniform_group ngl_group);

	std::map<std::string, NGL_shader_uniform_group> m_uniform_variable_name_to_group;
	std::map<uint32_t, NGL_shader_uniform_group> m_uniform_variable_id_to_group;

protected:

	bool CollectUniforms();
	bool CollectAttributes();
	bool CreateMainFunction();
	
	bool ShrinkFragmentOutput();

	bool m_mtl_use_subpass;
	KSLStructDefinitionNode* m_out_base_type;
};


#endif  // __KSL_MTL_TRANSLATOR__

