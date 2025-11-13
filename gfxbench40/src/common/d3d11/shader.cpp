/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#include <stdio.h>
#include <ctype.h>


#include "shader.h"
#include <kcl_os.h>
#include <kcl_io.h>
#include "zlib.h"
#include "platform.h"
#include "d3d11/IOUtils.h"

#include <wrl/client.h>
#include <ppl.h>
#include <ppltasks.h>

#if defined _DEBUG && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include <d3dcompiler.h>
#endif

extern KCL::OS *KCL::g_os;

std::map<std::string, VertexShaderStruct> Shader::m_vs_cache;
std::map<std::string, PixelShaderStruct> Shader::m_ps_cache;
std::map<std::string, Shader*> Shader::m_shader_cache;
std::string Shader::m_shader_prefix;
Shader* Shader::m_default_shader = NULL;

using namespace std;

struct defineToTag
{
	std::string define;
	char c;
};

std::string Shader::getShaderPrefix()
{
	return m_shader_prefix;
}

void Shader::setShaderPrefix(std::string prefix)
{
	m_shader_prefix = prefix;
}

void Shader::InitShaders(const std::string &requiredRenderAPI, bool force_fs_mediump)
{
	if( requiredRenderAPI == "es3")
	{
		m_shader_prefix = "shaders.30/";
	}
	else if ( requiredRenderAPI == "es2")
	{
		m_shader_prefix = "shaders.20/";
	}
    else if ( requiredRenderAPI == "lowlevel")
    {
        m_shader_prefix = "lowlevel/";
    }
	else
	{
		m_shader_prefix = "";
	}
}

#if defined _DEBUG && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#pragma comment(lib,"D3DCompiler.lib")

int recurse_mkdir(std::string dir)
{
	if (KCL::File::MkDir(dir.c_str())==0) return 0;
	if (errno==EEXIST) return 0;
	if (dir.find_last_of("/")==std::string::npos) return 1;
	if (recurse_mkdir(dir.substr(0,dir.find_last_of("/")))!=0) return 1;
	return KCL::File::MkDir(dir.c_str());
}

void CompileShader( const std::string &source,const std::string &destination, const char* target, const std::set<std::string> *defines)
{
	ID3DBlob* out =nullptr;
	ID3DBlob* err =nullptr;

	wchar_t* buf = new wchar_t[source.length()+1];
	swprintf(buf,source.length()+1,L"%S",source.c_str());

	D3D_SHADER_MACRO* macros = NULL;
			
	if (defines && defines->size()>0)
	{
		macros = new D3D_SHADER_MACRO[defines->size()+1];
		macros[defines->size()].Name=NULL;
		macros[defines->size()].Definition=NULL;
		int i =0;
		for (set<string>::iterator it=defines->begin();it!=defines->end();it++)
		{
			const char* define = (*it).c_str();
			std::string::size_type spacepos = it->find(' ');
			std::string substr;
			if (spacepos!=std::string::npos)
			{
				substr = it->substr(0,spacepos);
				define = substr.c_str();
			}
			macros[i].Name=new char[strlen(define)+1];
			strcpy((char*)macros[i].Name,define);
			macros[i].Definition="1";
			i++;
		}
	}

	printf("[Compile] %s\n",destination.c_str());

	D3DCompileFromFile(buf,macros, D3D_COMPILE_STANDARD_FILE_INCLUDE ,"main",target,/*D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION*/0,0,&out,&err);
	if (err!=NULL)
	{
		char* buf = new char[ err->GetBufferSize()+1];
		ZeroMemory(buf,err->GetBufferSize()+1);
		memcpy(buf,err->GetBufferPointer(),err->GetBufferSize());
		printf("Shader compilation message: %s\n",buf);
	}

	if (macros!=NULL)
	{
		for (int i=0;i<defines->size();i++)
		{
			delete[] macros[i].Name;
		}
		delete[] macros;
	}
	delete[] buf;

	if (out!=NULL && out->GetBufferSize()>0)
	{
        std::string destDir = KCL::File::GetDataPath() + destination;
        destDir = destDir.substr(0,destDir.find_last_of("/"));
		recurse_mkdir(destDir);
		KCL::File f(KCL::File::GetDataPath() + destination, KCL::Write, KCL::RDir);
		f.Write(out->GetBufferPointer(),out->GetBufferSize(),1);
		f.Close();
		if (err!=NULL)
		{
			char* buf = new char[ err->GetBufferSize()+1];
			ZeroMemory(buf,err->GetBufferSize()+1);
			memcpy(buf,err->GetBufferPointer(),err->GetBufferSize());
			printf("=====COMPILE WARNING=====\n%s\n\n",buf);
		}
	} else {
		if (err!=NULL)
		{
			char* buf = new char[ err->GetBufferSize()+1];
			ZeroMemory(buf,err->GetBufferSize()+1);
			memcpy(buf,err->GetBufferPointer(),err->GetBufferSize());
			printf("=====COMPILE FAILED=====\n%s\n\n",buf);
		}
	}
}
#endif

std::string define_subst[] = { 
	"AALIGNED", "AA",
	"ALPHA_TEST", "AT",
	"ANIMATE_NORMAL", "AN",
	"DEP_TEXTURING", "DT",
	"EMISSION", "EM",
	"FOG", "FOG",
	"FRESNEL", "FR",
	"INSTANCING", "IN",
	"LIGHTING", "LT",
	"LIGHTMAP", "LM",
	"MASK", "MK",
	"NEED_HIGHP", "HP",
	"POINT_LIGHT", "PT",
	"REFLECTION", "REF",
	"RELIEF", "REL",
	"RGB_ENCODED", "RGB",
	"SHADOW_MAP", "SM",
	"SKELETAL", "SK",
	"SOFT_SHADOW", "SS",
	"SPECIAL_DIFFUSE_CLAMP", "SC",
	"SPOT_LIGHT", "SP",
	"STIPPLE", "ST",
	"TRANSITION_EFFECT", "TE",
	"TRANSLATE_UV", "UV",
	"TRANSPARENCY", "TR",
	"WIDE_DIFFUSE_CLAMP", "WC",
	"ZPREPASS", "ZP",
	"UBYTE_NORMAL_TANGENT", "UBNT",
	"" };

std::string define_postfix(const std::set<std::string> *defines)
{
	if (defines==NULL) return "";
	std::string accum = "";
	set<string>::iterator it;
	for (it=defines->begin();it!=defines->end();it++)
	{
		std::string def = it->substr(0, it->find(' '));
		for (std::string *ptr=define_subst;*ptr!="";ptr++)
		{
			if ((*ptr) == def)
			{
				def = *(ptr+1);
				break;
			}
		}

		accum+="."+def;
	}
	return accum;
}

std::string FindShader( const char* file, const std::set<std::string> *defines, const char* shader_ver)
{
	std::string path = KCL::File::GetDataPath() + "shaders_dx_bin/";

	std::string defs = define_postfix(defines);

	std::string dstnamewithdefs = file + defs + ".cso";
	std::string dstname = std::string(file) + ".cso";

#if defined _DEBUG && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	std::string sourcepath = KCL::File::GetDataPath() + "shaders_dx/";
	std::string srcname = std::string(file) + ".hlsl";

    std::string write_path = "shaders_dx_bin/";
	if (!IO::ExistsFile(dstname,path) && IO::ExistsFile(srcname, sourcepath)) 
    {
        CompileShader(sourcepath + srcname, write_path + dstname, shader_ver, NULL);
    }
	if (defs.size()>0)
	{
		if (!IO::ExistsFile(dstnamewithdefs,path) && IO::ExistsFile(srcname, sourcepath))
		{
			CompileShader(sourcepath + srcname, write_path + dstnamewithdefs, shader_ver, defines);
		}
	}
#endif
	if (IO::ExistsFile(dstnamewithdefs,path)) return dstnamewithdefs;
	if (IO::ExistsFile(dstname,path)) return dstname;
	return "";
}

Shader* Shader::CreateShader( const char* vsfile, const char* psfile, const std::set<std::string> *defines, KCL::KCL_Status& error, ShaderTypes::Enum shaderType)
{
	std::string verstr;

	if (m_shader_prefix == "shaders.30/")
	{
		verstr = "4_0";
	} else {
		verstr = "4_0_level_9_3";
	}

	std::string target_vs = FindShader((m_shader_prefix + vsfile).c_str(),defines,("vs_" + verstr).c_str());
	std::string target_ps = FindShader((m_shader_prefix + psfile).c_str(),defines,("ps_" + verstr).c_str());
	if (target_vs == "" || target_ps == "")
	{
		if (target_vs == "") printf("couldn't load %s\n",vsfile);
		if (target_ps == "") printf("couldn't load %s\n",psfile);
		return NULL;
	}
	return CreateShader( target_vs.c_str(), target_ps.c_str(), error);
}


Shader* Shader::CreateShader( const char* vsfile, const char* psfile, KCL::KCL_Status& error)
{
	string id(vsfile);
	id+= " ";
	id+= psfile;
	
	map<string, Shader*>::iterator it = m_shader_cache.find( id );
	if (it != m_shader_cache.end())
	{
		return it->second;
	}
	else
	{
		VertexShaderStruct vs;
		PixelShaderStruct ps;
		LoadVertexShader(vsfile, vs);
		LoadPixelShader(psfile, ps);
		
		Shader *shader = new Shader;
		shader->m_vs = vs;
		shader->m_ps = ps;

		m_shader_cache[id] = shader; 
		return shader;
	}
}


void Shader::LoadVertexShader(string filename, VertexShaderStruct &shader)
{
	map<string, VertexShaderStruct>::iterator it = m_vs_cache.find(filename);
	if (it != m_vs_cache.end())
	{
		shader = it->second;
	}
	else
	{
        std::string shaderPath = "shaders_dx_bin/";

		auto loadVS = IO::ReadDataSync(filename, shaderPath);
		auto bytecodeVS = loadVS.data;
		if (bytecodeVS.size()==0)
		{
			INFO("Missing shader: '%s'", filename.c_str());
			printf("Missing shader: '%s'\n", filename.c_str());
			return;
		}
		shader.m_bytes = bytecodeVS;

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateVertexShader(
				&bytecodeVS[0],
				bytecodeVS.size(),
				nullptr,
				&shader.m_shader
				)
			);
		
		shader.m_name = filename;
		m_vs_cache[filename] = shader; 
	}

}

void Shader::LoadPixelShader(string filename, PixelShaderStruct &shader)
{

	map<string, PixelShaderStruct>::iterator it = m_ps_cache.find(filename);
	if (it != m_ps_cache.end())
	{
		shader = it->second;
	}
	else
	{
        std::string shaderPath = "shaders_dx_bin/";

		auto loadPS = IO::ReadDataSync(filename, shaderPath);
		auto bytecodePS = loadPS.data;
		if (bytecodePS.size()==0)
		{
			INFO("Missing shader: '%s'", filename.c_str());
			printf("Missing shader: '%s'\n", filename.c_str());
			return;
		}
        
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreatePixelShader(
				&bytecodePS[0],
				bytecodePS.size(),
				nullptr,
				&shader.m_shader
				)
			);
		
		shader.m_name = filename;
		m_ps_cache[filename] = shader; 
	}

}


void Shader::DeleteShaders()
{
	m_vs_cache.clear();
	m_ps_cache.clear();
	
	for (std::map<std::string, Shader*>::iterator it=m_shader_cache.begin();it!=m_shader_cache.end();it++)
	{
		delete it->second;
		it->second = NULL;
	}
	m_shader_cache.clear();
}


Shader::Shader()
{
}


Shader::~Shader()
{
}


void Shader::Bind( )
{
	
    DX::getContext()->VSSetShader(
		m_vs.m_shader.Get(),
        nullptr,
        0
        );
    DX::getContext()->PSSetShader(
        m_ps.m_shader.Get(),
        nullptr,
        0
        );
}
