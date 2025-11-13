/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#import <Foundation/Foundation.h>

#include "mtl_low_level.h"
#include "ng/log.h"

#import <Metal/Metal.h>
#import <simd/simd.h>
#include "kcl_math3d.h"

#import "mtl_low_level_base.h"

#import "graphics/graphicscontext.h"
#import "graphics/metalgraphicscontext.h"
#import "../gfxbench/global_test_environment.h"
#import "mtl_dynamic_data_buffer.h"

#include "kcl_io.h"

#include "fbo.h"
#include "fbo_metal.h"

using namespace KCL ;


//
//
//  Fragment uniforms data structure
//  MTL_TODO
//
//

struct FragmentUniforms{
    float          m_time ;
    simd::float2   m_aspectRatio ;
    simd::float3   m_lightDir ;
    simd::float3   m_eyePosition ;
    
    KCL::Matrix4x4 m_orientation ;
};



//
//
//  AluTest_Metal class
//
//

class ALUTest_Metal : public ALUTest_Base, public MetalLowLevelTestBase
{
public:
    ALUTest_Metal(const GlobalTestEnvironment* const gte);
    virtual ~ALUTest_Metal();
    
protected:
    
    virtual const char* getUom() const { return "frames"; }
    virtual bool isWarmup() const { return false; }
    virtual KCL::uint32 indexCount() const { return 0; }
    
    virtual KCL::KCL_Status init ();
    virtual bool render ();
    
    virtual void FreeResources();
    
private:
    // Textured Quad
    id <MTLBuffer> m_VertexBuffer ;
    id <MTLBuffer> m_IndexBuffer ;
    
    
    FragmentUniforms m_FragmentUniforms ;
    
    MetalRender::DynamicDataBufferPool* m_dynamic_data_buffer_pool ;
    MetalRender::DynamicDataBuffer* m_dynamic_data_buffer ;
    
    id <MTLRenderPipelineState>  m_PipelineState;
    id <MTLDepthStencilState>    m_DepthState;
    
    
    void Encode(id <MTLRenderCommandEncoder> renderEncoder) ;
    void Update();
    
   // MTLRenderPassDescriptor * getAluRenderPassDescriptor(id <MTLTexture> texture) ;
    
    bool m_isRotated ;
    KCL::Vector2D m_aspectRatio;
    
    MTLRenderPassDescriptor * m_aluRenderPassDescriptor ;
    
    const GlobalTestEnvironment* const m_gte ;
};


ALUTest_Metal::ALUTest_Metal(const GlobalTestEnvironment* const gte) : ALUTest_Base(gte), m_gte(gte)
{
    m_isRotated = false ;
    
}


ALUTest_Metal::~ALUTest_Metal()
{
    FreeResources() ;
}


KCL::KCL_Status ALUTest_Metal::init()
{
    InitMetalContext(m_gte->GetGraphicsContext(), m_settings) ;
    
    {
        // create a new command queue
        m_CommandQueue = m_Context->getMainCommandQueue();
        
        m_dynamic_data_buffer_pool = new MetalRender::DynamicDataBufferPool(MetalRender::METAL_MAX_FRAME_LAG) ;
        m_dynamic_data_buffer = m_dynamic_data_buffer_pool->GetNewBuffer(1024*1024) ;
        
        
        if(!m_CommandQueue) { NSLog(@">> ERROR: Failed creating a new command queue!"); return KCL::KCL_TESTERROR_UNKNOWNERROR; } // if
        
        m_ClearColor = MTLClearColorMake(0.0f, 0.0f, 0.0f, 1.0f);
        m_ClearDepth = 1.0;
        
        NSError *pError = nil;
        
        
        const char* alu_shader_source_filename = "shaders_mtl/lowlevel/metal_alu.metal" ;
        
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
        
        KCL::KCL_Status status = LoadLibrariesFromFile(alu_shader_source_filename,alu_shader_source_filename, vs_shader_header.c_str(), fs_shader_header.c_str()) ;
        
        if (status != KCL::KCL_TESTERROR_NOERROR)
        {
            return status ;
        }

        
        
        // load the fragment program into the library
        id <MTLFunction> fragment_program = [m_FragmentShaderLibrary newFunctionWithName:@"AluFragment"];
        
        
        if(!fragment_program) { NSLog(@">> ERROR: Failed creating a fragment shader!"); return KCL::KCL_TESTERROR_SHADER_ERROR; } // if
        
        
        // load the vertex program into the library
        id <MTLFunction> vertex_program = [m_VertexShaderLibrary newFunctionWithName:@"AluVertex"];
        
        
        if(!vertex_program) { NSLog(@">> ERROR: Failed creating a vertex shader!"); return KCL::KCL_TESTERROR_SHADER_ERROR; } // if
        
        //  create a pipeline state for the quad
        MTLRenderPipelineDescriptor *pAluPipelineStateDescriptor = [MTLRenderPipelineDescriptor new];
        
        if(!pAluPipelineStateDescriptor) { NSLog(@">> ERROR: Failed creating a pipeline state descriptor!"); return KCL::KCL_TESTERROR_UNKNOWNERROR; } // if
        
        
        // MTL_TODO
        //GLB::FBOMetalBase* metal_fbo = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind()) ;
        //pQuadPipelineStateDescriptor.colorAttachments[0].pixelFormat = metal_fbo->GetTexture().pixelFormat;
        
        pAluPipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        
        [pAluPipelineStateDescriptor setVertexFunction:vertex_program];
        [pAluPipelineStateDescriptor setFragmentFunction:fragment_program];
        
        
        //
        //  VertexLayout
        //
        
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
        
        
        [pAluPipelineStateDescriptor setVertexDescriptor:vertexDesc] ;
        releaseObj(vertexDesc) ;

        
        
        m_PipelineState = [m_Device newRenderPipelineStateWithDescriptor:pAluPipelineStateDescriptor
                                                                   error:&pError];

        
        releaseObj(pAluPipelineStateDescriptor) ;
        
        releaseObj(vertex_program) ;
        releaseObj(fragment_program) ;
        
        if(!m_PipelineState) { NSLog(@">> ERROR: Failed acquiring pipeline state descriptor: %@", pError); return KCL::KCL_TESTERROR_UNKNOWNERROR; } // if
        
       
        
        
        MTLDepthStencilDescriptor *pDepthStateDesc = [MTLDepthStencilDescriptor new];
        
        if(!pDepthStateDesc) { NSLog(@">> ERROR: Failed creating a depth stencil descriptor!"); return KCL::KCL_TESTERROR_UNKNOWNERROR; } // if
        
        pDepthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
        pDepthStateDesc.depthWriteEnabled    = NO;
        
        m_DepthState = [m_Device newDepthStencilStateWithDescriptor:pDepthStateDesc];
        
        
        releaseObj(pDepthStateDesc) ;
        
        if(!m_DepthState) { NSLog(@">> ERROR: Failed creating a depth stencil state descriptor!"); return KCL::KCL_TESTERROR_UNKNOWNERROR; } // if
        

        
        m_aluRenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];

        m_aluRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear ;
        m_aluRenderPassDescriptor.colorAttachments[0].clearColor = m_ClearColor;
        
        m_aluRenderPassDescriptor.depthAttachment.texture = nil ;
        m_aluRenderPassDescriptor.depthAttachment.loadAction = MTLLoadActionDontCare ;
        m_aluRenderPassDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare ;
        
        
        
        static const float screenBillboard[] =
        {
            -1, -1, 0, 0, 0,
            -1,  1, 0, 0, 1,
            1, -1, 0, 1, 0,
            1,  1, 0, 1, 1
        };
        
        
        KCL::uint32 vertex_buffer_size = sizeof(float) * 20 ;
#if TARGET_OS_EMBEDDED
        m_VertexBuffer = [m_Device newBufferWithLength:vertex_buffer_size
                                               options:0];
        m_VertexBuffer.label = @"VertexBuffer" ;
        float *pVertices = (float *)[m_VertexBuffer contents];
        memcpy(pVertices, screenBillboard, vertex_buffer_size);
#else
        m_VertexBuffer = [m_Device newBufferWithBytes:screenBillboard length:vertex_buffer_size options:MTLResourceStorageModeManaged];
        m_VertexBuffer.label = @"VertexBuffer" ;
#endif
        if (!m_VertexBuffer)
        {
            return KCL::KCL_TESTERROR_VBO_ERROR;
        }
        
        
        //
        //  Upload indices buffer
        //
        static const KCL::uint16 billboardIndices[] = { 0, 1, 2, 1, 2, 3 };
        KCL::uint32 index_buffer_size = sizeof(KCL::uint16) * 6 ;
#if TARGET_OS_EMBEDDED
        m_IndexBuffer = [m_Device newBufferWithLength:index_buffer_size
                                              options:0];
        m_IndexBuffer.label = @"IndexBuffer" ;
        KCL::uint16 *pIndices = (KCL::uint16 *)[m_IndexBuffer contents];
        memcpy(pIndices, billboardIndices, index_buffer_size);
#else
        m_IndexBuffer = [m_Device newBufferWithBytes:billboardIndices length:index_buffer_size options:MTLResourceStorageModeManaged];
        m_IndexBuffer.label = @"IndexBuffer" ;
#endif
        if (!m_IndexBuffer)
        {
            return KCL::KCL_TESTERROR_VBO_ERROR;
        }
        
        
       
    }
    
    
    return KCL::KCL_TESTERROR_NOERROR;
}


void ALUTest_Metal::FreeResources()
{
    id <MTLCommandBuffer> finishBuffer = [m_CommandQueue commandBuffer];
    [finishBuffer commit] ;
    [finishBuffer waitUntilCompleted] ;
    
    releaseObj(m_PipelineState) ;
    releaseObj(m_IndexBuffer) ;
    releaseObj(m_VertexBuffer) ;
    
    FreeShaderLibraries();
    
    delete  m_dynamic_data_buffer_pool ;

    releaseObj(m_DepthState) ;
}


void ALUTest_Metal::Encode(id <MTLRenderCommandEncoder> renderEncoder)
{
    // set context state with the render encoder
    [renderEncoder setViewport:m_Viewport];
    [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    
    [renderEncoder setRenderPipelineState:m_PipelineState];
    
    [renderEncoder setVertexBuffer:m_VertexBuffer
                            offset:0
                           atIndex:0 ];
    
    
    m_dynamic_data_buffer->WriteAndSetData<false, true>(renderEncoder, 0, &m_FragmentUniforms, sizeof(FragmentUniforms)) ;
    
    
    // tell the render context we want to draw our primitives
    [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                              indexCount:6
                               indexType:MTLIndexTypeUInt16
                             indexBuffer:m_IndexBuffer
                       indexBufferOffset:0];
    
    [renderEncoder endEncoding];
} // _encode


void ALUTest_Metal::Update()
{
    if (m_aspectRatio.x == 0.0f)
    {
        if( m_Viewport.width >= m_Viewport.height)
        {
            m_aspectRatio.x = (float)m_Viewport.width / m_Viewport.height;
            m_aspectRatio.y = 1.0f;
            m_isRotated = false  ;
        }
        else
        {
            m_aspectRatio.x = 1.0f;
            m_aspectRatio.y = (float)m_Viewport.height / m_Viewport.width;
            m_isRotated = true;
        }
    }

    
    m_FragmentUniforms.m_time = (m_time%1000)/1000.0 ;
    
    
    simd::float2 aspectRatio = { m_aspectRatio.x, m_aspectRatio.y } ;
    m_FragmentUniforms.m_aspectRatio = aspectRatio ;
    
    float lightTime   = m_time * 0.0003;
    simd::float3 lightDir = { float( sin(lightTime) ), float( 0.5 + 0.5 * cos(lightTime) ),float( 0.5 - 0.5 * cos(lightTime) ) }  ;
    lightDir = simd::normalize(lightDir) ;
    m_FragmentUniforms.m_lightDir = lightDir ;
    
    float flyTime = m_time * 0.0005;
    
    simd::float3 eyePosition = { float(10.0 * sin(flyTime)) , float( sin(flyTime * 0.5) * 2 + 2.1 ), float(10 * cos(flyTime)) };
    m_FragmentUniforms.m_eyePosition = eyePosition;
    
    
    Matrix4x4 yaw, pitch, roll, o ;
    Matrix4x4::RotateY(yaw, -cos(flyTime) * 28);
    Matrix4x4::RotateX(pitch, -cos(flyTime * 0.5) * 28);
    Matrix4x4::RotateZ(roll, - sin(flyTime) * 28);
    
    o = pitch * roll * yaw;
    
    
    if( m_isRotated)
    {
        KCL::Matrix4x4 t;
        KCL::Matrix4x4::RotateZ( t, -90);
        o = o * t;
    }
    
    m_FragmentUniforms.m_orientation = o ;
}


bool ALUTest_Metal::render()
{
    @autoreleasepool {
    
        m_dynamic_data_buffer_pool->InitFrame() ;

        Update() ;
        
        id <MTLTexture> frameBufferTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
        id <MTLCommandBuffer> commandBuffer = [m_CommandQueue commandBuffer];
        
        m_aluRenderPassDescriptor.colorAttachments[0].texture = frameBufferTexture ;
        
        //if(renderPassDescriptor)
        {
            // Get a render encoder
            id <MTLRenderCommandEncoder>  renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:m_aluRenderPassDescriptor];
            
            // Encode into a renderer
            Encode(renderEncoder) ;
            
            
            renderEncoder = nil ;
          //  renderPassDescriptor = nil ;
        }
        
        unsigned char slot = m_dynamic_data_buffer_pool->GetCurrentSlot();
        
        [commandBuffer addCompletedHandler:^(id <MTLCommandBuffer> completedCommandBuffer)
         {
             m_dynamic_data_buffer_pool->MarkSlotUnused(slot);
         }];

        [commandBuffer commit];
        
        
        commandBuffer = nil ;        
    }
    
    return true ;
}



ALUTest_Base *CreateALUTestMetal(const GlobalTestEnvironment* const gte)
{
    return new ALUTest_Metal(gte) ;
}




