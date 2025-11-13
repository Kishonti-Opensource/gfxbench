/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_COMPILER__
#define __KSL_COMPILER__

#include "ngl.h"
#include "ksl_tokenizer.h"
#include "ksl_common.h"
#include "ksl_analyzer.h"
#include "ksl_translator.h"
#include "ksl_generator.h"

#include <string>


class KSLCompiler
{
public:
	KSLCompiler();
	virtual ~KSLCompiler();

	void SetSource(const std::string & source);
	void SetShaderType(NGL_shader_type shader_type);
	void SetTargetApi(NGL_api target_api);
	void SetShaderUniformInfo(const std::vector<NGL_shader_uniform> &uniform_info);
	void SetTreatWarningAsErrors(bool b);
	void SetUseSubpass(bool b);

	bool Analyze();
	bool Generate();
	static bool ProcessStageInfo(std::vector<KSLCompiler*> stage_compilers);
	static void SetVulkanPushConstantCount(uint32_t count);

	bool Compile();

	std::string GetResult();
	bool CreateReflection(NGL_shader_source_descriptor &ssd);

	void Clear();
	
	// Errors
	bool HasErrors();
	std::string GetLog();
	std::vector<KSLError> & GetErrors();

	// Debug
	void SetGenerateDebugKSL(bool generate_debug_ksl);

	std::string & GetPreprocessedSource();
	std::string GetTokenizerDebug();
	std::string GetDebugKSL();

private:
	std::string m_source;
	NGL_shader_type m_shader_type;
	NGL_api m_target_api;

	std::string m_preprocessed_source;
	KSLProgramAST m_ast;

	KSLTokenizer m_tokenizer;
	KSLAnalyzer m_analyzer;
	KSLTranslator* m_translator;
	KSLGenerator* m_generator;

	const std::vector<NGL_shader_uniform> *m_uniform_info;
	bool m_use_subpass;

	// Error
	bool m_treat_warnings_as_error;
	std::stringstream m_log;
	std::vector<KSLError> m_errors;

	void ErrorApiNotSupported();

	// Debug
	bool m_generate_debug_ksl;
	KSLGenerator* m_debug_generator;
};


#endif //__KSL_COMPILER__

