/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_translator.h"

#include <sstream>


KSLVariableDefinitionsNode* KSLTranslator::CreatePositionVardef()
{
	KSLVariableDefinitionsNode *position_vardef = new KSLVariableDefinitionsNode();

	KSLVariable &v = ast->variables[ast->inbuilt_vertex_position_variable_id];
	position_vardef->variable_type = v.type;
	v.storage_type = KSL_STORAGE_OUT;

	KSLVariableDefinitionsNode::InnerNode in;
	in.variable_id = ast->inbuilt_vertex_position_variable_id;
	position_vardef->variables.push_back(in);

	return position_vardef;
}


class KSLCollectGlobalUsageVisitor : public KSLNodeVisitor
{
public:
	KSLCollectGlobalUsageVisitor(KSLProgramAST *ast, std::set<std::string> *user_defined_functions)
        : m_variable_group(NULL)
        , m_ast(ast)
		, m_user_defined_functions(user_defined_functions)
	{
		m_inbuilt_variable_ids.clear();

		if (ast->shader_type == NGL_VERTEX_SHADER)
		{
			m_inbuilt_variable_ids.insert(ast->inbuilt_vertex_id_variable_id);
		}

		if (ast->shader_type == NGL_FRAGMENT_SHADER)
		{
			m_inbuilt_variable_ids.insert(ast->inbuilt_fragment_fragcoord);
			m_inbuilt_variable_ids.insert(ast->inbuilt_fragment_frontfacing);
		}

		if (ast->shader_type == NGL_COMPUTE_SHADER)
		{
			m_inbuilt_variable_ids.insert(ast->inbuilt_compute_globalinvocationid_id);
			m_inbuilt_variable_ids.insert(ast->inbuilt_compute_localinvocationindex_id);
			m_inbuilt_variable_ids.insert(ast->inbuilt_compute_localinvocationid_id);
			m_inbuilt_variable_ids.insert(ast->inbuilt_compute_workgroupid_id);
		}
	}

	KSLGlobalUsageInfo m_globalusageinfo;
	std::map<std::string, uint32_t> *m_variable_group;

private:
	bool visit(KSLASTNode* n)
	{
		if (n->node_type == KSL_NODE_FUNCTION_CALL_EXPRESSION)
		{
			KSLFunctionCallExpressionNode *fcen = (KSLFunctionCallExpressionNode*)n;

			if (m_user_defined_functions->find(fcen->name) != m_user_defined_functions->end())
			{
				m_globalusageinfo.called_functions.insert(fcen->name);
			}
		}
		else if (n->node_type == KSL_NODE_VARIABLE_EXPRESSION)
		{
			KSLVariableExpressionNode *ven = (KSLVariableExpressionNode*)n;
			uint32_t v_id = ven->variable_id;
			KSLVariable &v = m_ast->variables[v_id];

			switch (v.storage_type)
			{
			case KSL_STORAGE_OUT:
				m_globalusageinfo.use_out_attribs |= true;
				break;

			case KSL_STORAGE_IN:
				m_globalusageinfo.use_in_attribs |= true;
				break;

			case KSL_STORAGE_UNIFORM:
				if (v.type.IsSampler())
				{
					m_globalusageinfo.used_samplers.insert(v_id);
				}
				else
				{
					std::map<std::string, uint32_t>::const_iterator it = m_variable_group->find(v.orig_name);					
					if (it != m_variable_group->end())
					{
						m_globalusageinfo.used_uniform_groups.insert(it->second);
					}
				}
				break;

			case KSL_STORAGE_BUFFER:
				m_globalusageinfo.used_buffers.insert(v_id);
				break;

			case KSL_STORAGE_SHARED:
				m_globalusageinfo.used_shared.insert(v_id);
				break;

			case KSL_STORAGE_DEFAULT:
			{
				if (v.type.GetTypeClass() == KSL_TYPECLASS_IMAGE)
				{
					m_globalusageinfo.used_images.insert(v_id);
				}

				std::set<uint32_t>::iterator it = m_inbuilt_variable_ids.find(ven->variable_id);
				if (it != m_inbuilt_variable_ids.end())
				{
					m_globalusageinfo.used_inbuilts.insert(v_id);
				}
			}

			default:
				break;
			}

			return false;
		}
		return true;
	}

	KSLProgramAST *m_ast;
	std::set<std::string> *m_user_defined_functions;
	std::set<uint32_t> m_inbuilt_variable_ids;
};


#if 0

void DumpStringSet(const std::set<std::string> &string_set)
{
	for (auto it = string_set.begin(); it != string_set.end(); it++)
	{
		printf("%s, ", it->c_str());
	}
}

void DumpVariableNames(const KSLProgramAST *ast, std::set<uint32_t> vars)
{
	for (auto it = vars.begin(); it != vars.end(); it++)
	{
		printf("%s, ", ast->variables[*it].orig_name.c_str());
	}
}

void DumpGlobalUsage(const KSLProgramAST *ast, const std::map<std::string, KSLGlobalUsageInfo> &global_usage)
{
	printf("\n\n");

	for (auto it = global_usage.begin(); it != global_usage.end(); it++)
	{
		printf("%s:\n", it->first.c_str());

		printf("uniform:          %d\n", it->second.use_uniforms);
		printf("in attribs:       %d\n", it->second.use_in_attribs);
		printf("out attribs:      %d\n", it->second.use_out_attribs);

		printf("used samplers:    ");
		DumpVariableNames(ast, it->second.used_samplers);
		printf("\nused buffers:    ");
		DumpVariableNames(ast, it->second.used_buffers);
		printf("\nused images:     ");
		DumpVariableNames(ast, it->second.used_images);
		printf("\nused shared:     ");
		DumpVariableNames(ast, it->second.used_shared);

		printf("\n");

		printf("called functions: ");
		DumpStringSet(it->second.called_functions);

		printf("\n\n");
	}

	printf("\n");
}

#else
#define DumpGlobalUsage(...) ;
#endif


bool MergeUsage(bool &dst, bool &src)
{
	if (src && !dst)
	{
		dst = true;
		return true;
	}
	return false;
}


template <typename T>
bool MergeUsageSet(std::set<T> &dst, std::set<T> &src)
{
	bool s = false;
	typename std::set<T>::iterator src_it = src.begin();
	for (; src_it != src.end(); src_it++)
	{
		if (dst.find(*src_it) == dst.end())
		{
			dst.insert(*src_it);
			s = true;
		}
	}
	return s;
}


bool KSLTranslator::CreateGlobalUsageInfo()
{
	std::map<std::string, uint32_t> variable_group;
	for (size_t i = 0; i < m_uniform_info->size(); i++)
	{
		variable_group[m_uniform_info->at(i).m_name] = m_uniform_info->at(i).m_group;
	}

	std::set<std::string> user_defined_functions;

	// collect user defined functions
	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];

		if (n->node_type == KSL_NODE_FUNCTION)
		{
			KSLFunctionNode* fn = dynamic_cast<KSLFunctionNode*>(n);
			KSLFunction &f = ast->functions[fn->function_id];

			user_defined_functions.insert(f.function_name);
		}
	}

	// get global variable and function usage per function
	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];

		if (n->node_type == KSL_NODE_FUNCTION)
		{
			KSLFunctionNode* fn = dynamic_cast<KSLFunctionNode*>(n);
			KSLFunction &f = ast->functions[fn->function_id];

			if (f.function_name == "main") continue;

			KSLCollectGlobalUsageVisitor cguv(ast, &user_defined_functions);
			cguv.m_variable_group = &variable_group;

			KSL::Traverse(n, cguv);

			m_global_usage[f.function_name] = cguv.m_globalusageinfo;
		}
	}

	//DumpGlobalUsage(ast, m_global_usage);

	bool changed = true;
	while (changed)
	{
		changed = false;

		std::map<std::string, KSLGlobalUsageInfo>::iterator guit = m_global_usage.begin();
		for (; guit != m_global_usage.end(); guit++)
		{
			KSLGlobalUsageInfo &gui = guit->second;

			std::set<std::string>::iterator cfit = gui.called_functions.begin();
			for (; cfit != gui.called_functions.end(); cfit++)
			{
				KSLGlobalUsageInfo &cfgui = m_global_usage[*cfit];

				changed |= MergeUsage(gui.use_in_attribs, cfgui.use_in_attribs);
				changed |= MergeUsage(gui.use_out_attribs, cfgui.use_out_attribs);

				changed |= MergeUsageSet(gui.used_buffers, cfgui.used_buffers);
				changed |= MergeUsageSet(gui.used_samplers, cfgui.used_samplers);
				changed |= MergeUsageSet(gui.used_shared, cfgui.used_shared);
				changed |= MergeUsageSet(gui.used_images, cfgui.used_images);
				changed |= MergeUsageSet(gui.used_inbuilts, cfgui.used_inbuilts);
				changed |= MergeUsageSet(gui.used_uniform_groups, cfgui.used_uniform_groups);

				changed |= MergeUsageSet(gui.called_functions, cfgui.called_functions);
			}
		}
	}

	//DumpGlobalUsage(ast, m_global_usage);

	return true;
}


void KSLTranslator::ErrorUniformInfoNotSet()
{
	std::stringstream sstream;
	sstream << "Uniform info not set";
	m_errors.push_back(KSLError(KSL_TRANSLATOR, KSL_ERROR, 0, 0, sstream.str()));
}


void KSLTranslator::ErrorUniformInfoNotSetFor(const std::string &variable_name)
{
	std::stringstream sstream;
	sstream << "Uniform info not set for: " << variable_name;
	m_errors.push_back(KSLError(KSL_TRANSLATOR, KSL_ERROR, 0, 0, sstream.str()));
}

