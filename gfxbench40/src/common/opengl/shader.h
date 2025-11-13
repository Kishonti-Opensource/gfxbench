/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SHADER_H
#define SHADER_H

#define UBO31


#include "kcl_base.h"
//#include "test_error.h"

#include <string>
#include <vector>
#include <set>
#include "render_statistics_defines.h"

#include "glb_shader_common.h"

//#define LOG_SHADERS

#ifdef LOG_SHADERS

//#define WRITE_LOG_SHADERS // to write out shaders + empty info files
#define READ_LOG_SHADERS


#ifdef READ_LOG_SHADERS

#define PER_SHADER_ALU_INFO

#define PER_FRAME_ALU_INFO

#define SAVE_ALU_INFO_EXCEL

#endif


namespace LOGSHADER
{
	class ShaderInfo;
	class FragmentShaderInfo;

	#ifdef PER_FRAME_ALU_INFO
	struct FrameAluInfo;
	#endif
}

#endif


class Shader : public GLB::GLBShaderCommon
{

public:
	static const KCL::uint32 UNIFORM_BLOCK_COUNT = 14;

	static std::set< std::string> m_defines_debug;

	struct sUBOnames
	{
		enum Enum
		{
			Frame = 0,
			Camera,
			Mat,
			Mesh,
			StaticMesh,
			Light,
			EmitterAdvect,
			EmitterRender,
			LightShaftConsts,
			LightLensFlareConsts,
			FilterConsts,
			TranslateUVConsts,
			EnvmapsInterpolatorConsts,
			LightInstancingConsts
		};
	};

	struct ShaderTypes
	{
		enum Enum
		{
			Default,
			TransformFeedback
		};
	};

	static Shader* m_last_shader;

	static void FillUniformBlockData(KCL::uint32 idx);
	static Shader* CreateShader( const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error, ShaderTypes::Enum shaderType = ShaderTypes::Default);
    
	static void DeleteShader(Shader* s);
	static void DeleteShaders();
	static void SetForceHIGHP(bool force_highp);
	static void InitShaders( const std::string &requiredRenderAPI, bool force_fs_mediump);

	static bool m_ubo_support_enabled;
	bool m_has_uniform_block[UNIFORM_BLOCK_COUNT];

	#ifdef LOG_SHADERS
	
	LOGSHADER::ShaderInfo *VertexShaderInfo;
	LOGSHADER::FragmentShaderInfo *FragmentShaderInfo;

	#ifdef PER_FRAME_ALU_INFO
	static void Push_New_FrameAluInfo();
	static void Add_FrameAluInfo(Shader* shader, KCL::uint32 samplesPassed, KCL::uint32 triangleCount);

private:
	static KCL::uint32 s_queryId;
	static std::vector<LOGSHADER::FrameAluInfo*> FrameAluInfo;
	static bool s_queryCreated;
public:
	static KCL::uint32 QueryId() { return s_queryId; }
	static void CreateQuery();
	static void DeleteQuery();

	static void SaveShaderAluInfo_Excel();
	static void SaveFrameAluInfo_Excel();

	#endif
	
	#endif


private:

	KCL::uint32 m_adler;
	static std::vector<Shader*> m_shader_cache;

	Shader();
	virtual ~Shader();
// 	static KCL::KCL_Status SaveBinaryForCaching(KCL::uint32 program);
// 	static KCL::KCL_Status LoadBinaryFromCache(KCL::uint32 program);

	///for vertexshaders
	//static void PutPrecisionQualifierToUniforms( std::string &vs_source, std::string &fs_source, PRECISION precision);

	static void AddPrecisionQualifierToUniforms( std::string &vs_source, std::string &fs_source);
	static void AddMediumPrecisionQualifierToVaryings( std::string &vs_source);
	static void AddHighPrecisionQualifierToVaryings( std::string &vs_source);
	static void AddUniformBlocks( std::string &source);

	static std::string m_shader_dir;
	static std::string m_shader_header;

	static const char *m_uniform_block_names[UNIFORM_BLOCK_COUNT];
	static std::string m_uniform_block_data[UNIFORM_BLOCK_COUNT];
};


#endif
