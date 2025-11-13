/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_PIPELINE_H
#define MTL_PIPELINE_H
#include <Metal/Metal.h>
#include "mtl_types.h"
#include "mtl_globals.h"

#include "kcl_material.h"
#include "kcl_scene_version.h"

#include <set>
#include <string>
#include <unordered_map>
#include "shader.h"

class MTLPipeLineBuilder ;

namespace MetalRender
{

class Pipeline
{
    friend class ::MTLPipeLineBuilder ;
    
public:
    Pipeline();
    virtual ~Pipeline();

    static void InitShaders(KCL::SceneVersion scene_version);
    
    struct Type
    {
        enum
        {
            RENDER_PIPELINE,
            COMPUTE_PIPELINE,
            INVALID
        };
    };

    typedef enum
    {
        DISABLED,
        ONE_X_ONE,
        ALPHA_X_ONE_MINUS_SOURCE_ALPHA,
        ONE_X_ONE_MINUS_SOURCE_ALPHA,
        COLOR_ALPHA_X_COLOR_ALPHA,
        COLOR_ALPHA_X_ZERO,
		NO_COLOR_WRITES,
        CONSTANT_COLOR_X_ONE_MINUS_CONSTANT_COLOR,
        ONE_X_ZERO,
        CONSTANT_ALPHA_X_ONE,
        CONSTANT_ALPHA_X_ONE_MINUS_CONSTANT_ALPHA,
        SRC_ALPHA_X_ONE_MINUS_SRC_ALPHA___X___ONE_X_ZERO,
        UNKNOWN
    } BlendType;
    
    
    struct GFXPipelineDescriptor
    {
        BlendType blendType ;
        ShaderType shaderType ;
        MTLVertexDescriptor* vertexDesc ;
        bool hasDepth ;
        bool forceHighP ;
        
        GFXPipelineDescriptor(BlendType blendType, ShaderType shaderType, MTLVertexDescriptor* vertexDesc,
                              bool hasDepth, bool forceHighP) :
            blendType(blendType),
            shaderType(shaderType),
            vertexDesc(vertexDesc),
            hasDepth(hasDepth),
            forceHighP(forceHighP)
        {}
    };
    

    static Pipeline* CreatePipeline(const char* vsname,
                                       const char* fsname,
                                       const std::set<std::string> *defines_set,
                                       const GFXPipelineDescriptor &gfx_desc,
                                       KCL::KCL_Status& error,
                                       MTLRenderPipelineDescriptor* base_desc = nil);
    
    static Pipeline* CreatePipeline(Shader* shader,
                                    const GFXPipelineDescriptor &gfx_desc,
                                    KCL::KCL_Status& error);
	
	static Pipeline* CreatePipelineFromLibrary(	const char* fileName,
													BlendType blendType,
													uint32_t renderTargetCount,
													bool hasDepth,
													bool depthWritesEnabled,
                                                    KCL::KCL_Status& err);

	static std::set<std::string> s_defines_debug;

    inline void Set(id <MTLRenderCommandEncoder> renderEncoder)
    {
        [renderEncoder setRenderPipelineState:m_pipeline];
    }
    
    void SetAsCompute(id <MTLComputeCommandEncoder> computeEncoder) ;
    unsigned int GetMaxThreadCount();
	
    inline unsigned int GetThreadExecutionWidth()
	{
        return [m_compute_pipeline threadExecutionWidth];
    }
    
    bool IsThreadCountOk(const char* pass_name, unsigned int thread_count);
	
	static Pipeline* s_default_pipeline;
    
    static void ClearCashes() ;

    std::string m_vs_name ;
    std::string m_fs_name ;
    std::string m_defines ;
    
    static MetalRender::ShaderType PixelFormatToShaderType(MTLPixelFormat pixelformat) ;
    static void SetupColorAttachment(MTLRenderPipelineColorAttachmentDescriptor* colorAttachment0, MetalRender::Pipeline::BlendType blendType);
    
protected:

    id <MTLRenderPipelineState> m_pipeline;
	id <MTLLibrary> m_vertexLibrary;
	id <MTLLibrary> m_fragmentLibrary;
    
    id <MTLComputePipelineState> m_compute_pipeline;
	
private:
    
    static std::string CollectDefines(const std::set<std::string> & defines_set) ;
    
    static Pipeline* CreateComputePipeline(const char* filename, const std::set<std::string> &defines_set, bool force_highp, bool tg = false) ;
    
    struct LoadShaderResult
    {
        id <MTLFunction> shaderFunction ;
        id <MTLLibrary> shaderLibrary ;
        
        LoadShaderResult()
        {
            shaderFunction = nil ;
            shaderLibrary = nil ;
        }
        
        ~LoadShaderResult()
        {
            releaseObj(shaderFunction) ;
            releaseObj(shaderLibrary) ;
        }
    };
    
    static bool LoadShaderSource(const std::string &shadername, std::string &source_str) ;
    static bool ResolveIncludes(std::string &source_str, std::set<std::string> &included_files) ;
    
    static LoadShaderResult LoadShader(const std::string &shadername, const std::string &defines) ;
    
	#ifdef DEBUG
	NSString* defines;
	#endif
    
	static std::unordered_map<unsigned long, Pipeline*> s_pipelineCache;
	static std::unordered_map<unsigned long, void*> s_shaderCache;
    static std::set<Pipeline*> s_computePipeLines;
	
    static std::vector<std::string> s_shader_dirs;
    
    id <MTLDevice> m_Device ;
};

}
#endif //MTL_PIPELINE_H
