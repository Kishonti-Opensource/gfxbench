/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_shader2.h"
#include "ng/log.h"

#include <algorithm>
#include <string>
#include <string.h>
#include <cctype>
#include "opengl/ext.h"

using namespace GLB;

static void AddDefaultPrecisions(GLBShaderBuilder::BuilderDescriptor &desc, bool force_highp);
static void AddExtensionDefine(GLBShaderBuilder::BuilderDescriptor &desc, const GLBExtId &id, const char *define, bool core_on_desktop = true);

void GLBShader2::InitShaders(KCL::SceneVersion scene_version, bool force_highp)
{
	is_highp = force_highp;

    GLBShaderBuilder::BuilderDescriptor desc;
    desc.m_stage_headers[GLBShaderBuilder::VERTEX_INDEX] += "#define TYPE_vertex\n"; 
    desc.m_stage_headers[GLBShaderBuilder::TESS_CONTROL_INDEX] += "#define TYPE_tess_control\n";
    desc.m_stage_headers[GLBShaderBuilder::TESS_EVALUATION_INDEX] += "#define TYPE_tess_eval\n";
    desc.m_stage_headers[GLBShaderBuilder::GEOMETRY_INDEX] += "#define TYPE_geometry\n";
    desc.m_stage_headers[GLBShaderBuilder::FRAGMENT_INDEX] += "#define TYPE_fragment\n";
    desc.m_stage_headers[GLBShaderBuilder::COMPUTE_INDEX] += "#define TYPE_compute\n";   

    const bool is_es = g_extension->isES();
	switch(scene_version)
	{
	case KCL::SV_25:
	case KCL::SV_27:
        desc.m_global_shader_dirs.push_back("shaders_40/shaders.20/");
        desc.m_target_limits = NULL;
		break;

	case KCL::SV_30:
        desc.m_global_shader_dirs.push_back("shaders_40/shaders.30/");
        desc.m_common_headers = is_es?"#version 300 es\n":"#version 400 core\n";
		desc.m_common_headers += "#define SV_30    1\n";
        desc.m_target_limits = NULL;
		break;

	case KCL::SV_31:
        desc.m_global_shader_dirs.push_back("shaders_40/shaders.31/");
        desc.m_global_shader_dirs.push_back("shaders_40/shaders.30/");
        desc.m_global_shader_dirs.push_back("shaders_40/common.31/");
        desc.m_common_headers = is_es?"#version 310 es\n":"#version 430 core\n";
        desc.m_common_headers += "#define GLSL_SHADER\n";
		desc.m_common_headers += "#define SV_31    1\n";
        desc.m_target_limits = ContextLimits::GetES31Limits();
		break;

	case KCL::SV_TESS:
	case KCL::SV_40:
		desc.m_global_shader_dirs.push_back("shaders_40/lowlevel4/");
        desc.m_global_shader_dirs.push_back("shaders_40/shaders.4/");
        desc.m_global_shader_dirs.push_back("shaders_40/common.31/");
        if (is_es)
        {           
            desc.m_common_headers = "#version 310 es\n";
            // Enable required extensions
            desc.m_stage_headers[GLBShaderBuilder::TESS_CONTROL_INDEX] += "#extension GL_EXT_tessellation_shader : enable\n";
            desc.m_stage_headers[GLBShaderBuilder::TESS_EVALUATION_INDEX] += "#extension GL_EXT_tessellation_shader : enable\n";       
            desc.m_stage_headers[GLBShaderBuilder::GEOMETRY_INDEX] += "#extension GL_EXT_geometry_shader : enable\n";		    
            // Optional extensions
		    if (g_extension->hasExtension(GLBEXT_primitive_bounding_box))
		    {
			    desc.m_stage_headers[GLBShaderBuilder::TESS_CONTROL_INDEX] += "#extension GL_EXT_primitive_bounding_box : enable\n";
		    }  
            if (g_extension->hasExtension(GLBEXT_texture_cube_map_array))
		    {
                desc.m_common_headers += "#extension GL_EXT_texture_cube_map_array : enable\n";
            }
        }
        else
        {
            desc.m_common_headers = (scene_version == KCL::SV_TESS)?"#version 400 core\n":"#version 430 core\n";
        }
        AddExtensionDefine(desc, GLBEXT_geometry_shader,        "HAS_GEOMETRY_SHADER_EXT");
        AddExtensionDefine(desc, GLBEXT_tessellation_shader,    "HAS_TESSELLATION_SHADER_EXT");
        AddExtensionDefine(desc, GLBEXT_texture_cube_map_array, "HAS_TEXTURE_CUBE_MAP_ARRAY_EXT");
        AddExtensionDefine(desc, GLBEXT_primitive_bounding_box, "HAS_PRIMITIVE_BOUNDING_BOX_EXT", false);

        desc.m_common_headers += "#define GLSL_SHADER\n";
        desc.m_common_headers += "#define SV_40    1\n";
		desc.m_target_limits = ContextLimits::GetES31_AEP_Limits();
		break;

	default:
		NGLOG_ERROR("GLBShader2::InitShaders - Unknown scene version: %s", scene_version);
	}

    AddDefaultPrecisions(desc, force_highp);
    GLBShaderBuilder::m_build_descriptor = desc;
    
	InitShaderCommon();
}

void AddExtensionDefine(GLBShaderBuilder::BuilderDescriptor &desc, const GLBExtId &id, const char *define,  bool core_on_desktop)
{
    bool has_extension = false;
    if (!g_extension->isES() && core_on_desktop) // This extension is a core feature on GL 4.3
    {      
        has_extension = true;
    }
    else if (g_extension->hasExtension(id)) // Check if the extension is supported
    {
        has_extension = true;     
    }
    if (has_extension)
    {
        desc.m_common_headers += std::string("#define ") + define + std::string("    1\n");
    }
}

void AddDefaultPrecisions(GLBShaderBuilder::BuilderDescriptor &desc, bool force_highp)
{   
    if (g_extension->isES())
    {
        if (!force_highp)
        {
            desc.m_stage_headers[GLBShaderBuilder::VERTEX_INDEX] += "precision highp float;\n";    
            desc.m_stage_headers[GLBShaderBuilder::VERTEX_INDEX] += "precision highp int;\n";    

            desc.m_stage_headers[GLBShaderBuilder::TESS_CONTROL_INDEX] += "precision highp float;\n";
            desc.m_stage_headers[GLBShaderBuilder::TESS_CONTROL_INDEX] += "precision highp int;\n";

            desc.m_stage_headers[GLBShaderBuilder::TESS_EVALUATION_INDEX] += "precision highp float;\n";
            desc.m_stage_headers[GLBShaderBuilder::TESS_EVALUATION_INDEX] += "precision highp int;\n";

            desc.m_stage_headers[GLBShaderBuilder::GEOMETRY_INDEX] += "precision highp float;\n";
            desc.m_stage_headers[GLBShaderBuilder::GEOMETRY_INDEX] += "precision highp int;\n";

            desc.m_stage_headers[GLBShaderBuilder::FRAGMENT_INDEX] += "precision mediump float;\n";
            desc.m_stage_headers[GLBShaderBuilder::FRAGMENT_INDEX] += "precision mediump int;\n";
        }
        else
        {
            desc.m_stage_headers[GLBShaderBuilder::VERTEX_INDEX] += "precision highp float;\n";
            desc.m_stage_headers[GLBShaderBuilder::VERTEX_INDEX] += "precision highp int;\n";

            desc.m_stage_headers[GLBShaderBuilder::TESS_CONTROL_INDEX] += "precision highp float;\n";
            desc.m_stage_headers[GLBShaderBuilder::TESS_CONTROL_INDEX] += "precision highp int;\n";

            desc.m_stage_headers[GLBShaderBuilder::TESS_EVALUATION_INDEX] += "precision highp float;\n";
            desc.m_stage_headers[GLBShaderBuilder::TESS_EVALUATION_INDEX] += "precision highp int;\n";

            desc.m_stage_headers[GLBShaderBuilder::GEOMETRY_INDEX] += "precision highp float;\n";
            desc.m_stage_headers[GLBShaderBuilder::GEOMETRY_INDEX] += "precision highp int;\n";

            desc.m_stage_headers[GLBShaderBuilder::FRAGMENT_INDEX] += "precision highp float;\n";
            desc.m_stage_headers[GLBShaderBuilder::FRAGMENT_INDEX] += "precision highp int;\n";
        }

        desc.m_stage_headers[GLBShaderBuilder::COMPUTE_INDEX] += "precision highp float;\n";   
        desc.m_stage_headers[GLBShaderBuilder::COMPUTE_INDEX] += "precision highp int;\n";  
    }
    else
    {
        desc.m_common_headers += "#define highp\n";
        desc.m_common_headers += "#define mediump\n";
        desc.m_common_headers += "#define lowp\n";
    }
}