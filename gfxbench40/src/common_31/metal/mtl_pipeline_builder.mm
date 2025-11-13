/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_pipeline_builder.h"

#include <sstream>
#include <iomanip>
#include <numeric>

using namespace MetalRender ;

static const int FLOAT_LITERAL_PRECISION = 10;

MTL_Scene_Base *MTLPipeLineBuilder::s_scene = nullptr ;
KCL::SceneVersion MTLPipeLineBuilder::s_scene_version = KCL::SV_INVALID;


MTLPipeLineBuilder::MTLPipeLineBuilder()
{
    Clear() ;
}


MTLPipeLineBuilder::~MTLPipeLineBuilder()
{
    Clear() ;
}


void MTLPipeLineBuilder::Clear()
{
    m_base_descriptor = nil;
    m_defines_set.clear();
    m_shader_file = "";
    m_pipeline_type = Pipeline::Type::INVALID;
    
    m_vertex_layout = nullptr ;
    m_shader_type = kShaderTypeSingleRGBA8Default ;
    m_has_depth = false;
    m_force_highp = false;
	m_always_tg	= false;
    m_blend_type = MetalRender::Pipeline::DISABLED;
}


void MTLPipeLineBuilder::DetectPipeLineType()
{
    if (strstr(m_shader_file.c_str(),".cshader"))
    {
        m_pipeline_type = Pipeline::Type::COMPUTE_PIPELINE ;
        return ;
    }
    
    //INFO("WARNING: Runtime type check of %s shader",m_shader_file.c_str()) ;
    
    std::string shader_source ;
    if ( !Pipeline::LoadShaderSource(m_shader_file,shader_source) )
    {
        return ;
    }
    
    if (shader_source.find("TYPE_compute") != std::string::npos)
    {
        m_pipeline_type = Pipeline::Type::COMPUTE_PIPELINE ;
        return ;
    }
    else
    {
        bool has_vertex   = shader_source.find("TYPE_vertex") ;
        bool has_fragment = shader_source.find("TYPE_fragment") ;
        
        if (has_vertex && has_fragment)
        {
            m_pipeline_type = Pipeline::Type::RENDER_PIPELINE ;
            return ;
        }
    }
}


MTLPipeLineBuilder &MTLPipeLineBuilder::SetBaseDescriptor(MTLRenderPipelineDescriptor* desc)
{
    m_base_descriptor = desc;
	return *this;
}

MetalRender::Pipeline *MTLPipeLineBuilder::Build(KCL::KCL_Status& error)
{
    error = KCL::KCL_TESTERROR_NOERROR;
    MetalRender::Pipeline* result = nullptr ;
    
    // detect type runtime
    if (m_pipeline_type == Pipeline::Type::INVALID)
    {
        DetectPipeLineType();
    }
    
	std::set<std::string> defines_set;
	
    if (s_scene_version != KCL::SV_INVALID)
    {
        assert(s_scene);
        AddDefine( s_scene->GetVersionStr() ) ;
        m_force_highp = m_force_highp || s_scene->IsForceHighp() ;
		defines_set.insert(s_scene->global_defines.begin(), s_scene->global_defines.end());
    }
    
	defines_set.insert(m_defines_set.begin(), m_defines_set.end());

    if (m_pipeline_type == Pipeline::Type::RENDER_PIPELINE)
    {
        Pipeline::GFXPipelineDescriptor pipeline_desc(m_blend_type,m_shader_type, m_vertex_layout, m_has_depth, m_force_highp);
        
        result = Pipeline::CreatePipeline(m_shader_file.c_str(), m_shader_file.c_str(), &defines_set, pipeline_desc, error, m_base_descriptor) ;
    }
    else if (m_pipeline_type == Pipeline::Type::COMPUTE_PIPELINE)
    {
		result = Pipeline::CreateComputePipeline(m_shader_file.c_str(), defines_set, m_force_highp, false);
		
        if(m_always_tg)
        {
            const auto threadExecutionWidth = [result->m_compute_pipeline threadExecutionWidth];
			
			// pipelines freed by Pipeline::ClearCashes
			//delete result;

            char strbuild[128];
            sprintf(strbuild, "THREADS_PER_THREADGROUP %lu\n", threadExecutionWidth);
            defines_set.emplace(strbuild);
            sprintf(strbuild, "CONTROL_POINTS_PER_THREAD %lu\n", std::max<int>(1, 16 / threadExecutionWidth));
            defines_set.emplace(strbuild);

            result = Pipeline::CreateComputePipeline(m_shader_file.c_str(), defines_set, m_force_highp, m_always_tg);

            assert(threadExecutionWidth == [result->m_compute_pipeline threadExecutionWidth] && "Threadgroup size not a multipule of threadExecutionWidth");
        }
    }
    else
    {
        assert(0) ;
    }
    
    Clear();
    return result ;
}



MTLPipeLineBuilder &MTLPipeLineBuilder::ShaderFile(const char *file)
{
    if (file)
    {
        m_shader_file = file;
    }
    else
    {
        m_shader_file.clear();
    }
    return *this;
}


MTLPipeLineBuilder &MTLPipeLineBuilder::AddShaderDir(const std::string &dir)
{
    Pipeline::s_shader_dirs.insert(Pipeline::s_shader_dirs.begin(), dir);
    return *this;
}

MTLPipeLineBuilder &MTLPipeLineBuilder::SetAlwaysTGSize(bool yes)
{
    m_always_tg = yes;
    return *this;
}

MTLPipeLineBuilder &MTLPipeLineBuilder::AddDefine(const std::string &define)
{
    m_defines_set.insert(define);
    return *this;
}


MTLPipeLineBuilder &MTLPipeLineBuilder::AddDefines(const std::set<std::string> *defines)
{
	m_defines_set.insert(defines->begin(),defines->end());
	return *this;
}


MTLPipeLineBuilder &MTLPipeLineBuilder::AddDefineVec2(const std::string &name, const KCL::Vector2D &value)
{
    std::stringstream sstream;
    sstream<<std::fixed<<std::setprecision(FLOAT_LITERAL_PRECISION);
    sstream << name << " _float2(" << value.x << ", " << value.y << ")";
    m_defines_set.insert(sstream.str());
    return *this;
}


MTLPipeLineBuilder &MTLPipeLineBuilder::AddDefineInt(const std::string &name, int value)
{
    std::stringstream sstream;
    sstream << name << " " << value;
    m_defines_set.insert(sstream.str());
    return *this;
}


MTLPipeLineBuilder &MTLPipeLineBuilder::AddDefineIVec2(const std::string &name, KCL::int32 x, KCL::int32 y)
{
    std::stringstream sstream;
    sstream << name << " _float2(" << x << ", " << y << ")";
    m_defines_set.insert(sstream.str());
    return *this;
}


void MTLPipeLineBuilder::AddGlobalDefine(const std::string &name, int value)
{
	std::stringstream sstream;
	sstream << name << " " << value;
	s_scene->global_defines.insert(sstream.str());
}

void MTLPipeLineBuilder::RemoveGlobalDefine(const std::string &name)
{
	std::set<std::string>::iterator it = s_scene->global_defines.find(name);

	if (it == s_scene->global_defines.end())
	{
		return;
	}

	s_scene->global_defines.erase(it);
}

MTLPipeLineBuilder &MTLPipeLineBuilder::SetType(MetalRender::ShaderType shader_type)
{
    m_shader_type = shader_type ;
    return *this ;
}


MTLPipeLineBuilder &MTLPipeLineBuilder::SetTypeByPixelFormat(MTLPixelFormat pixel_format)
{
    m_shader_type = Pipeline::PixelFormatToShaderType(pixel_format) ;
    return *this ;
}


MTLPipeLineBuilder &MTLPipeLineBuilder::HasDepth(bool has_depth)
{
    m_has_depth = has_depth ;
    return *this ;
}


MTLPipeLineBuilder &MTLPipeLineBuilder::SetVertexLayout(MTLVertexDescriptor* vertex_layout)
{
    m_vertex_layout = vertex_layout ;
    return *this ;
}


MTLPipeLineBuilder &MTLPipeLineBuilder::SetBlendType(MetalRender::Pipeline::BlendType blend_type)
{
    m_blend_type = blend_type ;
    return *this ;
}


MTLPipeLineBuilder &MTLPipeLineBuilder::ForceHighp(bool force_highp)
{
    m_force_highp = force_highp ;
    return *this ;
}


void MTLPipeLineBuilder::SetScene(KCL::SceneVersion scene_version, MTL_Scene_Base *scene)
{
    s_scene_version = scene_version;
    s_scene = scene;
}


