/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_pipeline.h"

#include <sstream>
#include "ng/log.h"

using namespace MetalRender;


bool Pipeline::LoadShaderSource(const std::string &shadername, std::string &source_str)
{
    bool file_found = false ;
    std::string s_shader_dir = "";
    
    for (int i = 0; i < s_shader_dirs.size(); i++)
    {
        if (KCL::AssetFile::Exists(s_shader_dirs[i] + shadername))
        {
            s_shader_dir = s_shader_dirs[i];
            file_found = true ;
            break;
        }
    }
    
    if (!file_found)
    {
        return false ;
    }
    
    std::string shaderFilePath = s_shader_dir + shadername;
    KCL::AssetFile shader_file(shaderFilePath);
    
    if(shader_file.GetLastError())
    {
        return false ;
    }
    
    source_str = shader_file.GetBuffer();
    return true;
}


bool Pipeline::ResolveIncludes(std::string &source_str, std::set<std::string> &included_files)
{
    std::istringstream source_str_stream(source_str) ;
    std::ostringstream output_stream ;
    
    std::size_t first_pos ;
    std::size_t second_pos ;
    
    while (!source_str_stream.eof())
    {
        std::string line ;
        getline(source_str_stream, line) ;
        
        if ( line.find("#include") != std::string::npos )
        {
            // Search for the < charachter
            first_pos = line.find('<');
            if (first_pos != std::string::npos )
            {
                output_stream << line << std::endl;
                continue ;
            }
            
            // Search for the first " charachter
            first_pos = line.find('"');
            // Also check if it is not the last one
            if (first_pos == std::string::npos || first_pos >= line.size() - 1)
            {
                NGLOG_ERROR("Shader error! #include \"filename\" expected in line: %s", line.c_str());
                return false;
            }
            
            // Search for the second " charachter
            second_pos = line.find('"', first_pos + 1);
            // Also check if there is someting between them
            if (second_pos == std::string::npos || second_pos - first_pos - 1 < 1)
            {
                NGLOG_ERROR("Shader error! #include \"filename\" expected in line: %s", line.c_str());
                return false;
            }
            
            // Include the file
            std::string header_file_name = line.substr(first_pos + 1, second_pos - first_pos - 1);
            if (included_files.find(header_file_name) != included_files.end())
            {
                // This file is already included, now we just skip this include directive
                continue;
            }
            
            std::string included_file;
            if( LoadShaderSource(header_file_name, included_file) )
            {
                included_files.insert(header_file_name);
                if ( !ResolveIncludes(included_file, included_files) )
                {
                    return false;
                }
                output_stream << included_file << std::endl;
            }
            else
            {
                output_stream << line << std::endl;
            }
        }
        else
        {
            // Just copy this line
            output_stream << line << std::endl;
        }
        
    }
    
    source_str = output_stream.str() ;
    return true ;
}


