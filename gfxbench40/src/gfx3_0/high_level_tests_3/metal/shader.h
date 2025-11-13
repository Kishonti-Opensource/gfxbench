/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __MTL_SHADER_H__
#define __MTL_SHADER_H__

#include <set>
#include <kcl_base.h>
#include <string>

class Shader
{
public:
    struct ShaderTypes
    {
        enum Enum
        {
            Default,
            TransformFeedback
        };
    };
    
    static Shader* CreateShader( const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error, ShaderTypes::Enum shaderType = ShaderTypes::Default) ;
    
    std::string getVsFile() ;
    std::string getFsFile() ;
    std::set<std::string> getDefines();
    void AddDefine(const char * define);
    
private:
    
    std::string m_vs_file ;
    std::string m_fs_file ;
    
    std::set<std::string> m_defines ;
    
    Shader(std::string vs_file, std::string fs_file, const std::set<std::string> defines) ;
};


#endif // __MTL_SHADER_H__