/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_low_level.h"
#include "mtl_low_level_base.h"


#include "kcl_math3d.h"
#import <simd/simd.h>

#import "../gfxbench/global_test_environment.h"

#include "fbo.h"
#include "fbo_metal.h"

#include "mtl_dynamic_data_buffer.h"

using namespace KCL ;

const int BufferCount = 4 ;

const int ColumnCount = 50;
const int RowCount = 50;


//
//
//  Vertex Uniform data structure
//  MTL_TODO
//
//

struct VertexUniforms
{
    float        u_rotation;
    simd::float2 u_position;
    simd::float2 u_scale;
    simd::float2 u_matrixSize;
    
    simd::float4 u_color0;
    simd::float4 u_color1;
    simd::float4 u_color2;
    simd::float4 u_color3;
    
    simd::float2 u_screenResolution;
};


//
//
//  Test Implementation
//
//


class CPUOverheadTest_Metal : public CPUOverheadTest_Base, public MetalLowLevelTestBase
{
public:
    CPUOverheadTest_Metal(const GlobalTestEnvironment* const gte);
    virtual ~CPUOverheadTest_Metal();
    
protected:
    
    
    virtual KCL::KCL_Status init ();
    virtual bool render ();
    
    virtual void FreeResources();
    
private:
    MTLRenderPipelineDescriptor *pPipelineStateDescriptor ;
    
    void InitTestData() ;
    void Update() ;
    
    void InitStates() ;
    void ClearStates() ;
    
    MetalRender::DynamicDataBufferPool* m_dynamic_data_buffer_pool ;
    MetalRender::DynamicDataBuffer* m_dynamic_data_buffer ;
    
    id <MTLBuffer> m_VertexBuffers[BufferCount] ;
    id <MTLBuffer> m_IndexBuffers[BufferCount] ;
    
    
    MTLRenderPassDescriptor * getRenderPassDescriptor(id<MTLTexture> texture) ;
    
    id<MTLTexture> m_DepthTex ;
    
    const GlobalTestEnvironment* const m_gte ;
    
    
    const static int DEPTH_STATE_COUNT = 7 ;
    const static int BLEND_STATE_COUNT = 6 ;
    
    id <MTLRenderPipelineState>  m_PipelineStates[BLEND_STATE_COUNT] ;
    id <MTLDepthStencilState> m_DepthStates[DEPTH_STATE_COUNT] ;

};


CPUOverheadTest_Metal::CPUOverheadTest_Metal(const GlobalTestEnvironment* const gte) : CPUOverheadTest_Base(gte), m_gte( gte )
{
    m_dynamic_data_buffer_pool = nullptr;
    m_dynamic_data_buffer = nullptr;
}


CPUOverheadTest_Metal::~CPUOverheadTest_Metal()
{
    FreeResources() ;
}


KCL::KCL_Status CPUOverheadTest_Metal::init ()
{
    InitMetalContext(m_gte->GetGraphicsContext(), m_settings) ;
    
    // create a new command queue
    m_CommandQueue = m_Context->getMainCommandQueue();
    
    if(!m_CommandQueue) { NSLog(@">> ERROR: Failed creating a new command queue!"); return KCL::KCL_TESTERROR_UNKNOWNERROR; } // if
    
    
    m_ClearDepth = 1.0;
    
    const char* cpuoverhead_shader_source_filename = "shaders_mtl/lowlevel/cpuoverhead_test.metal" ;
    
    std::string vs_shader_header = "" ;
    std::string fs_shader_header = "" ;
    
    bool force_highp = GetSetting().m_force_highp;
#if !TARGET_OS_IPHONE
    force_highp = true;
#endif
    if (force_highp)
    {
        vs_shader_header += "#define FORCE_HIGHP 1\n" ;
        vs_shader_header += "#define VARYING_HIGHP 1\n" ;
        
        fs_shader_header += "#define FORCE_HIGHP 1\n" ;
        fs_shader_header += "#define VARYING_HIGHP 1\n" ;
    }
    else
    {
        vs_shader_header += "#define FORCE_HIGHP 0\n" ;
        vs_shader_header += "#define VARYING_HIGHP 0\n" ;
        
        fs_shader_header += "#define FORCE_HIGHP 0\n" ;
        fs_shader_header += "#define VARYING_HIGHP 0\n" ;
    }
    
    KCL::KCL_Status status = LoadLibrariesFromFile(cpuoverhead_shader_source_filename,cpuoverhead_shader_source_filename, vs_shader_header.c_str(), fs_shader_header.c_str()) ;
    
    if (status != KCL::KCL_TESTERROR_NOERROR)
    {
        return status ;
    }
    
    // load the fragment program into the library
    id <MTLFunction> fragment_program = [m_FragmentShaderLibrary newFunctionWithName:@"cpuOverheadFragment"];
    
    
    if(!fragment_program) { NSLog(@">> ERROR: Failed creating a fragment shader!"); return KCL::KCL_TESTERROR_SHADER_ERROR; } // if
    
    
    // load the vertex program into the library
    id <MTLFunction> vertex_program = [m_VertexShaderLibrary newFunctionWithName:@"cpuOverheadVertex"];
    
    
    if(!vertex_program) { NSLog(@">> ERROR: Failed creating a vertex shader!"); return KCL::KCL_TESTERROR_SHADER_ERROR; } // if
    
    //  create a pipeline state for the quad
    pPipelineStateDescriptor = [MTLRenderPipelineDescriptor new];
    
    if(!pPipelineStateDescriptor) { NSLog(@">> ERROR: Failed creating a pipeline state descriptor!"); return KCL::KCL_TESTERROR_UNKNOWNERROR; } // if

    
    pPipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    pPipelineStateDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    
    [pPipelineStateDescriptor setVertexFunction:vertex_program];
    [pPipelineStateDescriptor setFragmentFunction:fragment_program];
    
    
    
    NSUInteger interleavedBufferStride = 5 * sizeof(float);
    MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];
    
    
    vertexDesc.attributes[0].format = MTLVertexFormatFloat3;
    vertexDesc.attributes[0].bufferIndex = 0;
    vertexDesc.attributes[0].offset = 0;
    vertexDesc.attributes[1].format = MTLVertexFormatFloat2;
    vertexDesc.attributes[1].bufferIndex = 0;
    vertexDesc.attributes[1].offset = 3 * sizeof(float);  // 6 bytes
    
    vertexDesc.layouts[0].stride = interleavedBufferStride ;
    vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    
    
    [pPipelineStateDescriptor setVertexDescriptor:vertexDesc] ;
    
    
    vertex_program   = nil;
    fragment_program = nil;
    
    
    MTLDepthStencilDescriptor *pDepthStateDesc = [MTLDepthStencilDescriptor new];
    
    if(!pDepthStateDesc) { NSLog(@">> ERROR: Failed creating a depth stencil descriptor!"); return KCL::KCL_TESTERROR_UNKNOWNERROR; } // if
    
    pDepthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
    pDepthStateDesc.depthWriteEnabled    = YES;
    
    
    pDepthStateDesc = nil;
    
    
    MTLTextureDescriptor *pDepthTexDesc= [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                            width:m_Viewport.width
                                                                                           height:m_Viewport.height
                                                                                        mipmapped:NO];
    
#if !TARGET_OS_EMBEDDED
    pDepthTexDesc.storageMode = MTLStorageModePrivate ;
#endif
    pDepthTexDesc.usage = MTLTextureUsageRenderTarget ;
    
    m_DepthTex = [m_CommandQueue.device newTextureWithDescriptor:pDepthTexDesc];
    
    pDepthTexDesc = nil ;
    
    m_dynamic_data_buffer_pool = new MetalRender::DynamicDataBufferPool(MetalRender::METAL_MAX_FRAME_LAG) ;
    m_dynamic_data_buffer = m_dynamic_data_buffer_pool->GetNewBuffer(2*1024*1024) ;
    
    InitTestData() ;
    InitStates() ;
    
    return KCL::KCL_TESTERROR_NOERROR ;
}


void CPUOverheadTest_Metal::FreeResources()
{
    // wait for finish all draw command
    id <MTLCommandBuffer> finishBuffer = [m_CommandQueue commandBuffer];
    [finishBuffer commit] ;
    [finishBuffer waitUntilCompleted] ;
    
    delete m_dynamic_data_buffer_pool ;
    
    for (int i = 0; i < BufferCount; i++)
    {
        releaseObj(m_VertexBuffers[i]) ;
        releaseObj(m_IndexBuffers[i]) ;
    }
    releaseObj(m_DepthTex) ;
    
    ClearStates() ;
    
    FreeShaderLibraries();
}


void CPUOverheadTest_Metal::InitStates()
{
    MTLDepthStencilDescriptor *pDepthStateDesc = [MTLDepthStencilDescriptor new];
    pDepthStateDesc.depthWriteEnabled    = YES;
 
    for (int i = 0 ; i < DEPTH_STATE_COUNT ; i++)
    {
        switch (i)
        {
                
            case 0:
                pDepthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
                pDepthStateDesc.depthWriteEnabled = false ;
                
                //glDisable(GL_DEPTH_TEST);
                //glDepthFunc(GL_NEVER);
                break;
                
            case 1:
                pDepthStateDesc.depthCompareFunction = MTLCompareFunctionLessEqual ;
                pDepthStateDesc.depthWriteEnabled = true ;
                
                //glEnable(GL_DEPTH_TEST);
                //glDepthFunc(GL_LEQUAL);
                break;
                
            case 2:
                pDepthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
                pDepthStateDesc.depthWriteEnabled = false ;
                
                //pDepthStateDesc.depthCompareFunction = MTLCompareFunctionEqual ;
                
                //glDisable(GL_DEPTH_TEST);
                //glDepthFunc(GL_EQUAL);
                break;
                
            case 3:
                pDepthStateDesc.depthCompareFunction = MTLCompareFunctionGreater ;
                pDepthStateDesc.depthWriteEnabled = true ;
                
                //glEnable(GL_DEPTH_TEST);
                //glDepthFunc(GL_GREATER);
                break;
                
            case 4:
                pDepthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
                pDepthStateDesc.depthWriteEnabled = false ;
                
                //pDepthStateDesc.depthCompareFunction = MTLCompareFunctionNotEqual ;
                
                //glDisable(GL_DEPTH_TEST);
                //glDepthFunc(GL_NOTEQUAL);
                break;
                
            case 5:
                pDepthStateDesc.depthCompareFunction = MTLCompareFunctionGreaterEqual ;
                pDepthStateDesc.depthWriteEnabled = true ;
                
                //glEnable(GL_DEPTH_TEST);
                //glDepthFunc(GL_GEQUAL);
                break;
                
            case 6:
                pDepthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
                pDepthStateDesc.depthWriteEnabled = true ;
                
                //glEnable(GL_DEPTH_TEST);
                //glDepthFunc(GL_ALWAYS);
                break;
        }
        
        
        m_DepthStates[i] = [m_Device newDepthStencilStateWithDescriptor:pDepthStateDesc];
        
    }
    
    releaseObj(pDepthStateDesc) ;
    
    
    pPipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
    
    pPipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    pPipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    
    
    MTLBlendFactor SourceFactor = MTLBlendFactorOne ;
    MTLBlendFactor DestFactor = MTLBlendFactorZero ;
    
    for (int j = 0; j < BLEND_STATE_COUNT ; j++)
    {
        switch (j)
        {
            case 0:
                SourceFactor = MTLBlendFactorBlendAlpha;
                DestFactor   = MTLBlendFactorOneMinusBlendAlpha;
                
                //glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
                //glEnable(GL_BLEND);
                break;
                
            case 1:
                SourceFactor = MTLBlendFactorSourceColor;
                DestFactor   = MTLBlendFactorOneMinusSourceColor;
                
                
                //glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
                //glEnable(GL_BLEND);
                break;
                
            case 2:
                SourceFactor = MTLBlendFactorBlendColor;
                DestFactor   = MTLBlendFactorOneMinusBlendColor;
                
                
                //glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR);
                //glEnable(GL_BLEND);
                break;
                
            case 3:
                SourceFactor = MTLBlendFactorSourceAlpha;
                DestFactor   = MTLBlendFactorOneMinusSourceAlpha;
                
                //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                //glEnable(GL_BLEND);
                break;
                
            case 4:
                SourceFactor = MTLBlendFactorZero;
                DestFactor   = MTLBlendFactorSourceColor;
                
                //glBlendFunc(GL_ZERO, GL_SRC_COLOR);
                //glEnable(GL_BLEND);
                break;
                
            case 5:
                SourceFactor = MTLBlendFactorSourceAlpha;
                DestFactor   = MTLBlendFactorOne;
                
                //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                //glEnable(GL_BLEND);
                break;
        }
        
        pPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = SourceFactor;
        pPipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = SourceFactor;
        
        pPipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = DestFactor;
        pPipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = DestFactor;
        
        
        
        NSError *pError = nil;
        m_PipelineStates[j] = [m_Device newRenderPipelineStateWithDescriptor:pPipelineStateDescriptor
                                                                       error:&pError];
        
    }
    
}


void CPUOverheadTest_Metal::ClearStates()
{
    for (int i = 0 ; i < DEPTH_STATE_COUNT ; i++)
    {
        releaseObj(m_DepthStates[i]) ;
    }
    
    for (int j = 0 ; j < BLEND_STATE_COUNT ; j++)
    {
        releaseObj(m_PipelineStates[j]) ;
    }
}



void CPUOverheadTest_Metal::InitTestData()
{
    
    int seed = 123;
    for (int i = 0; i < BufferCount; i++)
    {
        int vertexCount = i + 3;
        int triangleCount = vertexCount - 2;
        
        // Generate vertices
        float* vertices = new float[vertexCount * 5];
        float* vertPtr = vertices;
        float theta = 0.0f;
        float thetaStep = 6.283185307179586476925286766559f / vertexCount;
        
        for (int j = 0; j < vertexCount; j++, theta += thetaStep)
        {
            float cosTheta = cosf(theta);
            float sinTheta = sinf(theta);
            *(vertPtr++) = cosTheta;
            *(vertPtr++) = sinTheta;
            *(vertPtr++) = Math::randomf(&seed);		// Z value
            *(vertPtr++) = cosTheta * 0.5f + 0.5f;
            *(vertPtr++) = sinTheta * 0.5f + 0.5f;
        }
        
        
        KCL::uint32 vertex_buffer_size = sizeof(float)*5*vertexCount ;
#if TARGET_OS_EMBEDDED
        m_VertexBuffers[i] = [m_Device newBufferWithLength:vertex_buffer_size
                                                   options:0];
        m_VertexBuffers[i].label = @"VertexBuffer" ;
        
        
        float *pVertices = (float *)[m_VertexBuffers[i] contents];
        memcpy(pVertices, vertices, vertex_buffer_size);
#else
        m_VertexBuffers[i] = [m_Device newBufferWithBytes:vertices length:vertex_buffer_size options:MTLStorageModeManaged];
        m_VertexBuffers[i].label = @"VertexBuffer" ;
#endif
        
        
        KCL::uint16* indices = new KCL::uint16[triangleCount * 3];
        KCL::uint16* indPtr = indices;
        for (int j = 1; j <= triangleCount; j++)
        {
            *(indPtr++) = 0;
            *(indPtr++) = j;
            *(indPtr++) = j + 1;
        }
        
        
        
        KCL::uint32 index_buffer_size = sizeof(KCL::uint16) * triangleCount * 3 ;
#if TARGET_OS_EMBEDDED
        m_IndexBuffers[i] = [m_Device newBufferWithLength:index_buffer_size
                                                  options:0];
        m_IndexBuffers[i].label = @"IndexBuffer" ;
        
        
        KCL::uint16 *pIndices = (KCL::uint16 *)[m_IndexBuffers[i] contents];
        memcpy(pIndices, indices, index_buffer_size);
#else
        m_IndexBuffers[i] = [m_Device newBufferWithBytes:indices length:index_buffer_size options:MTLResourceStorageModeManaged];
        m_IndexBuffers[i].label = @"IndexBuffer" ;
#endif
        
        delete[] vertices;
        delete[] indices;
    }
}



void CPUOverheadTest_Metal::Update()
{
    float sint = cosf(m_time * 0.0015707963267948966192313216916398f);
    m_ClearColor = MTLClearColorMake(0.25f, 0.75f - 0.25f * sint, 0.75f + 0.25f * sint, 1.0f);
}


MTLRenderPassDescriptor * CPUOverheadTest_Metal::getRenderPassDescriptor(id<MTLTexture> texture)
{
    MTLRenderPassDescriptor * m_pClearPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    
    m_pClearPassDescriptor.colorAttachments[0].texture = texture ;
    m_pClearPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear ;
    m_pClearPassDescriptor.colorAttachments[0].clearColor = m_ClearColor;
    
    
    m_pClearPassDescriptor.depthAttachment.texture = m_DepthTex ;
    m_pClearPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear ;
    m_pClearPassDescriptor.depthAttachment.clearDepth = m_ClearDepth ;
    
    return m_pClearPassDescriptor;
}


bool CPUOverheadTest_Metal::render ()
{
    @autoreleasepool {
        
        m_dynamic_data_buffer_pool->InitFrame() ;
    
        Update() ;
        
        id <MTLCommandBuffer> commandBuffer = [m_CommandQueue commandBuffer];
        
        
        // obtain the renderpass descriptor for this drawable
        id <MTLTexture> frameBufferTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
        
        MTLRenderPassDescriptor *ClearRenderPassDescriptor = getRenderPassDescriptor (frameBufferTexture);
        
        // Get a render encoder
        id <MTLRenderCommandEncoder>  renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:ClearRenderPassDescriptor];
        
        // Encode into a renderer
        [renderEncoder setViewport:m_Viewport];
        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        
        
        float m_screenWidth  = m_Viewport.width;
        float m_screenHeight = m_Viewport.height;
        
        float rotation = 0.003 * m_time;
        
        
        // set context state with the render encoder
        
        int seed = 0;
        
        
        int i = 0 ;
        
        // Get a render encoder
        
        int instanceIdx = 0;
        for (int y = 0; y < RowCount; y++)
        {
            for (int x = 0; x < ColumnCount; x++)
            {
                int j = x*RowCount+y ;
                
                float r0 = Math::randomf(&seed);
                float r1 = Math::randomf(&seed);
                float r2 = Math::randomf(&seed);
                float r3 = Math::randomf(&seed);
                int stateIdx = Math::randomf(&seed) * 140;
                instanceIdx++;
                seed += x + y;
                
                VertexUniforms vertexUniforms ;
                
                if (m_screenWidth > m_screenHeight)
                {
                    simd::float2 position         = { float(x + 0.5f * (y & 1)), float(y) } ;        vertexUniforms.u_position   = position ;
                    simd::float2 matrixSize       = { float(ColumnCount),        float(RowCount) } ; vertexUniforms.u_matrixSize = matrixSize ;
                }
                else
                {
                    // MTL_TODO
                    // Workaround for iphones
                    simd::float2 position         = { float(RowCount)-float(y), float(ColumnCount)-float(x + 0.5f * (y & 1)) } ;
                    vertexUniforms.u_position   = position ;
                    simd::float2 matrixSize       = { float(RowCount),        float(ColumnCount) } ; vertexUniforms.u_matrixSize = matrixSize ;
                }
                
                float        u_rotation       =   float( rotation * (0.2f + r2) + r0 ) ;         vertexUniforms.u_rotation   = u_rotation  ;
                
                simd::float2 scale            = { float( r1 + 0.5f ),     float ( r2 + 0.5f ) }     ; vertexUniforms.u_scale = scale ;
                simd::float2 screenResolution = { float( m_screenWidth ), float( m_screenHeight ) } ; vertexUniforms.u_screenResolution = screenResolution ;
                
                simd::float4 color0 = { r0, r1, r2, r3 }; vertexUniforms.u_color0 = color0 ;
                simd::float4 color1 = { r1, r2, r3, r0 }; vertexUniforms.u_color1 = color1 ;
                simd::float4 color2 = { r2, r3, r0, r1 }; vertexUniforms.u_color2 = color2 ;
                simd::float4 color3 = { r3, r0, r1, r2 }; vertexUniforms.u_color3 = color3 ;
                
                int bufferIdx = stateIdx % BufferCount;
                int vertexCount = bufferIdx + 3;
                int triangleCount = vertexCount - 2;
                
                
                m_dynamic_data_buffer->WriteAndSetData<true, false>(renderEncoder, 1, &vertexUniforms, sizeof(VertexUniforms)) ;
                
                
                // Toggle between different settings
                int depthStateIdx = (instanceIdx / 100) % 7 ;
                
                [renderEncoder setDepthStencilState:m_DepthStates[depthStateIdx]];
                
                
                // glBlendColor(r3, r2, r1, r0);
                [renderEncoder setBlendColorRed:r3 green:r2 blue:r1 alpha:r0] ;
                
                int blendStateIdx = (instanceIdx / 10)%5 ;
                
                
                [renderEncoder setRenderPipelineState:m_PipelineStates[blendStateIdx]];
                
                
                [renderEncoder setVertexBuffer:m_VertexBuffers[bufferIdx]
                                        offset:0
                                       atIndex:0 ];
                
                // tell the render context we want to draw our primitives
                [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                          indexCount:triangleCount * 3
                                           indexType:MTLIndexTypeUInt16
                                         indexBuffer:m_IndexBuffers[bufferIdx]
                                   indexBufferOffset:0];
                
                
                i++ ;
                
                
            }
        }
        
        [renderEncoder endEncoding];
        renderEncoder = nil ;
        
        
        // END DRAW PRIMITIVES
        
        unsigned char slot = m_dynamic_data_buffer_pool->GetCurrentSlot();
        
        [commandBuffer addCompletedHandler:^(id <MTLCommandBuffer> completedCommandBuffer)
         {
             m_dynamic_data_buffer_pool->MarkSlotUnused(slot);
         }];
        
        [commandBuffer commit];
        
        
        
        commandBuffer = nil;
        
    }
 
    return true ;
}


CPUOverheadTest_Base *CreateCPUOverheadTestMetal(const GlobalTestEnvironment* const gte)
{
    return new CPUOverheadTest_Metal(gte) ;
}

