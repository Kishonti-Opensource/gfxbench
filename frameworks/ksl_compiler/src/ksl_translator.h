/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_TRANSLATOR__
#define __KSL_TRANSLATOR__

#include "ksl_ast_tree.h"


struct KSLGlobalUsageInfo
{
	bool use_in_attribs;
	bool use_out_attribs;

	bool UseUniforms();
	bool UsedUniformGroup(uint32_t ngl_group)
	{
		return used_uniform_groups.find(ngl_group) != used_uniform_groups.end();
	}

	std::set<uint32_t> used_buffers;
	std::set<uint32_t> used_images;
	std::set<uint32_t> used_samplers;
	std::set<uint32_t> used_shared;
	std::set<uint32_t> used_inbuilts;
	std::set<uint32_t> used_uniform_groups;

	std::set<std::string> called_functions;

	KSLGlobalUsageInfo()
		: use_in_attribs(false)
		, use_out_attribs(false)
	{
	}
};


class KSLTranslator
{
public:
	KSLTranslator()
		: ast(NULL)
	{
	}

	virtual ~KSLTranslator()
	{
	}

	virtual bool Translate() = 0;
	
	bool CreateGlobalUsageInfo();
	
	KSLProgramAST *ast;

	uint32_t m_shader_type;
	std::map<std::string, KSLGlobalUsageInfo> m_global_usage;

	std::vector<KSLError> m_errors;

	const std::vector<NGL_shader_uniform> *m_uniform_info;

protected:

	void ErrorUniformInfoNotSet();
	void ErrorUniformInfoNotSetFor(const std::string &variable_name);

	KSLVariableDefinitionsNode* CreatePositionVardef();
};


#endif // __KSL_TRANSLATOR__

