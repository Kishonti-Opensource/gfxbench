/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_compiler.h"

#include "glslpp/glslpp.h"

#include <sstream>
#include <assert.h>
#include <algorithm>

// GL
#if defined(ENABLE_GL_GENERATOR) || defined(ENABLE_VULKAN_GENERATOR)
#include "gl/ksl_gl_generator.h"
#include "gl/ksl_gl_translator.h"
#endif

// Vulkan
#if defined(ENABLE_VULKAN_GENERATOR)
#include "vulkan/ksl_vk_generator.h"
#include "vulkan/ksl_vk_translator.h"
#endif

// Metal
#if defined(ENABLE_METAL_GENERATOR)
#include "metal/ksl_mtl_translator.h"
#include "metal/ksl_mtl_generator.h"
#endif

// Direct3D
#if defined(ENABLE_D3D_GENERATOR)
#include "d3d/ksl_d3d_translator.h"
#include "d3d/ksl_d3d_generator.h"
#endif


KSLCompiler::KSLCompiler()
: m_translator(NULL)
, m_generator(NULL)
, m_uniform_info(NULL)
, m_use_subpass(true)
, m_treat_warnings_as_error(false)
, m_debug_generator(NULL)
{
	Clear();
}


KSLCompiler::~KSLCompiler()
{
	Clear();
}


bool KSLCompiler::Analyze()
{
	bool s = true;

	// Add KSL defines
	std::stringstream sstream;

	sstream << "#define KSL_COMPILER 1" << std::endl;

	KSLApiFamily api_family = GetAPIFamily(m_target_api);
	switch (api_family)
	{
	case KSL_API_FAMILY_METAL:
		sstream << "#define KSL_TARGET_LANGUAGE_METAL 1" << std::endl;
		break;
	case KSL_API_FAMILY_GL:
		sstream << "#define KSL_TARGET_LANGUAGE_GLSL 1" << std::endl;
		break;
	case KSL_API_FAMILY_D3D:
		sstream << "#define KSL_TARGET_LANGUAGE_D3D 1" << std::endl;
		break;
	default: assert(0);
	}
	sstream << m_source;

	// Preprocessing
	{
		m_preprocessed_source = sstream.str();
		
		std::string pp_log;
		s = PreProcessShaderCode(m_preprocessed_source, pp_log);

		if (!s)
		{
			std::stringstream sstream;
			sstream << "Preprocessing failed!\n";
			sstream << pp_log;

			m_errors.push_back(KSLError(KSL_PREPROCESSOR, KSL_ERROR, 0, 0, sstream.str()));
			return false;
		}
	}


	// Tokenize
	{
		m_tokenizer.m_source = m_preprocessed_source;
		s = m_tokenizer.Tokenize();

		std::vector<KSLError> &te = m_tokenizer.m_errors;
		m_errors.insert(m_errors.end(), te.begin(), te.end());
		if (!s)
		{
			return false;
		}
	}


	// Analyze
	m_ast.shader_type = m_shader_type;
	{
		m_analyzer.m_tokens = &m_tokenizer.m_tokens;
		m_analyzer.m_shader_type = m_shader_type;
        m_analyzer.m_precision_mismatch_severity = (api_family == KSL_API_FAMILY_METAL)?KSL_ERROR:KSL_WARNING;
		s = m_analyzer.Analyze(m_ast);

		std::vector<KSLError> &ae = m_analyzer.GetErrors();
		m_errors.insert(m_errors.end(), ae.begin(), ae.end());
		if (!s)
		{
			return false;
		}
	}

	if (HasErrors())
	{
		return false;
	}


	// Debug generator
	if (m_generate_debug_ksl)
	{
		m_debug_generator = new KSLGenerator();

		m_debug_generator->ast = &m_ast;
		s = m_debug_generator->Generate();

		std::vector<KSLError> &ge = m_debug_generator->m_errors;
		m_errors.insert(m_errors.end(), ge.begin(), ge.end());

		if (!s)
		{
			return false;
		}
	}


	// Init translator
	{
		m_translator = NULL;
		switch (api_family)
		{

		case KSL_API_FAMILY_GL:
			if (m_target_api == NGL_VULKAN)
			{
#if defined(ENABLE_VULKAN_GENERATOR)
				KSLVKTranslator* vk_translator = new KSLVKTranslator(m_use_subpass);
				m_translator = vk_translator;
#endif
			}
			else
			{
#if defined(ENABLE_GL_GENERATOR)
				m_translator = new KSLGLTranslator();
#endif
			}
			break;

		case KSL_API_FAMILY_D3D:
#if defined(ENABLE_D3D_GENERATOR)
			{
				KSLD3DTranslator* d3d_translator = new KSLD3DTranslator();
				m_translator = d3d_translator;
			}
#endif
			break;
			
		case KSL_API_FAMILY_METAL:
#if defined(ENABLE_METAL_GENERATOR)
			m_translator = new KSLMetalTranslator(m_use_subpass);
#endif
			break;

		default:
			break;
		}

		if (m_translator == NULL)
		{
			ErrorApiNotSupported();
			return false;
		}

		m_translator->ast = &m_ast;
		m_translator->m_shader_type = m_shader_type;
		m_translator->m_uniform_info = m_uniform_info;
	}

	return s;
}


bool KSLCompiler::Generate()
{
	bool s = true;
	KSLApiFamily api_family = GetAPIFamily(m_target_api);

	// Translate
	{
		bool s = m_translator->Translate();

		std::vector<KSLError> &te = m_translator->m_errors;
		m_errors.insert(m_errors.end(), te.begin(), te.end());

		if (!s)
		{
			return false;
		}
	}


	// Generate
	{
		m_generator = NULL;
		switch (api_family)
		{

		case KSL_API_FAMILY_GL:
			if (m_target_api == NGL_VULKAN)
			{
#if defined(ENABLE_VULKAN_GENERATOR)
				m_generator = new KSLVKGenerator(m_use_subpass);
#endif
			}
			else
			{
#if defined(ENABLE_GL_GENERATOR)
				m_generator = new KSLGLGenerator();
#endif
			}
			break;

		case KSL_API_FAMILY_D3D:
#if defined(ENABLE_D3D_GENERATOR)
			m_generator = new KSLD3DGenerator();
			break;
#endif

		case KSL_API_FAMILY_METAL:
#if defined(ENABLE_METAL_GENERATOR)
			m_generator = new KSLMetalGenerator(m_use_subpass, m_target_api);
			break;
#endif

		default:
			break;
		}

		if (m_generator == NULL)
		{
			ErrorApiNotSupported();
			return false;
		}

		m_generator->ast = &m_ast;
		m_generator->SetTranslator(m_translator);
		s = m_generator->Generate();

		std::vector<KSLError> &ge = m_generator->m_errors;
		m_errors.insert(m_errors.end(), ge.begin(), ge.end());

		if (!s)
		{
			return false;
		}
	}

	if (HasErrors())
	{
		return false;
	}

	return s;
}


bool KSLCompiler::Compile()
{
	bool s = true;

	if (s) s &= Analyze();

	if (s) s &= Generate();

	return s;
}


bool KSLCompiler::ProcessStageInfo(std::vector<KSLCompiler*> stage_compilers)
{
	std::vector<KSLTranslator*> translators;
	translators.resize(stage_compilers.size(), NULL);

	NGL_api target_api = NGL_NULL_DRIVER;
	for (size_t i = 0; i < stage_compilers.size(); i++)
	{
		if (stage_compilers[i] == NULL) continue;
		translators[i] = stage_compilers[i]->m_translator;
		target_api = stage_compilers[i]->m_target_api;
	}

	KSLApiFamily api_family = GetAPIFamily(target_api);

	switch (api_family)
	{
		case KSL_API_FAMILY_GL:
			if (target_api == NGL_VULKAN)
			{
#if defined(ENABLE_VULKAN_GENERATOR)
				KSLVKTranslator::ProcessStageInfo(translators);
#endif
			}
			break;

		case KSL_API_FAMILY_METAL:
		case KSL_API_FAMILY_D3D:
			break;

		default:
			assert(0);
			break;
	}

	return true;
}


void KSLCompiler::SetSource(const std::string & source)
{
	m_source = source;
}


void KSLCompiler::SetShaderType(NGL_shader_type shader_type)
{
	m_shader_type = shader_type;
}


void KSLCompiler::SetTargetApi(NGL_api target_api)
{
	m_target_api = target_api;
}


void KSLCompiler::SetTreatWarningAsErrors(bool b)
{
	m_treat_warnings_as_error = b;
}


void KSLCompiler::SetUseSubpass(bool b)
{
	m_use_subpass = b;
}


void KSLCompiler::SetVulkanPushConstantCount(uint32_t count)
{
#if defined(ENABLE_VULKAN_GENERATOR)
	KSLVKTranslator::m_pust_constant_count = count;
#endif
}


bool KSLCompiler::CreateReflection(NGL_shader_source_descriptor &ssd)
{
	return m_analyzer.CreateReflection(ssd);
}


std::string KSLCompiler::GetResult()
{
	return m_generator->GetResult();
}


bool KSLCompiler::HasErrors()
{
	if (m_treat_warnings_as_error)
	{
		return m_errors.size() > 0;
	}

	for (size_t i = 0; i < m_errors.size(); i++)
	{
		if (m_errors[i].severity == KSL_ERROR)
		{
			return true;
		}
	}

	return false;
}


std::vector<KSLError> & KSLCompiler::GetErrors() 
{
	return m_errors;
}


void KSLCompiler::SetGenerateDebugKSL(bool generate_debug_ksl)
{
	m_generate_debug_ksl = generate_debug_ksl;
}


void KSLCompiler::SetShaderUniformInfo(const std::vector<NGL_shader_uniform> &uniform_info)
{
	m_uniform_info = &uniform_info;
}


std::string & KSLCompiler::GetPreprocessedSource()
{
	return m_preprocessed_source;
}


std::string KSLCompiler::GetTokenizerDebug()
{
	return m_tokenizer.DebugDump();
}


std::string KSLCompiler::GetDebugKSL()
{
	return m_debug_generator->GetResult();
}


void KSLCompiler::Clear()
{
	m_generate_debug_ksl = false;

	delete m_translator;
	m_translator = NULL;

	delete m_generator;
	m_generator = NULL;

	delete m_debug_generator;
	m_debug_generator = NULL;

	m_errors.clear();
	m_tokenizer.Clear();
}


std::string KSLCompiler::GetLog()
{
	m_log.clear();

	// make source lines
	std::vector<std::string> source_lines;
	{
		std::stringstream sstream(m_source);
		std::string line;
		while (std::getline(sstream, line))
		{
			source_lines.push_back(line);
		}
	}

	m_log << "KSL compiler log" << std::endl << std::endl;
	m_log << "compile status: " << ((HasErrors()) ? "FAILED" : "SUCCESS") << std::endl << std::endl;

	m_log << "Source:"<<std::endl;
	for (size_t i = 0; i < source_lines.size(); i++)
	{
		m_log << i + 3 << ": " << source_lines[i] << std::endl;
	}
	m_log << std::endl;

	std::stable_sort(m_errors.begin(), m_errors.end(), KSLErrorByLineCompare());

	for (size_t i = 0; i < m_errors.size(); i++)
	{
		KSLError e = m_errors[i];
		m_log << ((e.severity == KSL_ERROR) ? "[ERR ] " : "[WARN] ") << e.message << std::endl;
	}
	m_log << std::endl;

	return m_log.str();
}


void KSLCompiler::ErrorApiNotSupported()
{
	std::stringstream sstream;
	sstream << "Unsupported target api" << std::endl;
	m_errors.push_back(KSLError(KSL_GENERATOR, KSL_ERROR, 0, 0, sstream.str()));
}

