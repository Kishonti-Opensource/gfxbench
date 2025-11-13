/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "offscrman.h"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cassert>

#include "fbo.h"
#import "fbo_metal.h"

#include <kcl_os.h>
//#include "misc2.h"
#include "test_descriptor.h"

#include "kcl_base.h"
#include "kcl_io.h"

#import "graphics/metalgraphicscontext.h"
#import "mtl_shader_helper.h"

#include "gfxbench/global_test_environment.h"

#define SAFE_DELETE(x)	if (NULL != (x)) { delete (x); (x) = NULL; }


using namespace GLB;
using namespace KCL;

#include "mtl_globals.h"
#include <Metal/Metal.h>

#include "gfxbench/gfxbench.h"



#define WAIT_FOR_FINISH 0



class OffscreenManager_Metal : public OffscreenManager
{
public:
    OffscreenManager_Metal(const GlobalTestEnvironment* const gte, int w, int h) ;
    virtual ~OffscreenManager_Metal() ;
    
    virtual int Init( unsigned int onscreen_width, unsigned int onscreen_height, const TestDescriptor &td) ;
    virtual void PostRender(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer) ;
    virtual void Clear() ;
    virtual void ResetGlobalFBO(int current_viewport_width, int current_viewport_height) ;
    virtual void RenderLastFrames(int current_viewport_width, int current_viewport_height);
    
protected:
    
    void InitCommon();
    void deleteOffscrProg();
    
    virtual void renderRectWithTexture(GLB::FBO* fbo, TextureCoordinatesMode textureCoordinatesMode, bool isRotated) ; // use only in PostRender!!!
    
    void renderToScratch() ;		        // use only in PostRender!!!
    void renderScratchToMosaic() ;	        // use only in PostRender!!!
    void renderScratchToBackScreen() ;      // use only in PostRender!!!
    
    void renderToOnscrMosaic();				                   // use only in PostRender!!!
    void renderOnscrMosaicToBackScreen(const bool isRotated) ; // use only in PostRender!!!
    void renderOffscrToBackScreen(const bool isRotated) ;
    
    void setMosaicViewPort() ;
    void setOnscrSampleViewPort() ;
    void setOnscrMosaicViewPort(const bool isRotated) ;
    
    void MetalViewport(double originX, double originY, double width, double height ) ;
    void MetalFinish() ;
    
    virtual void PostRender_original(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer);
    
    id <MTLLibrary> m_offscreen_vs_library ;
    id <MTLLibrary> m_offscreen_fs_library ;
    id <MTLDevice>  m_device ;
    id <MTLCommandQueue>  m_CommandQueue ;
    
    id <MTLFunction> m_vert1 ;
    id <MTLFunction> m_vert2 ;
    id <MTLFunction> m_frag  ;
    
    id <MTLBuffer> m_offscreen_vbo ;
    id <MTLBuffer> m_offscreen_ibo ;
    
    MTLViewport m_view_port ;
    
    bool m_clear_needed ;
    
    MTLRenderPassDescriptor* mp_postRenderPassDescriptor ;
    id <MTLDepthStencilState> m_DepthState ;
    
    id <MTLRenderPipelineState> m_PipelineState_Vert1 ;
    id <MTLRenderPipelineState> m_PipelineState_Vert2 ;
};


OffscreenManager* OffscreenManager::Create(const GlobalTestEnvironment* const gte, int w, int h)
{
	return new OffscreenManager_Metal(gte, w, h);
}


//
//
//  OffscreenManager constructor / destructor
//
//

OffscreenManager::OffscreenManager(const GlobalTestEnvironment* const gte, int w, int h) : m_gte(gte), m_offscreen_default_viewport_width( w), m_offscreen_default_viewport_height( h)
{
	srand( time(0));

	memset(m_sample_times, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_sample_coords_x, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_sample_coords_y, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_mosaic_coords_x, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_mosaic_coords_y, 0, sizeof(KCL::uint32)*SAMPLE_C);

	m_width = 0;
	m_height = 0;
	m_globalFBO = 0;

    m_virtual_resolution = false;

    m_onscreen_width = 0;
    m_onscreen_height = 0;
	
    for (int i = 0 ; i < OFFSCREEN_FBO_COUNT ; i++)
    {
        m_offscreen_fbo[i] = 0;
        m_scratch_fbo[i] = 0;
    }

	m_current_fbo_index = 0;
	m_mosaic_fbo = 0;

	m_onscr_mosaic_fbo = 0;
	m_onscr_mosaic_idx = 0;
	init_onscr_mosaic_coords();
	
	m_next_slot_time_interval = 0;
	m_next_slot_previous_time = 0;
	m_last_refresh_msec = 0;
	m_mosaic_idx = 0;
}


void OffscreenManager::init_onscr_mosaic_coords()
{
	for(KCL::uint32 i=0; i<ONSCR_SAMPLE_C; ++i)
	{
		m_onscr_mosaic_coords_x[i] = (i % ONSCR_SAMPLE_NUM_X) * ONSCR_SAMPLE_W;
		m_onscr_mosaic_coords_y[i] = (i / ONSCR_SAMPLE_NUM_X) * ONSCR_SAMPLE_H;
	}
}


OffscreenManager::~OffscreenManager()
{

}


//
//
//  OffscreenManager_Metal constructor / destructor
//
//


OffscreenManager_Metal::OffscreenManager_Metal(const GlobalTestEnvironment* const gte, int w, int h) : OffscreenManager(gte,w,h)
{
    m_offscreen_vs_library = nil ;
    m_offscreen_fs_library = nil ;
    m_device = nil;
    m_CommandQueue = nil;
    
    m_vert1 = nil;
    m_vert2 = nil;
    m_frag  = nil;
    
    m_offscreen_vbo = nil ;
    m_offscreen_ibo = nil ;
    
    m_clear_needed = false  ;
    
    mp_postRenderPassDescriptor = nil ;
    
    m_DepthState = nil ;
    
    m_PipelineState_Vert1 = nil ;
    m_PipelineState_Vert2 = nil ;
    
    MetalGraphicsContext* metal_context = dynamic_cast<MetalGraphicsContext*>(gte->GetGraphicsContext()) ;
    
    m_device = metal_context->getDevice() ;
    m_CommandQueue = metal_context->getMainCommandQueue() ;
}


OffscreenManager_Metal::~OffscreenManager_Metal()
{
    Clear() ;
}


void OffscreenManager_Metal::MetalViewport(double originX, double originY, double width, double height )
{
    m_view_port.originX = originX ;
    m_view_port.originY = originY ;
    m_view_port.width   = width ;
    m_view_port.height  = height ;
    m_view_port.znear   = 0.0 ;
    m_view_port.zfar    = 1.0 ;
}


void OffscreenManager_Metal::MetalFinish()
{
#if WAIT_FOR_FINISH
    assert(0) ;
    @autoreleasepool {
        id <MTLCommandBuffer> finishCommandBuffer = [m_CommandQueue commandBuffer];
		finishCommandBuffer.label = @"OffScrMan Finish Buffer";
        [finishCommandBuffer commit] ;
        [finishCommandBuffer waitUntilCompleted] ;
        releaseObj(finishCommandBuffer) ;
    }
#endif
}


void OffscreenManager_Metal::InitCommon()
{
    if( (m_offscreen_vs_library != nil) && (m_offscreen_fs_library != nil))
	{
		return;
	}
    
    std::string vs_shader_source = "#define TYPE_VERTEX 1\n" ;
    std::string fs_shader_source = "#define TYPE_FRAGMENT 1\n" ;
    
    bool force_highp = false ;
#if !TARGET_OS_IPHONE
    force_highp = true;
#endif
    if (force_highp)
    {
        vs_shader_source += "#define FORCE_HIGHP 1\n" ;
        vs_shader_source += "#define VARYING_HIGHP 1\n" ;
        
        fs_shader_source += "#define FORCE_HIGHP 1\n" ;
        fs_shader_source += "#define VARYING_HIGHP 1\n" ;
    }
    else
    {
        vs_shader_source += "#define FORCE_HIGHP 1\n" ;
        vs_shader_source += "#define VARYING_HIGHP 0\n" ;
        
        fs_shader_source += "#define FORCE_HIGHP 0\n" ;
        fs_shader_source += "#define VARYING_HIGHP 0\n" ;
    }

    
    KCL::AssetFile float_header_file("shaders_mtl/common/float_header.h") ;
    KCL::AssetFile shader_source_file("shaders_mtl/common/offscrman.metal") ;
    
    vs_shader_source += float_header_file.GetBuffer() ;
    vs_shader_source += shader_source_file.GetBuffer() ;
    m_offscreen_vs_library  = MetalRender::LoadShaderLibraryFromString(m_device,vs_shader_source) ;
    
    assert(m_offscreen_vs_library != nil) ;
    
    fs_shader_source += float_header_file.GetBuffer() ;
    fs_shader_source += shader_source_file.GetBuffer() ;
    m_offscreen_fs_library  = MetalRender::LoadShaderLibraryFromString(m_device,fs_shader_source) ;
    
    assert(m_offscreen_fs_library != nil) ;
    
    if (!m_virtual_resolution)
    {
        m_vert1 = [m_offscreen_vs_library newFunctionWithName:@"vert1"];
        m_vert2 = [m_offscreen_vs_library newFunctionWithName:@"vert2"];
    }
    else
    {
        m_vert1 = [m_offscreen_vs_library newFunctionWithName:@"virtres_vert1"];
        m_vert2 = [m_offscreen_vs_library newFunctionWithName:@"virtres_vert2"];
    }
    m_frag  = [m_offscreen_fs_library newFunctionWithName:@"frag"];

	
	//glGenBuffers(1, &m_offscreen_vbo);
    //glGenBuffers(1, &m_offscreen_ebo);

	float* vertices_texcoords = new float[24+8*SAMPLE_C];
	vertices_texcoords[0] = -1.0f; //0
	vertices_texcoords[1] = -1.0f; //0

	vertices_texcoords[2] =  1.0f; //1
	vertices_texcoords[3] = -1.0f; //1

	vertices_texcoords[4] =  1.0f; //2
	vertices_texcoords[5] =  1.0f; //2

	vertices_texcoords[6] = -1.0f; //3
	vertices_texcoords[7] =  1.0f; //3

	float X_L, Y_D, X_R, Y_U;
	for(KCL::uint32 i=1; i<=SAMPLE_C; ++i)
	{
		calcSampleTexCoords(i-1, X_L, Y_D, X_R, Y_U);

		vertices_texcoords[i*8    ] = X_L;
		vertices_texcoords[i*8 + 1] = Y_D;

		vertices_texcoords[i*8 + 2] = X_R;
		vertices_texcoords[i*8 + 3] = Y_D;
		
		vertices_texcoords[i*8 + 4] = X_R;
		vertices_texcoords[i*8 + 5] = Y_U;
		
		vertices_texcoords[i*8 + 6] = X_L;
		vertices_texcoords[i*8 + 7] = Y_U;
		
		KCL::uint32 idx = i-1;
		m_mosaic_coords_x[idx] = (idx % SAMPLE_NUM_X) * SAMPLE_W;
		m_mosaic_coords_y[idx] = (idx / SAMPLE_NUM_X) * SAMPLE_H;
	}

	X_L = 0;
	Y_D = 0;
	X_R = X_L + (float)SAMPLE_W / (float)SCRATCH_WIDTH;
	Y_U = Y_D + (float)SAMPLE_H / (float)SCRATCH_HEIGHT;

	//texture coordinate for scratch
	vertices_texcoords[8+8*SAMPLE_C    ] = X_L;
	vertices_texcoords[8+8*SAMPLE_C + 1] = Y_D;

	vertices_texcoords[8+8*SAMPLE_C + 2] = X_R;
	vertices_texcoords[8+8*SAMPLE_C + 3] = Y_D;

	vertices_texcoords[8+8*SAMPLE_C + 4] = X_R;
	vertices_texcoords[8+8*SAMPLE_C + 5] = Y_U;

	vertices_texcoords[8+8*SAMPLE_C + 6] = X_L;
	vertices_texcoords[8+8*SAMPLE_C + 7] = Y_U;


	//texture coordinate for full view
	vertices_texcoords[16+8*SAMPLE_C    ] =  0.0f; //0
	vertices_texcoords[16+8*SAMPLE_C + 1] =  0.0f; //0

	vertices_texcoords[16+8*SAMPLE_C + 2] =  1.0f; //1
	vertices_texcoords[16+8*SAMPLE_C + 3] =  0.0f; //1

	vertices_texcoords[16+8*SAMPLE_C + 4] =  1.0f; //2
	vertices_texcoords[16+8*SAMPLE_C + 5] =  1.0f; //2

	vertices_texcoords[16+8*SAMPLE_C + 6] =  0.0f; //3
	vertices_texcoords[16+8*SAMPLE_C + 7] =  1.0f; //3

    
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (24+8*SAMPLE_C), (const void*)vertices_texcoords, GL_STATIC_DRAW);
    
    KCL::uint32 vertex_buffer_size = sizeof(float) * (24+8*SAMPLE_C) ;
#if TARGET_OS_EMBEDDED
    m_offscreen_vbo = [m_device newBufferWithLength:vertex_buffer_size
                                               options:0];
    m_offscreen_vbo.label = @"VertexBuffer" ;
    
    
    float *pVertices = (float *)[m_offscreen_vbo contents];
    memcpy(pVertices, vertices_texcoords, vertex_buffer_size);
#else
    m_offscreen_vbo = [m_device newBufferWithBytes:vertices_texcoords length:vertex_buffer_size options:MTLResourceStorageModeManaged];
    m_offscreen_vbo.label = @"VertexBuffer" ;
#endif

    
    
	delete[] vertices_texcoords;

	KCL::uint32 indicesTmp[] =
	{
		0, 1, 2,
		0, 2, 3
	};
    
    
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(KCL::uint8) * 6, (const void*)indicesTmp, GL_STATIC_DRAW);
    
    KCL::uint32 index_buffer_size = sizeof(KCL::uint32) * 6 ;
#if TARGET_OS_EMBEDDED
    m_offscreen_ibo = [m_device newBufferWithLength:index_buffer_size
                                              options:0];
    m_offscreen_ibo.label = @"IndexBuffer" ;
    
    
    KCL::uint32 *pIndices = (KCL::uint32 *)[m_offscreen_ibo contents];
    memcpy(pIndices, indicesTmp, index_buffer_size);
#else
    m_offscreen_ibo = [m_device newBufferWithBytes:indicesTmp length:index_buffer_size options:MTLStorageModeManaged];
    m_offscreen_ibo.label = @"IndexBuffer" ;
#endif
    
	m_globalFBO = FBO::GetGlobalFBO();
    
    
    //
    //  RenderPassDescriptor
    //
    
    MTLClearColor ClearColor = MTLClearColorMake(0.0f, 0.0f , 0.0f , 1.0f);
    
    mp_postRenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor] ;
    mp_postRenderPassDescriptor.colorAttachments[0].clearColor = ClearColor ;
    
    mp_postRenderPassDescriptor.depthAttachment.texture = nil ;
    
    
    //
    //  DepthState
    //
    
    MTLDepthStencilDescriptor *pDepthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    
    pDepthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
    pDepthStateDesc.depthWriteEnabled    = NO;
    
    m_DepthState = [m_device newDepthStencilStateWithDescriptor:pDepthStateDesc];
    releaseObj(pDepthStateDesc) ;
    
    
    //
    //  Vertex Descriptor
    //
    
    NSUInteger interleavedBufferStride = 2 * sizeof(float);
    MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];
    
    vertexDesc.attributes[0].format = MTLVertexFormatFloat2;
    vertexDesc.attributes[0].bufferIndex = 0;
    vertexDesc.attributes[0].offset = 0;
    
    vertexDesc.attributes[1].format = MTLVertexFormatFloat2;
    vertexDesc.attributes[1].bufferIndex = 1;
    vertexDesc.attributes[1].offset = 0;
    
    vertexDesc.layouts[0].stride = interleavedBufferStride ;
    vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    
    vertexDesc.layouts[1].stride = interleavedBufferStride ;
    vertexDesc.layouts[1].stepFunction = MTLVertexStepFunctionPerVertex;
    
    
    //
    //  PipeLine State
    //
    MTLPixelFormat defaultPixelFormat = MTLPixelFormatBGRA8Unorm ;
    
    NSError *pError = nil;
    
    MTLRenderPipelineDescriptor *pPipelineStateDescriptor_Vert1 = [[MTLRenderPipelineDescriptor alloc] init];
    
    pPipelineStateDescriptor_Vert1.colorAttachments[0].pixelFormat = defaultPixelFormat;
    pPipelineStateDescriptor_Vert1.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
    [pPipelineStateDescriptor_Vert1 setVertexFunction:m_vert1];
    [pPipelineStateDescriptor_Vert1 setFragmentFunction:m_frag];
    [pPipelineStateDescriptor_Vert1 setVertexDescriptor:vertexDesc] ;
    
    m_PipelineState_Vert1 = [m_device newRenderPipelineStateWithDescriptor:pPipelineStateDescriptor_Vert1
                                                                    error:&pError] ;
    
    assert(m_PipelineState_Vert1) ;
    
    releaseObj(pPipelineStateDescriptor_Vert1) ;
    
    
    MTLRenderPipelineDescriptor *pPipelineStateDescriptor_Vert2 = [[MTLRenderPipelineDescriptor alloc] init];
    
    pPipelineStateDescriptor_Vert2.colorAttachments[0].pixelFormat = defaultPixelFormat;
    pPipelineStateDescriptor_Vert2.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
    [pPipelineStateDescriptor_Vert2 setVertexFunction:m_vert2];
    [pPipelineStateDescriptor_Vert2 setFragmentFunction:m_frag];
    [pPipelineStateDescriptor_Vert2 setVertexDescriptor:vertexDesc] ;
    
    m_PipelineState_Vert2 = [m_device newRenderPipelineStateWithDescriptor:pPipelineStateDescriptor_Vert2
                                                                     error:&pError] ;
    
    assert(m_PipelineState_Vert2) ;

    releaseObj(pPipelineStateDescriptor_Vert2) ;
    releaseObj(vertexDesc) ;
}


void OffscreenManager_Metal::deleteOffscrProg()
{
	releaseObj(m_offscreen_vs_library) ;
    releaseObj(m_offscreen_fs_library) ;
    releaseObj(m_vert1) ;
    releaseObj(m_vert2) ;
    releaseObj(m_frag) ;
    
    
    releaseObj(m_offscreen_vbo) ;
    releaseObj(m_offscreen_ibo) ;
	
    for (int i = 0 ; i < OFFSCREEN_FBO_COUNT ; i++)
    {
        SAFE_DELETE(m_offscreen_fbo[i]);
        SAFE_DELETE(m_scratch_fbo[i]);
    }
    
	SAFE_DELETE(m_mosaic_fbo);
	SAFE_DELETE(m_onscr_mosaic_fbo);

	m_onscr_mosaic_idx = 0;
}

void OffscreenManager_Metal::renderRectWithTexture(GLB::FBO* fbo, TextureCoordinatesMode textureCoordinatesMode, bool isRotated)
{
    @autoreleasepool {
       
        
        //draw with the texture
        //glBindTexture(GL_TEXTURE_2D, textureId);
        
        id <MTLCommandBuffer> commandBuffer = [m_CommandQueue commandBuffer];
		commandBuffer.label = @"OffScrMan Render Rect";

        
        //
        //  RenderPassDescriptor
        //
        
        mp_postRenderPassDescriptor.colorAttachments[0].texture = dynamic_cast<FBOMetalBase*>(FBO::GetLastBind())->GetTexture() ;
        mp_postRenderPassDescriptor.colorAttachments[0].loadAction = (m_clear_needed)?MTLLoadActionClear:MTLLoadActionLoad;
        
        
        size_t offset = 0;
        
        switch ( textureCoordinatesMode)
        {
            case OffscreenManager::USE_MOSAIC:
                offset = (m_mosaic_idx + 1) * 8 * sizeof(float);
                break;
            case OffscreenManager::USE_SCRATCH:
                offset = (8 * SAMPLE_C + 8) * sizeof(float);
                break;
            case OffscreenManager::USE_FULL:
                offset = (8 * SAMPLE_C + 16) * sizeof(float);
                break;
            case OffscreenManager::USE_FULL_NEAREST:
            {
                assert(false) ;
                
                //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                //offset = (8 * SAMPLE_C + 16) * sizeof(float);
                //break;
            }
        }
        
        
        
        
        // Get a render encoder
        id <MTLRenderCommandEncoder>  renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:mp_postRenderPassDescriptor];
        
        [renderEncoder setViewport:m_view_port] ;
        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        [renderEncoder setDepthStencilState:m_DepthState] ;
        
        if (!isRotated)
        {
            [renderEncoder setRenderPipelineState:m_PipelineState_Vert1] ;
        }
        else
        {
            [renderEncoder setRenderPipelineState:m_PipelineState_Vert2] ;

        }

        //glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, (const void*)offset);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
        
        [renderEncoder setVertexBuffer:m_offscreen_vbo
                                offset:0
                               atIndex:0 ];
        
        [renderEncoder setVertexBuffer:m_offscreen_vbo
                                offset:offset
                               atIndex:1 ];
        
        FBOMetalBase* metal_fbo = dynamic_cast<FBOMetalBase*>(fbo) ;

        [renderEncoder setFragmentTexture:metal_fbo->GetTexture()
                                  atIndex:0];

        
        // tell the render context we want to draw our primitives
        [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                  indexCount:6
                                   indexType:MTLIndexTypeUInt32
                                 indexBuffer:m_offscreen_ibo
                           indexBufferOffset:0];
        
        
        
        
        [renderEncoder endEncoding] ;
        [commandBuffer commit] ;
        
        releaseObj(renderEncoder) ;
        releaseObj(commandBuffer) ;
        
        m_clear_needed = false  ;
    }
}


void OffscreenManager::calcSampleTexCoords(size_t idx, float &X_L, float &Y_D, float &X_R, float &Y_U)
{
	KCL::uint32 xui = rand() % (m_width - SAMPLE_W);
	KCL::uint32 yui = rand() % (m_height - SAMPLE_H);

	X_L = (float)xui / (float)m_width;
	Y_D = (float)yui / (float)m_height;
	X_R = X_L + (float)SAMPLE_W / m_width;
	Y_U = Y_D + (float)SAMPLE_H / m_height;

	m_sample_coords_x[idx] = xui;
	m_sample_coords_y[idx] = yui;
}


void OffscreenManager::clear_saved_sample_data()
{
	memset(m_sample_times, 0, sizeof(KCL::uint32)*SAMPLE_C);	
	memset(m_sample_coords_x, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_sample_coords_y, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_mosaic_coords_x, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_mosaic_coords_y, 0, sizeof(KCL::uint32)*SAMPLE_C);
	m_mosaic_idx = 0;
}


void OffscreenManager_Metal::renderToScratch()
{
	FBO::bind( m_scratch_fbo[m_current_fbo_index] );
#ifndef STRIP_REDUNDANT_CLEARS
	//glClear(GL_COLOR_BUFFER_BIT);
    
    m_clear_needed = true ;
#endif
	MetalViewport( 0, 0, SAMPLE_W, SAMPLE_H);

	renderRectWithTexture(m_offscreen_fbo[m_current_fbo_index], USE_MOSAIC,false);
}


void OffscreenManager_Metal::renderScratchToMosaic()
{
	FBO::bind( m_mosaic_fbo );
	
	setMosaicViewPort();

	renderRectWithTexture(m_scratch_fbo[m_current_fbo_index], USE_SCRATCH,false);
}


void OffscreenManager_Metal::renderScratchToBackScreen()
{
	FBO::bind( 0 );
			
	MetalViewport( 0, 0, SAMPLE_W, SAMPLE_H);

	
    renderRectWithTexture(m_scratch_fbo[m_current_fbo_index], USE_SCRATCH, false);
}


void OffscreenManager_Metal::renderToOnscrMosaic()
{
	m_onscr_mosaic_idx %= ONSCR_SAMPLE_C;

	FBO::bind( m_onscr_mosaic_fbo );
	setOnscrSampleViewPort();
	++m_onscr_mosaic_idx;

	renderRectWithTexture(m_offscreen_fbo[m_current_fbo_index], USE_FULL, false);
}


void OffscreenManager_Metal::renderOnscrMosaicToBackScreen(const bool isRotated)
{
	//OpenGLStateManager::GlUseProgram(m_onscr_mosaic_program[isRotated]);

	FBO::bind( 0 );
#ifndef STRIP_REDUNDANT_CLEARS
	//glClearColor(0,0,0,0);
	//glClear(GL_COLOR_BUFFER_BIT);
    
    m_clear_needed = true ;
#endif

	setOnscrMosaicViewPort(isRotated);

	renderRectWithTexture(m_onscr_mosaic_fbo, USE_FULL,isRotated);
}

void OffscreenManager_Metal::renderOffscrToBackScreen(const bool isRotated)
{
    //OpenGLStateManager::GlUseProgram(m_onscr_mosaic_program[isRotated]);

	FBO::bind( 0 );
#ifndef STRIP_REDUNDANT_CLEARS
	//glClearColor(0,0,0,0);
	//glClear(GL_COLOR_BUFFER_BIT);
    
    m_clear_needed = true ;
#endif

	if(false)
    {
        MetalViewport( 0, 0, m_onscreen_height, m_onscreen_width);
    }
    else
    {
        MetalViewport( 0, 0, m_onscreen_width, m_onscreen_height);
    }

    renderRectWithTexture(m_offscreen_fbo[m_current_fbo_index], USE_FULL,isRotated);
}

void OffscreenManager_Metal::setOnscrSampleViewPort()
{
    MetalViewport(m_onscr_mosaic_coords_x[m_onscr_mosaic_idx], m_onscr_mosaic_coords_y[m_onscr_mosaic_idx],
                  ONSCR_SAMPLE_W, ONSCR_SAMPLE_H) ;
}


void OffscreenManager_Metal::setOnscrMosaicViewPort(const bool isRotated)
{
    MetalViewport(m_onscr_mosaic_x[isRotated], m_onscr_mosaic_y[isRotated],
                  m_onscr_mosaic_viewport_width[isRotated], m_onscr_mosaic_viewport_height[isRotated]) ;
}


int OffscreenManager_Metal::Init( unsigned int onscreen_width, unsigned int onscreen_height, const TestDescriptor &td)
{
	FBO_COLORMODE color_mode;
	FBO_DEPTHMODE depth_mode;

	m_refresh_msec = td.m_hybrid_refresh_msec;

    m_virtual_resolution = td.m_virtual_resolution && (td.GetScreenMode() == SMode_Onscreen);
    m_method = ( (SMode_Offscreen == td.GetScreenMode() || m_virtual_resolution) ? OM_ORIGINAL : OM_HYBRID);

	int w = td.m_viewport_width;
	int h = td.m_viewport_height;


	int max_texture_dim = 4096; // hardcoded from documentation v1_1, v2_1
	
#if TARGET_OS_IOS
	if ([m_device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1])
	{
		max_texture_dim = 16384;
	}
	else if ([m_device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v2])
	{
		max_texture_dim = 8192;
	}
	else if ([m_device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v2])
	{
		max_texture_dim = 8192;
	}
#elif TARGET_OS_OSX
	if ([m_device supportsFeatureSet:MTLFeatureSet_OSX_GPUFamily1_v1])
	{
		max_texture_dim = 16384;
	}
#endif // TARGET_OS_IOS / TARGET_OS_OSX

    
	INFO("Max texture size: %d" , max_texture_dim);

	if( w > max_texture_dim || h > max_texture_dim)
	{
		return KCL_TESTERROR_INVALID_SCREEN_RESOLUTION;
	}

	if( m_offscreen_default_viewport_width > max_texture_dim || m_offscreen_default_viewport_height > max_texture_dim)
	{
		return KCL_TESTERROR_INVALID_SCREEN_RESOLUTION;
	}
	

	if( w < 1 || h < 1)
	{
		return KCL_TESTERROR_INVALID_SCREEN_RESOLUTION;
	}

	if( td.m_play_time < 1)
	{
		assert(0);
	}
	m_width = w;
	m_height = h;

    m_onscreen_width = onscreen_width;
    m_onscreen_height = onscreen_height;

	m_onscr_mosaic_viewport_width [0] = ONSCR_MOSAIC_WIDTH;
	m_onscr_mosaic_viewport_height[0] = ONSCR_MOSAIC_HEIGHT;

	m_onscr_mosaic_viewport_width [1] = ONSCR_MOSAIC_HEIGHT;
	m_onscr_mosaic_viewport_height[1] = ONSCR_MOSAIC_WIDTH;
	
	m_onscr_mosaic_x[0] = onscreen_width  > m_onscr_mosaic_viewport_width [0] ? (onscreen_width  - m_onscr_mosaic_viewport_width [0]) / 2 : 0;
	m_onscr_mosaic_y[0] = onscreen_height > m_onscr_mosaic_viewport_height[0] ? (onscreen_height - m_onscr_mosaic_viewport_height[0]) / 2 : 0;
	
	m_onscr_mosaic_x[1] = onscreen_width  > m_onscr_mosaic_viewport_width [1] ? (onscreen_width  - m_onscr_mosaic_viewport_width [1]) / 2 : 0;
	m_onscr_mosaic_y[1] = onscreen_height > m_onscr_mosaic_viewport_height[1] ? (onscreen_height - m_onscr_mosaic_viewport_height[1]) / 2 : 0;

    InitCommon() ;
	
	m_next_slot_time_interval = (KCL::uint32)td.m_play_time / (int)SAMPLE_C;
	if((KCL::uint32)td.m_play_time % (int)SAMPLE_C)
	{
		++m_next_slot_time_interval;
	}
	m_next_slot_previous_time = 0;
	
	m_globalFBO = FBO::GetGlobalFBO();

	if( td.m_color_bpp >= 24)
	{
		color_mode = RGB888_Linear;
	}
	else
	{
		color_mode = RGB565_Linear;
	}

	if( td.m_depth_bpp >= 24)
	{
		depth_mode = DEPTH_24_RB;
	}
	else
	{
		depth_mode = DEPTH_16_RB;
	}

	try
	{
        for (int i = 0 ; i < OFFSCREEN_FBO_COUNT ; i++)
        {
            m_offscreen_fbo[i] = FBO::CreateFBO(m_gte,m_width, m_height, td.m_fsaa, color_mode, depth_mode, "m_offscreen_fbo");
            m_scratch_fbo[i] = FBO::CreateFBO(m_gte,SCRATCH_WIDTH, SCRATCH_HEIGHT, td.m_fsaa, color_mode, DEPTH_None, "m_scratch_fbo");
        }
        
        
       
		m_mosaic_fbo = FBO::CreateFBO(m_gte,MOSAIC_WIDTH, MOSAIC_HEIGHT, td.m_fsaa, color_mode, DEPTH_None, "m_mosaic_fbo") ;
		m_onscr_mosaic_fbo = FBO::CreateFBO(m_gte,ONSCR_MOSAIC_WIDTH, ONSCR_MOSAIC_HEIGHT, td.m_fsaa, color_mode, DEPTH_None, "m_onscr_mosaic_fbo");
    }
	catch (...)
	{
        for (int i = 0 ; i < OFFSCREEN_FBO_COUNT ; i++)
        {
            SAFE_DELETE(m_offscreen_fbo[i]);
            SAFE_DELETE(m_scratch_fbo[i]);
        }
        
		SAFE_DELETE(m_mosaic_fbo);
		SAFE_DELETE(m_onscr_mosaic_fbo);

		if(td.m_fsaa > 0)
			return KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED_IN_MSAA;

		return KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED;
	}

	{
		bool isRotated = onscreen_width < onscreen_height;
		
		float vw = m_width;
		float vh = m_height;
		float w = !isRotated ? onscreen_width : onscreen_height;
		float h = !isRotated ? onscreen_height : onscreen_width;
		float var = vw / vh;
		float ar = w / h;

		if ( var > ar )
		{
			h = w / var;
		}
		else
		{
			w = var * h;
		}

		if (isRotated)
		{
			float tmp = w;
			w = h;
			h = tmp;
		}

		m_hybrid_onscreen_width = w;
		m_hybrid_onscreen_height = h;
	}

	FBO::SetGlobalFBO( m_globalFBO );
	return KCL_TESTERROR_NOERROR;
}


void OffscreenManager::PreRender() const
{
	FBO::SetGlobalFBO( m_offscreen_fbo[m_current_fbo_index]);
	FBO::bind( m_offscreen_fbo[m_current_fbo_index] );
}


void OffscreenManager_Metal::PostRender(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer)
{
	switch( m_method)
	{
	case OM_ORIGINAL:
		{
			PostRender_original( time, frame, current_viewport_width, current_viewport_height, force_swap_buffer);
			break;
		}
	case OM_HYBRID:
		{
			PostRender_hybrid( time, frame, current_viewport_width, current_viewport_height, force_swap_buffer);
			break;
		}
	}
}


void OffscreenManager_Metal::PostRender_original(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer)
{
	renderToScratch();

	if(time - m_next_slot_previous_time >= m_next_slot_time_interval || force_swap_buffer)
	{
		renderScratchToMosaic();
		save_sample_time(time);
		m_next_slot_previous_time = time;
		m_mosaic_idx = (m_mosaic_idx + 1) % COUNT_OF(m_sample_times);
        
        MetalFinish() ;
	}

	renderToOnscrMosaic();

	FBO::SetGlobalFBO( m_globalFBO );

	if(( frame % OFFSCR_RENDER_TIMER) == 0 || force_swap_buffer)
	{
		renderScratchToBackScreen();
        
        MetalFinish() ;
	}

    if(!m_virtual_resolution)
    {
	    if( IsSwapBufferNeeded() || force_swap_buffer)
	    {
		    renderOnscrMosaicToBackScreen(current_viewport_width < current_viewport_height);
            
            MetalFinish() ;
	    }
	    else
	    {
            // MTL_TODO
            //glFlush();
	    }
    }
    else
    {
        renderOffscrToBackScreen(current_viewport_width < current_viewport_height);
    }

	MetalViewport( 0, 0, current_viewport_width, current_viewport_height);
    
	m_current_fbo_index++;
    m_current_fbo_index %= OFFSCREEN_FBO_COUNT ;
}




void OffscreenManager::PostRender_hybrid(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer)
{
    INFO("Hybrid offscreen manager unimplemented in metal");
    assert(false) ;
}


void OffscreenManager_Metal::Clear()
{
    // wait for finish all GPU operation
    id <MTLCommandBuffer> finishCommandBuffer = [m_CommandQueue commandBuffer];
	finishCommandBuffer.label = @"OffScrMgr Destruction Buffer";
    [finishCommandBuffer commit] ;
    [finishCommandBuffer waitUntilCompleted] ;
    releaseObj(finishCommandBuffer) ;
    
    releaseObj(mp_postRenderPassDescriptor) ;
    releaseObj(m_DepthState) ;
    
    releaseObj(m_PipelineState_Vert1) ;
    releaseObj(m_PipelineState_Vert2) ;
    
    releaseObj(m_offscreen_vs_library) ;
    releaseObj(m_offscreen_fs_library) ;
    releaseObj(m_device) ;
    releaseObj(m_CommandQueue) ;
    
	deleteOffscrProg();
	clear_saved_sample_data();
}

void OffscreenManager_Metal::ResetGlobalFBO(int current_viewport_width, int current_viewport_height)
{
	FBO::SetGlobalFBO( m_globalFBO );
	FBO::bind( 0 );
    
	//glClear(GL_COLOR_BUFFER_BIT);
    m_clear_needed = true ;
	
    MetalViewport( 0, 0, current_viewport_width, current_viewport_height);
	//glFlush();
}


void OffscreenManager_Metal::setMosaicViewPort()
{
	MetalViewport(m_mosaic_coords_x[m_mosaic_idx], m_mosaic_coords_y[m_mosaic_idx], SAMPLE_W, SAMPLE_H);
}


void OffscreenManager::SaveForTesting() const
{
    INFO("SaveForTesting unimplemented in metal");
    assert(false) ;
}


void OffscreenManager_Metal::RenderLastFrames(int current_viewport_width, int current_viewport_height)
{
    FBO::SetGlobalFBO( m_globalFBO );
    renderOnscrMosaicToBackScreen(current_viewport_width < current_viewport_height);
}


