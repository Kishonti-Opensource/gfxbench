/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#import <Foundation/Foundation.h>
#include "mtl_globals.h"
#include "mtl_pipeline.h"
#include "zlib.h" //for adler32 - should we use jenkins instead?
#include "mtl_types.h"
#import "graphics/metalgraphicscontext.h"
#ifdef OPT_TEST_GFX40
#include "metal/mtl_scene_40.h"
#include "metal/mtl_gbuffer.h"
#endif

// Warning: Turning off the cache cause random crash in t-rex
#define PIPELINE_CACHE_ENABLED 1
#define ASSERT_ON_SHADER_COMPILE_ERROR 0

#define COMMON_INCLUDE "common.h"
#define FLOAT_HEADER "../common/float_header.h"

using namespace MetalRender;

std::unordered_map<unsigned long, Pipeline*> Pipeline::s_pipelineCache;
std::unordered_map<unsigned long, void*> Pipeline::s_shaderCache;
std::set<Pipeline*> Pipeline::s_computePipeLines;

std::set<std::string> Pipeline::s_defines_debug;
std::vector<std::string> Pipeline::s_shader_dirs;

Pipeline* Pipeline::s_default_pipeline;

#if !PIPELINE_CACHE_ENABLED
unsigned long g_cache_id = 0 ;
#endif


#define PIPELINE_CACHE_LOG_ENABLED 0
#if PIPELINE_CACHE_LOG_ENABLED
unsigned long g_shader_cache_miss = 0;
unsigned long g_shader_cache_hit  = 0;
unsigned long g_pipeline_cache_miss = 0;
unsigned long g_pipeline_cache_hit  = 0;

void log_shader_hit()    { INFO("SHADER_HIT:    %d", ++g_shader_cache_hit )   ; }
void log_shader_miss()   { INFO("SHADER_MISS:   %d", ++g_shader_cache_miss)   ; }
void log_pipeline_hit()  { INFO("PIPELINE_HIT:  %d", ++g_pipeline_cache_hit)  ; }
void log_pipeline_miss() { INFO("PIPELINE_MISS: %d", ++g_pipeline_cache_miss) ; }
#else
#define log_shader_hit(...) ;
#define log_shader_miss(...) ;
#define log_pipeline_hit(...) ;
#define log_pipeline_miss(...) ;
#endif


Pipeline::Pipeline()
{
    m_pipeline = nil;
    m_compute_pipeline = nil;
}

Pipeline::~Pipeline()
{	
	releaseObj(m_pipeline);
	releaseObj(m_vertexLibrary);
	releaseObj(m_fragmentLibrary);
}

void Pipeline::InitShaders(KCL::SceneVersion scene_version)
{
	if(scene_version == KCL::SV_40)
	{
		s_shader_dirs.push_back("shaders_mtl/shaders.40/");
		s_shader_dirs.push_back("shaders_mtl/common.31/");
	}
    else if(scene_version == KCL::SV_30)
	{
		s_shader_dirs.push_back("shaders_mtl/shaders.30/");
	}
    else if(scene_version == KCL::SV_31)
    {
        s_shader_dirs.push_back("shaders_mtl/shaders.31/");
        s_shader_dirs.push_back("shaders_mtl/shaders.30/");
        s_shader_dirs.push_back("shaders_mtl/common.31/");
    }
    else if(scene_version == KCL::SV_27)
    {
        s_shader_dirs.push_back("shaders_mtl/shaders.20/");
    }
    else
    {
        assert(0) ;
    }
}


void Pipeline::SetupColorAttachment(MTLRenderPipelineColorAttachmentDescriptor* colorAttachment0, MetalRender::Pipeline::BlendType blendType)
{
    //different blending depending on material
    switch(blendType)
    {
        case ALPHA_X_ONE_MINUS_SOURCE_ALPHA:
        {
            colorAttachment0.blendingEnabled = YES;
            colorAttachment0.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
            colorAttachment0.sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
            colorAttachment0.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            colorAttachment0.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            break;
        }
        case ONE_X_ONE:
        {
            colorAttachment0.blendingEnabled = YES;
            colorAttachment0.sourceRGBBlendFactor = MTLBlendFactorOne;
            colorAttachment0.sourceAlphaBlendFactor = MTLBlendFactorOne;
            colorAttachment0.destinationRGBBlendFactor = MTLBlendFactorOne;
            colorAttachment0.destinationAlphaBlendFactor = MTLBlendFactorOne;
            break;
        }
        case ONE_X_ONE_MINUS_SOURCE_ALPHA:
        {
            colorAttachment0.blendingEnabled = YES;
            colorAttachment0.sourceRGBBlendFactor = MTLBlendFactorOne;
            colorAttachment0.sourceAlphaBlendFactor = MTLBlendFactorOne;
            colorAttachment0.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            colorAttachment0.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            break;
        }
        case COLOR_ALPHA_X_COLOR_ALPHA:
        {
            colorAttachment0.blendingEnabled = YES;
            colorAttachment0.sourceRGBBlendFactor = MTLBlendFactorDestinationColor;
            colorAttachment0.sourceAlphaBlendFactor = MTLBlendFactorDestinationAlpha;
            colorAttachment0.destinationRGBBlendFactor = MTLBlendFactorSourceColor;
            colorAttachment0.destinationAlphaBlendFactor = MTLBlendFactorSourceAlpha;
            break;
        }
        case COLOR_ALPHA_X_ZERO:
        {
            colorAttachment0.blendingEnabled = YES;
            colorAttachment0.sourceRGBBlendFactor = MTLBlendFactorDestinationColor;
            colorAttachment0.sourceAlphaBlendFactor = MTLBlendFactorDestinationAlpha;
            colorAttachment0.destinationRGBBlendFactor = MTLBlendFactorZero;
            colorAttachment0.destinationAlphaBlendFactor = MTLBlendFactorZero;
            break;
        }
        case DISABLED:
        {
            colorAttachment0.blendingEnabled = NO;
            break;
        }
        case NO_COLOR_WRITES:
        {
            colorAttachment0.writeMask = 0;
            break;
        }
        case CONSTANT_COLOR_X_ONE_MINUS_CONSTANT_COLOR:
        {
            colorAttachment0.blendingEnabled = YES;
            colorAttachment0.sourceRGBBlendFactor = MTLBlendFactorBlendColor;
            colorAttachment0.sourceAlphaBlendFactor = MTLBlendFactorBlendColor;
            colorAttachment0.destinationRGBBlendFactor = MTLBlendFactorOneMinusBlendColor;
            colorAttachment0.destinationAlphaBlendFactor = MTLBlendFactorOneMinusBlendColor;
            break;
        }
        case ONE_X_ZERO:
        {
            colorAttachment0.blendingEnabled = YES;
            colorAttachment0.sourceRGBBlendFactor = MTLBlendFactorOne;
            colorAttachment0.sourceAlphaBlendFactor = MTLBlendFactorOne;
            colorAttachment0.destinationRGBBlendFactor = MTLBlendFactorZero;
            colorAttachment0.destinationAlphaBlendFactor = MTLBlendFactorZero;
            break;
        }
        case CONSTANT_ALPHA_X_ONE:
        {
            colorAttachment0.blendingEnabled = YES;
            colorAttachment0.sourceRGBBlendFactor = MTLBlendFactorBlendAlpha;
            colorAttachment0.sourceAlphaBlendFactor = MTLBlendFactorBlendAlpha;
            colorAttachment0.destinationRGBBlendFactor = MTLBlendFactorOne;
            colorAttachment0.destinationAlphaBlendFactor = MTLBlendFactorOne;
            break;
        }
        case CONSTANT_ALPHA_X_ONE_MINUS_CONSTANT_ALPHA:
        {
            colorAttachment0.blendingEnabled = YES;
            colorAttachment0.sourceRGBBlendFactor = MTLBlendFactorBlendAlpha;
            colorAttachment0.sourceAlphaBlendFactor = MTLBlendFactorBlendAlpha;
            colorAttachment0.destinationRGBBlendFactor = MTLBlendFactorOneMinusBlendAlpha;
            colorAttachment0.destinationAlphaBlendFactor = MTLBlendFactorOneMinusBlendAlpha;
            break;
        }
        case SRC_ALPHA_X_ONE_MINUS_SRC_ALPHA___X___ONE_X_ZERO:
        {
            colorAttachment0.blendingEnabled = YES;
            colorAttachment0.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
            colorAttachment0.sourceAlphaBlendFactor = MTLBlendFactorOne;
            colorAttachment0.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            colorAttachment0.destinationAlphaBlendFactor = MTLBlendFactorZero;
            break;
        }
        default:
            assert(0) ;
            break;
    }
}

Pipeline::LoadShaderResult Pipeline::LoadShader(const std::string &shadername, const std::string &defines)
{
    LoadShaderResult r ;

    KCL::KCL_Status error ;
    
#if PIPELINE_CACHE_ENABLED
    
    uLong runningHash = 0 ;
    runningHash = adler32(runningHash, (const unsigned char*) defines.c_str(), (uInt) defines.size());
    runningHash = adler32(runningHash, (const unsigned char*) shadername.c_str(), (uInt) shadername.size());
    auto shaderIt = s_shaderCache.find(runningHash);
    if(shaderIt != s_shaderCache.end())
    {
        log_shader_hit();
        r.shaderFunction = (__bridge id <MTLFunction>) shaderIt->second;
    }
#endif
    
    if(!r.shaderFunction)
    {
        log_shader_miss();
        
        std::string shader_file_str ;
        
        if( !LoadShaderSource(shadername, shader_file_str) )
        {
            INFO("ERROR: Shader %s not found!\n", shadername.c_str());
            error = KCL::KCL_TESTERROR_FILE_NOT_FOUND;
            return r ;
        }
        
        std::string shader_source_str = "" ;
#if TARGET_OS_EMBEDDED
        shader_source_str += "#define IOS\n" ;
        shader_source_str += "#define PLATFORM_IOS\n" ;
#else
        shader_source_str += "#define OSX\n" ;
        shader_source_str += "#define PLATFORM_OSX\n" ;
#endif
        shader_source_str += defines+"\n" ;
        shader_source_str += std::string("#include \"") + FLOAT_HEADER + "\"\n";
        shader_source_str += std::string("#include \"") + COMMON_INCLUDE + "\"\n";
        shader_source_str += shader_file_str ;
        
        std::set<std::string> included_files ;
        if ( !ResolveIncludes(shader_source_str, included_files ))
        {
            error = KCL::KCL_TESTERROR_FILE_NOT_FOUND;
            return r ;
        }
        
        NSString* shader_source = [NSString stringWithUTF8String:shader_source_str.c_str()];
        
        NSError* error = nil;
        r.shaderLibrary = [MetalRender::GetContext()->getDevice() newLibraryWithSource:shader_source options:nil error:&error];
        if(!r.shaderLibrary)
        {
            printf("Shader %s failed to build:\n%s\n",shadername.c_str(), error.localizedDescription.UTF8String);
            return r ;
        }
        assert(r.shaderLibrary);
        r.shaderFunction = [r.shaderLibrary newFunctionWithName:@"shader_main"];
        assert(r.shaderFunction);
#if PIPELINE_CACHE_ENABLED
        s_shaderCache[runningHash] = (__bridge_retained void*) r.shaderFunction;
#else
        s_shaderCache[++g_cache_id] = (__bridge_retained void*) r.shaderFunction;
#endif
    }
    
    return r ;
}


std::string Pipeline::CollectDefines(const std::set<std::string> & defines_set)
{
    std::string defines = "" ;
    
    std::set<std::string>::const_iterator it = defines_set.begin();
    
    while(it != defines_set.end())
    {
        defines.append(std::string("#define ") + *it + std::string("\n"));
        it++;
    }
    
    return defines ;
}


Pipeline* Pipeline::CreatePipeline(const char* vsname,
                                         const char* fsname,
                                         const std::set<std::string> *defines_set,
                                         const GFXPipelineDescriptor &gfx_desc,
                                         KCL::KCL_Status& error,
                                         MTLRenderPipelineDescriptor* base_desc)
{
	std::string defines;
    
	//always use UBO
	defines.append("#define USE_UBOs\n");
	defines.append("#define HAS_TEXTURE_CUBE_MAP_ARRAY_EXT\n");
	
	if(defines_set)
	{
        defines.append(CollectDefines(*defines_set)) ;
	}
	
	
	//debug defines
	std::set<std::string>::const_iterator it = s_defines_debug.begin();
	while(it != s_defines_debug.end())
	{
		defines.append(std::string("#define ") + *it + std::string("\n"));
		it++;
	}
	
    std::string vsFilePath = vsname;
	std::string fsFilePath = fsname;
	
	    
#if PIPELINE_CACHE_ENABLED
    
    //hash the defines, the shader names and the material type + shader type
    //this should get us a unique key for this pipeline
    uLong pipelineHash = 0;
    pipelineHash = adler32(pipelineHash, (const unsigned char*) defines.c_str(), (uInt) defines.size());
	pipelineHash = adler32(pipelineHash, (const unsigned char*) vsFilePath.c_str(), (uInt) vsFilePath.size());
	pipelineHash = adler32(pipelineHash, (const unsigned char*) fsFilePath.c_str(), (uInt) fsFilePath.size());
    pipelineHash = adler32(pipelineHash, (const unsigned char*) &gfx_desc, sizeof(gfx_desc));
    
	
	auto pipelineIt = s_pipelineCache.find(pipelineHash);
	static unsigned int cacheHit = 0;
	if(pipelineIt != s_pipelineCache.end())
	{
        log_pipeline_hit();
		cacheHit++;
		return pipelineIt->second;
	}
#endif
	
	//so we don't have this pipeline, but we may have the individual shaders somewhere
    log_pipeline_miss();
    
    LoadShaderResult vs ;
    LoadShaderResult fs ;
    
    std::string vs_defines = "#define TYPE_vertex\n" ;
    std::string fs_defines = "#define TYPE_fragment\n" ;
    
    vs_defines += defines ;
    fs_defines += defines ;
    
    vs_defines += "#define FORCE_HIGHP 1\n" ;
    
    
    if (gfx_desc.forceHighP)
    {
        vs_defines += "#define VARYING_HIGHP 1\n" ;
        
        fs_defines += "#define FORCE_HIGHP 1\n" ;
        fs_defines += "#define VARYING_HIGHP 1\n" ;
    }
    else
    {
        vs_defines += "#define VARYING_HIGHP 0\n" ;

        fs_defines += "#define FORCE_HIGHP 0\n" ;
        fs_defines += "#define VARYING_HIGHP 0\n" ;
    }
    
    vs = LoadShader(vsname,vs_defines) ;
	
	//find fragment shader
    fs = LoadShader(fsname,fs_defines) ;
	
#if ASSERT_ON_SHADER_COMPILE_ERROR
    assert(vs.shaderFunction && fs.shaderFunction);
#endif
    
    if ( !(vs.shaderFunction && fs.shaderFunction) )
    {
        error = KCL::KCL_TESTERROR_SHADER_ERROR;
        return nullptr ;
    }
    
    MTLRenderPipelineDescriptor* desc;
    
    if(base_desc != nil)
	{
        desc = [base_desc copy];
	}
    else
	{
        desc = [[MTLRenderPipelineDescriptor alloc] init];
	}
	
	desc.vertexFunction = vs.shaderFunction;
	desc.fragmentFunction = fs.shaderFunction;
    
    if (gfx_desc.vertexDesc != nullptr)
    {
        [desc setVertexDescriptor:gfx_desc.vertexDesc] ;
    }
    
	
	MTLRenderPipelineColorAttachmentDescriptor* colorAttachment0 = desc.colorAttachments[0];
    SetupColorAttachment(colorAttachment0, gfx_desc.blendType);
    
	
	if(gfx_desc.hasDepth)
	{
		desc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
	}
	else
    {
        desc.depthAttachmentPixelFormat = MTLPixelFormatInvalid ;
    }
    
    if(gfx_desc.shaderType == kShaderTypeTransformFeedback)
    {
        // Mask off everything
        colorAttachment0.blendingEnabled = NO;
        colorAttachment0.pixelFormat = MTLPixelFormatRGBA8Unorm;
        
        desc.fragmentFunction = nil;
        desc.rasterizationEnabled = NO;
    }
	else if(gfx_desc.shaderType == kShaderTypeSingleBGRA8)
	{
		//output format is BGRA
		colorAttachment0.pixelFormat = MTLPixelFormatBGRA8Unorm;
	}
	else if(gfx_desc.shaderType == kShaderTypeSingleRGBA8Default)
	{
		//all our shaders have at least 1 output of RGBA8
		colorAttachment0.pixelFormat = MTLPixelFormatRGBA8Unorm;
	}
    else if (gfx_desc.shaderType == kShaderTypeSingleBGR565)
    {
#if TARGET_OS_EMBEDDED
        colorAttachment0.pixelFormat = MTLPixelFormatB5G6R5Unorm ;
#else
        assert(0);
#endif
    }
    else if (gfx_desc.shaderType == kShaderTypeSingleRGB10A2)
    {
        colorAttachment0.pixelFormat = MTLPixelFormatRGB10A2Unorm ;
    }
	else if (gfx_desc.shaderType == kShaderTypeSingleRG8)
	{
		colorAttachment0.pixelFormat = MTLPixelFormatRG8Unorm ;
	}
	else if (gfx_desc.shaderType == kShaderTypeSingleR8)
	{
		colorAttachment0.pixelFormat = MTLPixelFormatR8Unorm ;
	}
    else if (gfx_desc.shaderType == kShaderTypeNoColorAttachment)
    {
        colorAttachment0.blendingEnabled = NO;
        colorAttachment0.pixelFormat = MTLPixelFormatInvalid ;
    }
    else if (gfx_desc.shaderType == kShaderTypeManhattanGBuffer)
    {
        desc.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA8Unorm;
        desc.colorAttachments[1].pixelFormat = MTLPixelFormatRGBA8Unorm;
        desc.colorAttachments[2].pixelFormat = MTLPixelFormatRGBA8Unorm;
        desc.colorAttachments[3].pixelFormat = MTLPixelFormatRGBA8Unorm;
    }
#ifdef OPT_TEST_GFX40
	else if (gfx_desc.shaderType == kShaderTypeCarChaseShadow)
	{
		colorAttachment0.blendingEnabled = NO;
		colorAttachment0.pixelFormat = MTLPixelFormatInvalid ;
		desc.depthAttachmentPixelFormat = MTL_Scene_40::GetShadowMapFormat();
	}
	else if (gfx_desc.shaderType == kShaderTypeCarChaseGBuffer)
	{
		desc.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA8Unorm;
		desc.colorAttachments[1].pixelFormat = GFXB4::GBuffer::VELOCITY_BUFFER_RGBA8 ? MTLPixelFormatRGBA8Unorm : MTLPixelFormatRGB10A2Unorm;
		desc.colorAttachments[2].pixelFormat = GFXB4::GBuffer::NORMAL_BUFFER_RGBA8 ? MTLPixelFormatRGBA8Unorm : MTLPixelFormatRGB10A2Unorm;
		desc.colorAttachments[3].pixelFormat = MTLPixelFormatRGBA8Unorm;
	}
#endif
    else // Unknown shader type
    {
        assert(0) ;
    }
	
	//only blending for first framebuffer?
	//TODO different pixelformats depending on material
	NSError* err = nil;
    id <MTLRenderPipelineState> pipe = [MetalRender::GetContext()->getDevice() newRenderPipelineStateWithDescriptor:desc error:&err];
	if(!pipe)
	{
		NSLog(@"Error creating pipeline %@", [err localizedDescription]);
	}
	assert(pipe);
	
	Pipeline* p = new Pipeline();
	p->m_pipeline = pipe;
	p->m_vertexLibrary = vs.shaderLibrary;
	p->m_fragmentLibrary = fs.shaderLibrary;
    
    p->m_vs_name = vsname ;
    p->m_fs_name = fsname ;
    p->m_defines = defines;
	
	#ifdef DEBUG
	p->defines = [NSString stringWithUTF8String:defines.c_str()];
	#endif
	
#if PIPELINE_CACHE_ENABLED
	s_pipelineCache[pipelineHash] = p;
#else
    s_pipelineCache[++g_cache_id] = p;
#endif
	return p;
}

Pipeline* Pipeline::CreatePipeline(Shader* shader,
                                const GFXPipelineDescriptor &gfx_desc,
                                KCL::KCL_Status& error)
{
    std::set<std::string> shader_defines = shader->getDefines() ;
    return CreatePipeline(shader->getVsFile().c_str(), shader->getFsFile().c_str(), &shader_defines, gfx_desc, error) ;
}

Pipeline* Pipeline::CreatePipelineFromLibrary(	const char* fileName,
												BlendType blendType,
												uint32_t renderTargetCount,
												bool hasDepth,
												bool depthWritesEnabled,
                                                KCL::KCL_Status& err)
{
	NSString* libSource;
			
	KCL::AssetFile library_file(fileName);
	
	if(library_file.GetLastError())
	{
		INFO("ERROR: library %s not found!\n", fileName);
		err = KCL::KCL_TESTERROR_FILE_NOT_FOUND;
		return NULL;
	}
	
	libSource = [NSString stringWithUTF8String:library_file.GetBuffer()];
	
	NSError* error = nil;
	id <MTLLibrary> lib = [MetalRender::GetContext()->getDevice() newLibraryWithSource:libSource options:nil error:&error];
	if(lib == nil)
	{
		NSLog(@"Shader lib failed to build:\n%@", error);
		err = KCL::KCL_TESTERROR_SHADER_ERROR;
		return NULL;
	}
	assert(!error);
	id <MTLFunction> vsFunction = [lib newFunctionWithName:@"vertex_main"];
	assert(vsFunction);
	id <MTLFunction> fsFunction = [lib newFunctionWithName:@"fragment_main"];
	assert(fsFunction);
	
	MTLRenderPipelineDescriptor* desc = [MTLRenderPipelineDescriptor new];
	desc.vertexFunction = vsFunction;
	desc.fragmentFunction = fsFunction;
    
	
	if(hasDepth)
	{
		desc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
	}
    else
    {
        desc.depthAttachmentPixelFormat = MTLPixelFormatInvalid ;
    }
    
    MTLRenderPipelineColorAttachmentDescriptor* colorAttachment0 = desc.colorAttachments[0];
    
    SetupColorAttachment(colorAttachment0, blendType);
	colorAttachment0.pixelFormat = MTLPixelFormatRGBA8Unorm;
	
	id <MTLRenderPipelineState> akp = [MetalRender::GetContext()->getDevice() newRenderPipelineStateWithDescriptor:desc error:&error];
	assert(akp);
	if(!akp)
	{
		NSLog(@"%@", [error localizedDescription]);
		err = KCL::KCL_TESTERROR_SHADER_ERROR;
		return NULL;
	}
	
	Pipeline* p = new Pipeline();
	p->m_pipeline = akp;
	p->m_vertexLibrary = lib;
	p->m_fragmentLibrary = nil;
	
	releaseObj(desc);
	
	err = KCL::KCL_TESTERROR_NOERROR;
	
	return p;
}


void Pipeline::ClearCashes()
{
    @autoreleasepool {
        
    for (auto it = MetalRender::Pipeline::s_pipelineCache.begin(); it != MetalRender::Pipeline::s_pipelineCache.end(); it++)
    {
        delete it->second ;
    }
    MetalRender::Pipeline::s_pipelineCache.clear();
    
    
    for (auto it = MetalRender::Pipeline::s_shaderCache.begin(); it != MetalRender::Pipeline::s_shaderCache.end(); it++)
    {
        id <MTLFunction> shader = (__bridge_transfer id <MTLFunction>) (it->second) ;
        releaseObj( shader );
    }
    MetalRender::Pipeline::s_shaderCache.clear();
        
        for (auto it = MetalRender::Pipeline::s_computePipeLines.begin(); it != MetalRender::Pipeline::s_computePipeLines.end();it++)
        {
           delete *it ;
        }
        MetalRender::Pipeline::s_computePipeLines.clear();
        
        s_shader_dirs.clear() ;
    }
}


MetalRender::ShaderType Pipeline::PixelFormatToShaderType(MTLPixelFormat pixelformat)
{
    switch (pixelformat) {
        case MTLPixelFormatRGBA8Unorm:
            return kShaderTypeSingleRGBA8Default ;
            
#if TARGET_OS_EMBEDDED
        case MTLPixelFormatB5G6R5Unorm:
            return kShaderTypeSingleBGR565 ;
#endif
            
        case MTLPixelFormatBGRA8Unorm:
            return kShaderTypeSingleBGRA8 ;
            
        case MTLPixelFormatRGB10A2Unorm:
            return kShaderTypeSingleRGB10A2 ;

		case MTLPixelFormatRG8Unorm:
			return kShaderTypeSingleRG8 ;

		case MTLPixelFormatR8Unorm:
			return kShaderTypeSingleR8 ;
            
        default:
            // unknow shadertype
            assert(0) ;
            return kShaderTypeUnknown ;
    }
}

