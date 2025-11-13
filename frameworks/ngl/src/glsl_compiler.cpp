/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glslang/Public/ShaderLang.h"
#include "OGLCompilersDLL/InitializeDll.h"
#include "SPIRV/GlslangToSpv.h"
#include "SPIRV/GLSL.std.450.h"
#include "SPIRV/doc.h"
#include "SPIRV/disassemble.h"
#include <string>
#include <vector>
#include "ngl.h"
#include "glsl_compiler.h"


void AddReflection(const char *parent_block, const char* block, int type, int format, int size, int binding_or_offset_or_location, int stage, int set, void *user_data)
{
	_shader_reflection &r = *(_shader_reflection*)user_data;

	if (parent_block)
	{
		for (size_t i = 0; i < r.uniforms.size(); i++)
		{
			_shader_reflection::Block &mb = r.uniforms[i];

			if (mb.name == parent_block && mb.stage == stage && mb.set == set)
			{
				_shader_reflection::Block b;

				b.name = block;
				b.binding_or_offset_or_location = binding_or_offset_or_location;
				b.size = size;
				b.format = format;
				b.type = type;
				b.stage = stage;
				b.set = set;

				mb.blocks.push_back(b);
			}
		}
	}
	else
	{
		_shader_reflection::Block b;

		b.name = block;
		b.binding_or_offset_or_location = binding_or_offset_or_location;
		b.size = size;
		b.format = format;
		b.type = type;
		b.stage = stage;
		b.set = set;

		if (type == 0)
		{
			if (stage == 0)
			{
				r.attributes.push_back(b);
			}
		}
		else
		{
			r.uniforms.push_back(b);
		}
	}
}


void *TLSpool;

static void ProcessConfigFile(TBuiltInResource &Resources)
{
    char* config = 0;

    if (config == 0) 
	{
		const char* DefaultConfig =
			"MaxLights 32\n"
			"MaxClipPlanes 6\n"
			"MaxTextureUnits 32\n"
			"MaxTextureCoords 32\n"
			"MaxVertexAttribs 64\n"
			"MaxVertexUniformComponents 4096\n"
			"MaxVaryingFloats 64\n"
			"MaxVertexTextureImageUnits 32\n"
			"MaxCombinedTextureImageUnits 80\n"
			"MaxTextureImageUnits 32\n"
			"MaxFragmentUniformComponents 4096\n"
			"MaxDrawBuffers 32\n"
			"MaxVertexUniformVectors 128\n"
			"MaxVaryingVectors 8\n"
			"MaxFragmentUniformVectors 16\n"
			"MaxVertexOutputVectors 16\n"
			"MaxFragmentInputVectors 15\n"
			"MinProgramTexelOffset -8\n"
			"MaxProgramTexelOffset 7\n"
			"MaxClipDistances 8\n"
			"MaxComputeWorkGroupCountX 65535\n"
			"MaxComputeWorkGroupCountY 65535\n"
			"MaxComputeWorkGroupCountZ 65535\n"
			"MaxComputeWorkGroupSizeX 1024\n"
			"MaxComputeWorkGroupSizeY 1024\n"
			"MaxComputeWorkGroupSizeZ 64\n"
			"MaxComputeUniformComponents 1024\n"
			"MaxComputeTextureImageUnits 16\n"
			"MaxComputeImageUniforms 8\n"
			"MaxComputeAtomicCounters 8\n"
			"MaxComputeAtomicCounterBuffers 1\n"
			"MaxVaryingComponents 60\n"
			"MaxVertexOutputComponents 64\n"
			"MaxGeometryInputComponents 64\n"
			"MaxGeometryOutputComponents 128\n"
			"MaxFragmentInputComponents 128\n"
			"MaxImageUnits 8\n"
			"MaxCombinedImageUnitsAndFragmentOutputs 8\n"
			"MaxCombinedShaderOutputResources 8\n"
			"MaxImageSamples 0\n"
			"MaxVertexImageUniforms 0\n"
			"MaxTessControlImageUniforms 0\n"
			"MaxTessEvaluationImageUniforms 0\n"
			"MaxGeometryImageUniforms 0\n"
			"MaxFragmentImageUniforms 8\n"
			"MaxCombinedImageUniforms 8\n"
			"MaxGeometryTextureImageUnits 16\n"
			"MaxGeometryOutputVertices 256\n"
			"MaxGeometryTotalOutputComponents 1024\n"
			"MaxGeometryUniformComponents 1024\n"
			"MaxGeometryVaryingComponents 64\n"
			"MaxTessControlInputComponents 128\n"
			"MaxTessControlOutputComponents 128\n"
			"MaxTessControlTextureImageUnits 16\n"
			"MaxTessControlUniformComponents 1024\n"
			"MaxTessControlTotalOutputComponents 4096\n"
			"MaxTessEvaluationInputComponents 128\n"
			"MaxTessEvaluationOutputComponents 128\n"
			"MaxTessEvaluationTextureImageUnits 16\n"
			"MaxTessEvaluationUniformComponents 1024\n"
			"MaxTessPatchComponents 120\n"
			"MaxPatchVertices 32\n"
			"MaxTessGenLevel 64\n"
			"MaxViewports 16\n"
			"MaxVertexAtomicCounters 0\n"
			"MaxTessControlAtomicCounters 0\n"
			"MaxTessEvaluationAtomicCounters 0\n"
			"MaxGeometryAtomicCounters 0\n"
			"MaxFragmentAtomicCounters 8\n"
			"MaxCombinedAtomicCounters 8\n"
			"MaxAtomicCounterBindings 1\n"
			"MaxVertexAtomicCounterBuffers 0\n"
			"MaxTessControlAtomicCounterBuffers 0\n"
			"MaxTessEvaluationAtomicCounterBuffers 0\n"
			"MaxGeometryAtomicCounterBuffers 0\n"
			"MaxFragmentAtomicCounterBuffers 1\n"
			"MaxCombinedAtomicCounterBuffers 1\n"
			"MaxAtomicCounterBufferSize 16384\n"
			"MaxTransformFeedbackBuffers 4\n"
			"MaxTransformFeedbackInterleavedComponents 64\n"
			"MaxCullDistances 8\n"
			"MaxCombinedClipAndCullDistances 8\n"
			"MaxSamples 4\n"

			"nonInductiveForLoops 1\n"
			"whileLoops 1\n"
			"doWhileLoops 1\n"
			"generalUniformIndexing 1\n"
			"generalAttributeMatrixVectorIndexing 1\n"
			"generalVaryingIndexing 1\n"
			"generalSamplerIndexing 1\n"
			"generalVariableIndexing 1\n"
			"generalConstantMatrixVectorIndexing 1\n"
			;

		config = new char[strlen(DefaultConfig) + 1];
		strcpy(config, DefaultConfig);
	}

    const char* delims = " \t\n\r";
    const char* token = strtok(config, delims);
    while (token) {
        const char* valueStr = strtok(0, delims);
        if (valueStr == 0 || ! (valueStr[0] == '-' || (valueStr[0] >= '0' && valueStr[0] <= '9'))) {
            printf("Error: '%s' bad .conf file.  Each name must be followed by one number.\n", valueStr ? valueStr : "");
            return;
        }
        int value = atoi(valueStr);

        if (strcmp(token, "MaxLights") == 0)
            Resources.maxLights = value;
        else if (strcmp(token, "MaxClipPlanes") == 0)
            Resources.maxClipPlanes = value;
        else if (strcmp(token, "MaxTextureUnits") == 0)
            Resources.maxTextureUnits = value;
        else if (strcmp(token, "MaxTextureCoords") == 0)
            Resources.maxTextureCoords = value;
        else if (strcmp(token, "MaxVertexAttribs") == 0)
            Resources.maxVertexAttribs = value;
        else if (strcmp(token, "MaxVertexUniformComponents") == 0)
            Resources.maxVertexUniformComponents = value;
        else if (strcmp(token, "MaxVaryingFloats") == 0)
            Resources.maxVaryingFloats = value;
        else if (strcmp(token, "MaxVertexTextureImageUnits") == 0)
            Resources.maxVertexTextureImageUnits = value;
        else if (strcmp(token, "MaxCombinedTextureImageUnits") == 0)
            Resources.maxCombinedTextureImageUnits = value;
        else if (strcmp(token, "MaxTextureImageUnits") == 0)
            Resources.maxTextureImageUnits = value;
        else if (strcmp(token, "MaxFragmentUniformComponents") == 0)
            Resources.maxFragmentUniformComponents = value;
        else if (strcmp(token, "MaxDrawBuffers") == 0)
            Resources.maxDrawBuffers = value;
        else if (strcmp(token, "MaxVertexUniformVectors") == 0)
            Resources.maxVertexUniformVectors = value;
        else if (strcmp(token, "MaxVaryingVectors") == 0)
            Resources.maxVaryingVectors = value;
        else if (strcmp(token, "MaxFragmentUniformVectors") == 0)
            Resources.maxFragmentUniformVectors = value;
        else if (strcmp(token, "MaxVertexOutputVectors") == 0)
            Resources.maxVertexOutputVectors = value;
        else if (strcmp(token, "MaxFragmentInputVectors") == 0)
            Resources.maxFragmentInputVectors = value;
        else if (strcmp(token, "MinProgramTexelOffset") == 0)
            Resources.minProgramTexelOffset = value;
        else if (strcmp(token, "MaxProgramTexelOffset") == 0)
            Resources.maxProgramTexelOffset = value;
        else if (strcmp(token, "MaxClipDistances") == 0)
            Resources.maxClipDistances = value;
        else if (strcmp(token, "MaxComputeWorkGroupCountX") == 0)
            Resources.maxComputeWorkGroupCountX = value;
        else if (strcmp(token, "MaxComputeWorkGroupCountY") == 0)
            Resources.maxComputeWorkGroupCountY = value;
        else if (strcmp(token, "MaxComputeWorkGroupCountZ") == 0)
            Resources.maxComputeWorkGroupCountZ = value;
        else if (strcmp(token, "MaxComputeWorkGroupSizeX") == 0)
            Resources.maxComputeWorkGroupSizeX = value;
        else if (strcmp(token, "MaxComputeWorkGroupSizeY") == 0)
            Resources.maxComputeWorkGroupSizeY = value;
        else if (strcmp(token, "MaxComputeWorkGroupSizeZ") == 0)
            Resources.maxComputeWorkGroupSizeZ = value;
        else if (strcmp(token, "MaxComputeUniformComponents") == 0)
            Resources.maxComputeUniformComponents = value;
        else if (strcmp(token, "MaxComputeTextureImageUnits") == 0)
            Resources.maxComputeTextureImageUnits = value;
        else if (strcmp(token, "MaxComputeImageUniforms") == 0)
            Resources.maxComputeImageUniforms = value;
        else if (strcmp(token, "MaxComputeAtomicCounters") == 0)
            Resources.maxComputeAtomicCounters = value;
        else if (strcmp(token, "MaxComputeAtomicCounterBuffers") == 0)
            Resources.maxComputeAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxVaryingComponents") == 0)
            Resources.maxVaryingComponents = value;
        else if (strcmp(token, "MaxVertexOutputComponents") == 0)
            Resources.maxVertexOutputComponents = value;
        else if (strcmp(token, "MaxGeometryInputComponents") == 0)
            Resources.maxGeometryInputComponents = value;
        else if (strcmp(token, "MaxGeometryOutputComponents") == 0)
            Resources.maxGeometryOutputComponents = value;
        else if (strcmp(token, "MaxFragmentInputComponents") == 0)
            Resources.maxFragmentInputComponents = value;
        else if (strcmp(token, "MaxImageUnits") == 0)
            Resources.maxImageUnits = value;
        else if (strcmp(token, "MaxCombinedImageUnitsAndFragmentOutputs") == 0)
            Resources.maxCombinedImageUnitsAndFragmentOutputs = value;
        else if (strcmp(token, "MaxCombinedShaderOutputResources") == 0)
            Resources.maxCombinedShaderOutputResources = value;
        else if (strcmp(token, "MaxImageSamples") == 0)
            Resources.maxImageSamples = value;
        else if (strcmp(token, "MaxVertexImageUniforms") == 0)
            Resources.maxVertexImageUniforms = value;
        else if (strcmp(token, "MaxTessControlImageUniforms") == 0)
            Resources.maxTessControlImageUniforms = value;
        else if (strcmp(token, "MaxTessEvaluationImageUniforms") == 0)
            Resources.maxTessEvaluationImageUniforms = value;
        else if (strcmp(token, "MaxGeometryImageUniforms") == 0)
            Resources.maxGeometryImageUniforms = value;
        else if (strcmp(token, "MaxFragmentImageUniforms") == 0)
            Resources.maxFragmentImageUniforms = value;
        else if (strcmp(token, "MaxCombinedImageUniforms") == 0)
            Resources.maxCombinedImageUniforms = value;
        else if (strcmp(token, "MaxGeometryTextureImageUnits") == 0)
            Resources.maxGeometryTextureImageUnits = value;
        else if (strcmp(token, "MaxGeometryOutputVertices") == 0)
            Resources.maxGeometryOutputVertices = value;
        else if (strcmp(token, "MaxGeometryTotalOutputComponents") == 0)
            Resources.maxGeometryTotalOutputComponents = value;
        else if (strcmp(token, "MaxGeometryUniformComponents") == 0)
            Resources.maxGeometryUniformComponents = value;
        else if (strcmp(token, "MaxGeometryVaryingComponents") == 0)
            Resources.maxGeometryVaryingComponents = value;
        else if (strcmp(token, "MaxTessControlInputComponents") == 0)
            Resources.maxTessControlInputComponents = value;
        else if (strcmp(token, "MaxTessControlOutputComponents") == 0)
            Resources.maxTessControlOutputComponents = value;
        else if (strcmp(token, "MaxTessControlTextureImageUnits") == 0)
            Resources.maxTessControlTextureImageUnits = value;
        else if (strcmp(token, "MaxTessControlUniformComponents") == 0)
            Resources.maxTessControlUniformComponents = value;
        else if (strcmp(token, "MaxTessControlTotalOutputComponents") == 0)
            Resources.maxTessControlTotalOutputComponents = value;
        else if (strcmp(token, "MaxTessEvaluationInputComponents") == 0)
            Resources.maxTessEvaluationInputComponents = value;
        else if (strcmp(token, "MaxTessEvaluationOutputComponents") == 0)
            Resources.maxTessEvaluationOutputComponents = value;
        else if (strcmp(token, "MaxTessEvaluationTextureImageUnits") == 0)
            Resources.maxTessEvaluationTextureImageUnits = value;
        else if (strcmp(token, "MaxTessEvaluationUniformComponents") == 0)
            Resources.maxTessEvaluationUniformComponents = value;
        else if (strcmp(token, "MaxTessPatchComponents") == 0)
            Resources.maxTessPatchComponents = value;
        else if (strcmp(token, "MaxPatchVertices") == 0)
            Resources.maxPatchVertices = value;
        else if (strcmp(token, "MaxTessGenLevel") == 0)
            Resources.maxTessGenLevel = value;
        else if (strcmp(token, "MaxViewports") == 0)
            Resources.maxViewports = value;
        else if (strcmp(token, "MaxVertexAtomicCounters") == 0)
            Resources.maxVertexAtomicCounters = value;
        else if (strcmp(token, "MaxTessControlAtomicCounters") == 0)
            Resources.maxTessControlAtomicCounters = value;
        else if (strcmp(token, "MaxTessEvaluationAtomicCounters") == 0)
            Resources.maxTessEvaluationAtomicCounters = value;
        else if (strcmp(token, "MaxGeometryAtomicCounters") == 0)
            Resources.maxGeometryAtomicCounters = value;
        else if (strcmp(token, "MaxFragmentAtomicCounters") == 0)
            Resources.maxFragmentAtomicCounters = value;
        else if (strcmp(token, "MaxCombinedAtomicCounters") == 0)
            Resources.maxCombinedAtomicCounters = value;
        else if (strcmp(token, "MaxAtomicCounterBindings") == 0)
            Resources.maxAtomicCounterBindings = value;
        else if (strcmp(token, "MaxVertexAtomicCounterBuffers") == 0)
            Resources.maxVertexAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxTessControlAtomicCounterBuffers") == 0)
            Resources.maxTessControlAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxTessEvaluationAtomicCounterBuffers") == 0)
            Resources.maxTessEvaluationAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxGeometryAtomicCounterBuffers") == 0)
            Resources.maxGeometryAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxFragmentAtomicCounterBuffers") == 0)
            Resources.maxFragmentAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxCombinedAtomicCounterBuffers") == 0)
            Resources.maxCombinedAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxAtomicCounterBufferSize") == 0)
            Resources.maxAtomicCounterBufferSize = value;
        else if (strcmp(token, "MaxTransformFeedbackBuffers") == 0)
            Resources.maxTransformFeedbackBuffers = value;
        else if (strcmp(token, "MaxTransformFeedbackInterleavedComponents") == 0)
            Resources.maxTransformFeedbackInterleavedComponents = value;
        else if (strcmp(token, "MaxCullDistances") == 0)
            Resources.maxCullDistances = value;
        else if (strcmp(token, "MaxCombinedClipAndCullDistances") == 0)
            Resources.maxCombinedClipAndCullDistances = value;
        else if (strcmp(token, "MaxSamples") == 0)
            Resources.maxSamples = value;

        else if (strcmp(token, "nonInductiveForLoops") == 0)
            Resources.limits.nonInductiveForLoops = (value != 0);
        else if (strcmp(token, "whileLoops") == 0)
            Resources.limits.whileLoops = (value != 0);
        else if (strcmp(token, "doWhileLoops") == 0)
            Resources.limits.doWhileLoops = (value != 0);
        else if (strcmp(token, "generalUniformIndexing") == 0)
            Resources.limits.generalUniformIndexing = (value != 0);
        else if (strcmp(token, "generalAttributeMatrixVectorIndexing") == 0)
            Resources.limits.generalAttributeMatrixVectorIndexing = (value != 0);
        else if (strcmp(token, "generalVaryingIndexing") == 0)
            Resources.limits.generalVaryingIndexing = (value != 0);
        else if (strcmp(token, "generalSamplerIndexing") == 0)
            Resources.limits.generalSamplerIndexing = (value != 0);
        else if (strcmp(token, "generalVariableIndexing") == 0)
            Resources.limits.generalVariableIndexing = (value != 0);
        else if (strcmp(token, "generalConstantMatrixVectorIndexing") == 0)
            Resources.limits.generalConstantMatrixVectorIndexing = (value != 0);
        else
            printf("Warning: unrecognized limit (%s) in configuration file.\n", token);

        token = strtok(0, delims);
    }

	delete [] config;
}


//0 - Vertex, 1 - TessControl, 2 - TessEvaluation, 3 - Geometry, 4 - Fragment, 5 - Compute
void CompileGLSL(NGL_shader_source_descriptor ssds[6], _shader_reflection &reflection)
{
	bool CompileFailed = false;
	bool LinkFailed = false;
	TBuiltInResource Resources;
	bool EOptionDefaultDesktop = true;
	bool EOptionSuppressInfolog = true;
	bool want_link = true;
	bool EOptionDumpReflection = false;
	bool EOptionBil = true;

	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules); 
	std::vector<glslang::TShader*> shaders;

	ProcessConfigFile( Resources);

	glslang::InitializeProcess();

	NGL_shader_type ngl_shader_types[6] =
	{
		NGL_VERTEX_SHADER,
		NGL_TESS_CONTROL_SHADER,
		NGL_TESS_EVALUATION_SHADER,
		NGL_GEOMETRY_SHADER,
		NGL_FRAGMENT_SHADER,
		NGL_COMPUTE_SHADER
	};


	for (int m = 0; m < 6; m++)
	{
		EShLanguage glslang_shader = (EShLanguage)m;
		NGL_shader_type ngl_shader = ngl_shader_types[m];

		glslang::TShader *shader = 0;

		if (ssds[ngl_shader].m_source_data.length())
		{
			shader = new glslang::TShader(glslang_shader);
		}

		if( shader)
		{
			//ez a tomb elvileg csak compile time kell
			const char * shaderStrings =
			{
				ssds[ngl_shader].m_source_data.c_str()
			};

			shader->setStrings( &shaderStrings, 1);
			if (ssds[ngl_shader].m_entry_point.length())
			{
				shader->setEntryPoint(ssds[ngl_shader].m_entry_point.c_str());
			}
			else
			{
				shader->setEntryPoint("main");
			}
			if (! shader->parse(&Resources, EOptionDefaultDesktop ? 110 : 100, false, messages))
				CompileFailed = true;

			if (! EOptionSuppressInfolog)
			{
				puts(shader->getInfoLog());
				puts(shader->getInfoDebugLog());
			}

			shaders.push_back( shader);
		}
	}

	if( !shaders.size())
	{
		printf( "no shaders.\n");
	}
	else if( want_link)
	{
		//
		// Program-level processing...
		//	

		glslang::TProgram *program = new glslang::TProgram;

		for( size_t i=0; i<shaders.size(); i++)
		{
			program->addShader( shaders[i]);
		}

		if (! program->link(messages))
			LinkFailed = true;

		if (! EOptionSuppressInfolog) 
		{
			puts(program->getInfoLog());
			puts(program->getInfoDebugLog());
		}

		if (EOptionDumpReflection) 
		{
			program->buildReflection();
			program->dumpReflection();
		}
			
		if (EOptionBil) {
			if (CompileFailed || LinkFailed)
			{
				for (int m = 0; m < 6; m++)
				{
					printf("shader begin:\n");
					if (ssds[m].m_source_data.length())
					{
						printf("%s", ssds[m].m_source_data.c_str());
					}
					printf("shader end:\n");
				}

				printf("Bil is not generated for failed compile or link\n");
			}
			else
			{
				for (int stage = 0; stage < EShLangCount; ++stage) 
				{
					EShLanguage glslang_shader = (EShLanguage)stage;
					NGL_shader_type ngl_shader = ngl_shader_types[stage];

					if (program->getIntermediate(glslang_shader))
					{
						std::vector<unsigned int> binary;

						glslang::GlslangToSpv(*program->getIntermediate(glslang_shader), binary, AddReflection, &reflection);
						
						size_t binary_size = binary.size() * 4;

						ssds[ngl_shader].m_binary_data.resize(binary_size);

						memcpy(&ssds[ngl_shader].m_binary_data[0], &binary[0], binary_size);

						//glslang::OutputSpv(shader_binaries[stage], "compute");
					}
				}
			}
		}

		delete program;
	}

    // Free everything up, program has to go before the shaders
    // because it might have merged stuff from the shaders, and
    // the stuff from the shaders has to have its destructors called
    // before the pools holding the memory in the shaders is freed.
    while (shaders.size() > 0) 
	{
        delete shaders.back();
        shaders.pop_back();
    }

	glslang::FinalizeProcess();
	
	//glslang::DetachThread();
}
