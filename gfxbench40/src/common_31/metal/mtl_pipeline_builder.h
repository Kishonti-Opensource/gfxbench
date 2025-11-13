/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __MTL_PIPELINE_BUILDER_H__
#define __MTL_PIPELINE_BUILDER_H__

#include "mtl_pipeline.h"
#include "mtl_scene_base.h"

#include <string>
#include <set>


class MTLPipeLineBuilder
{
public:
    
    MTLPipeLineBuilder() ;
    ~MTLPipeLineBuilder();
    
    //Add custom shader directory
    MTLPipeLineBuilder &AddShaderDir(const std::string &dir);
    
    // Add some custom defines to the code
    MTLPipeLineBuilder &AddDefine(const std::string &define);
	MTLPipeLineBuilder &AddDefines(const std::set<std::string> *defines);
    MTLPipeLineBuilder &AddDefineVec2(const std::string &name, const KCL::Vector2D &value);
    MTLPipeLineBuilder &AddDefineInt(const std::string &name, int value);
    MTLPipeLineBuilder &AddDefineIVec2(const std::string &name, KCL::int32 x, KCL::int32 y);
    
    
    MTLPipeLineBuilder &SetBaseDescriptor(MTLRenderPipelineDescriptor* desc);
    MTLPipeLineBuilder &SetAlwaysTGSize(bool yes);
    MTLPipeLineBuilder &SetType(MetalRender::ShaderType shader_type);
    MTLPipeLineBuilder &SetTypeByPixelFormat(MTLPixelFormat pixel_format) ;
    MTLPipeLineBuilder &HasDepth(bool has_depth);
    MTLPipeLineBuilder &SetVertexLayout(MTLVertexDescriptor* vertex_layout);
    MTLPipeLineBuilder &SetBlendType(MetalRender::Pipeline::BlendType blend_type);
    MTLPipeLineBuilder &ForceHighp(bool force_highp);
    
    // Load shader from .shader file
    MTLPipeLineBuilder &ShaderFile(const char *file);
    
    // Create the shader
    MetalRender::Pipeline *Build(KCL::KCL_Status& error);
  
	static void AddGlobalDefine(const std::string &name, int value);
	static void RemoveGlobalDefine(const std::string &name);

    static void SetScene(KCL::SceneVersion scene_version, MTL_Scene_Base *scene);
    
private:
    
    void Clear() ;
    void DetectPipeLineType() ;
    
    std::set<std::string> m_defines_set;
    std::string m_shader_file;
    KCL::uint32 m_pipeline_type;
    
    MTLVertexDescriptor* m_vertex_layout;
    MetalRender::ShaderType m_shader_type ;
    MetalRender::Pipeline::BlendType m_blend_type;
    bool m_has_depth;
    bool m_force_highp;
    bool m_always_tg;
    
    MTLRenderPipelineDescriptor* m_base_descriptor;

    static KCL::SceneVersion s_scene_version;
    static MTL_Scene_Base *s_scene;
};


#endif // __MTL_PIPELINE_BUILDER_H__

