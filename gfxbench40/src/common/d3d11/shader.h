/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SHADER2_H
#define SHADER2_H


#include <string>
#include <vector>
#include <set>
#include <map>

#include <kcl_base.h>
#include <d3d11_1.h>
#include "d3d11/DXUtils.h"
#include "d3d11/DX.h"


struct VertexShaderStruct
{
	std::string m_name;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_shader;
	std::vector<byte> m_bytes;
};
struct PixelShaderStruct
{
	std::string m_name;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_shader;
};

class Shader
{
public:
	VertexShaderStruct m_vs;
	PixelShaderStruct m_ps;
	static Shader* m_default_shader; //used to avoid crashes after recompile errors

	struct ShaderTypes
	{
		enum Enum
		{
			Default,
			TransformFeedback
		};
	};

	void Bind();

	//TODO:Remove
	KCL::uint32 m_attrib_locations[64];
	KCL::uint32 m_uniform_locations[64];
	static void InitShaders(const std::string &requiredRenderAPI, bool force_fs_mediump);

	static Shader* CreateShader( const char* vsfile, const char* psfile, const std::set<std::string> *defines, KCL::KCL_Status& error, ShaderTypes::Enum shaderType = ShaderTypes::Default);
    static Shader* CreateShader( const char* file, const std::set<std::string> *defines, KCL::KCL_Status& error) { return NULL; } //TODO for GFXBench4, krl_material.cpp/InitShaders4s
	static void DeleteShaders();
	static std::string getShaderPrefix();
	static void setShaderPrefix(std::string prefix);
private:
	static Shader* CreateShader( const char* vsfile, const char* psfile, KCL::KCL_Status& error);
	static std::map<std::string, VertexShaderStruct> m_vs_cache;
	static std::map<std::string, PixelShaderStruct> m_ps_cache;
	static std::map<std::string, Shader*> m_shader_cache;
	static std::string m_shader_prefix;
	static void LoadVertexShader(std::string filename, VertexShaderStruct &shader);
	static void LoadPixelShader(std::string filename, PixelShaderStruct &shader);


	Shader();
	~Shader();
};


#endif
