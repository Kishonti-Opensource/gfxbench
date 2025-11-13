/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "shader.h"

#import <Foundation/Foundation.h>

Shader* Shader::CreateShader( const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error, ShaderTypes::Enum shaderType)
{
    return new Shader(vsfile,fsfile,*defines) ;
}

Shader::Shader(std::string vs_file, std::string fs_file, const std::set<std::string> defines)
{
    m_vs_file = vs_file ;
    m_fs_file = fs_file ;
    
    m_defines = defines ;
}

std::string Shader::getFsFile()
{
    return m_fs_file ;
}

std::string Shader::getVsFile()
{
    return m_vs_file ;
}

std::set<std::string> Shader::getDefines()
{
    return m_defines ;
}

void Shader::AddDefine(const char * define)
{
    m_defines.insert(define);
}

