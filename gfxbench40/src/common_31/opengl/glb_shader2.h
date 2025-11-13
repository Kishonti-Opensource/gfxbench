/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SHADER2_H
#define SHADER2_H

#include <kcl_base.h>
#include <kcl_scene_version.h>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "opengl/glb_shader_common.h"
#include "opengl/glb_opengl_limits.h"

namespace GLB
{

class GLBShader2 : public GLBShaderCommon
{
	friend class GLBShaderBuilder;
public:

	struct ShaderTypes
	{
		enum Enum
		{
			VertexShader = 1,
			FragmentShader = 2,
			TransformFeedback = 4,
			ComputeShader = 8,
			TessControlShader = 16,
			TessEvaluationShader = 32,
			GeometryShader = 64
		};
	};

	KCL::uint32 GetShaderType() const;
	KCL::uint32 HasShader(ShaderTypes::Enum shaderType) const;
	bool IsComputeShader() const;
    
	// Static public functions
	static void InitShaders(KCL::SceneVersion scene_version, bool force_highp);
	static void DeleteShaders();
    static void InvalidateShaderCache();

	static std::string GLSLTextureFormat(KCL::uint32 textureFormat) ;   
	
private:
	GLBShader2();
	virtual ~GLBShader2();

    KCL::uint32 m_vs_shader;
	KCL::uint32 m_fs_shader;
	KCL::uint32 m_cs_shader;
	KCL::uint32 m_tcs_shader;
	KCL::uint32 m_tes_shader;
	KCL::uint32 m_gs_shader;

    KCL::uint32 m_shader_type;
	KCL::uint32 m_checksum;

	// Static shader cache
    struct CachedShader
    {
        GLBShader2 *m_shader;       
        bool m_dirty;
        CachedShader()
        {
            m_shader = NULL;
            m_dirty = false;
        }
    };
	static GLBShader2 * LookupShader(KCL::uint32 checksum);
	static void CacheShader(GLBShader2 *shader, KCL::uint32 checksum);
	static std::map<KCL::uint32, CachedShader> m_shader_cache;
    static std::vector<GLBShader2*> m_invalid_shaders;
};

class GLBShaderBuilder
{
	friend class GLBShader2;  
public:    
    enum ShaderIndices
	{
		VERTEX_INDEX = 0,
		FRAGMENT_INDEX = 1,
		COMPUTE_INDEX = 2,
		TESS_CONTROL_INDEX = 3,
		TESS_EVALUATION_INDEX = 4,
		GEOMETRY_INDEX = 5,

		INDEX_COUNT = 6,
	};

    struct BuilderDescriptor
    {
        std::vector<std::string> m_global_shader_dirs;
	    ContextLimits * m_target_limits;

        std::string m_common_headers;
        std::string m_stage_headers[INDEX_COUNT];
        BuilderDescriptor();
    };

	GLBShaderBuilder();
	~GLBShaderBuilder();

	// Add custom shader shader directory
	GLBShaderBuilder &AddShaderDir(const std::string &dir);

	// Add some custom defines to the code
	GLBShaderBuilder &AddDefines(const std::set<std::string> *defines);
    GLBShaderBuilder &AddDefine(const std::string &define);
	GLBShaderBuilder &AddDefineInt(const std::string &name, int value);
    GLBShaderBuilder &AddDefineFloat(const std::string &name, float value);
    GLBShaderBuilder &AddDefineVec2(const std::string &name, const KCL::Vector2D &value);
    GLBShaderBuilder &AddDefineVec3(const std::string &name, const KCL::Vector3D &value);
    GLBShaderBuilder &AddDefineVec4(const std::string &name, const KCL::Vector4D &value);

    GLBShaderBuilder &AddDefineIVec2(const std::string &name, KCL::int32 x, KCL::int32 y);

	// Load shader from .shader file
	GLBShaderBuilder &ShaderFile(const char *file);

	// Load shaders from separete files
	GLBShaderBuilder &File(GLBShader2::ShaderTypes::Enum type, const char *file);
	GLBShaderBuilder &FileVs(const char *file);
	GLBShaderBuilder &FileFs(const char *file);
	GLBShaderBuilder &FileCs(const char *file);

	// Create shader from source
	GLBShaderBuilder &Source(GLBShader2::ShaderTypes::Enum type, const char * source);
	GLBShaderBuilder &SourceVs(const char *source);
	GLBShaderBuilder &SourceFs(const char *source);
	GLBShaderBuilder &SourceCs(const char *source);

	// Set the glTransformFeedbackVaryings
	GLBShaderBuilder &TransformFeedback(bool value);

	// Create the shader
	GLBShader2 *Build(KCL::KCL_Status& error);

	// Reset the builder
	void Clear();

    static void AddGlobalDefine(const std::string &name, int value);
    static void RemoveGlobalDefine(const std::string &name);
private:    
	GLBShader2 *CreateShader(KCL::KCL_Status &error);

    KCL::KCL_Status LoadFiles();
	KCL::KCL_Status LoadFile(const std::string &file, std::string &dst, bool isHeader);
	KCL::KCL_Status ParseSources();
    KCL::KCL_Status ParseSource(std::string &source, std::set<std::string> &included_files);

    std::string GetSourceList();

    KCL::uint32 m_checksum;
    GLBShader2 *LookupShader();
    static KCL::uint32 CalculateChecksum(KCL::uint32 current, const char *str, KCL::uint32 len);
	static KCL::uint32 CalculateChecksum(KCL::uint32 current, const std::vector<std::string> &sources);
	static KCL::uint32 GetShaderTypeIndex(GLBShader2::ShaderTypes::Enum type);

	std::vector<std::string> m_shader_dirs;

	std::vector<std::string> m_sources[INDEX_COUNT];
	std::vector<std::string> m_files[INDEX_COUNT];

	std::string m_shader_file;
	std::string m_shader_source;

	bool m_transform_feedback;

	std::set<std::string> m_defines_set;
       
    static BuilderDescriptor m_build_descriptor;
};

}

#endif
