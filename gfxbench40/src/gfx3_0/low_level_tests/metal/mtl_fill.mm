/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "fill_base.h"

#import "mtl_low_level_base.h"

#include "mtl_texture.h"
#import "mtl_dynamic_data_buffer.h"

#include "fbo.h"
#include "fbo_metal.h"


struct VertexUniforms
{
    float u_rotation;
    float u_aspectRatio;
    
    float __pad1 ;
    float __pad2 ;
};


class CompressedFillTest_Metal : public CompressedFillTest_Base, public MetalLowLevelTestBase
{
public:
    CompressedFillTest_Metal(const GlobalTestEnvironment* const gte);
    virtual ~CompressedFillTest_Metal();
    
protected:
    virtual void FreeResources() ;
    virtual KCL::KCL_Status init() ;
    virtual bool render() ;
    
    virtual KCL::Texture* CreateTexture(KCL::Image* image) ;
    
private:
    
    MTLRenderPassDescriptor * getRenderPassDescriptor(id<MTLTexture> texture) ;
    
    MetalRender::DynamicDataBufferPool* m_dynamic_data_buffer_pool ;
    MetalRender::DynamicDataBuffer* m_dynamic_data_buffer ;
    
    
    id <MTLBuffer> m_VertexBuffer ;
    id <MTLBuffer> m_IndexBuffer ;
    
    id <MTLDepthStencilState> m_DepthState ;
    id <MTLRenderPipelineState> m_PipelineState ;
    
    float m_aspectRatio;
    
};


CompressedFillTest_Metal::CompressedFillTest_Metal(const GlobalTestEnvironment* const gte) : CompressedFillTest_Base(gte),
     m_aspectRatio(0.0f)
{
    
    
}


CompressedFillTest_Metal::~CompressedFillTest_Metal()
{
    FreeResources() ;
}


KCL::Texture* CompressedFillTest_Metal::CreateTexture(KCL::Image* image)
{
    MetalRender::TextureFactory f;
    
    return f.CreateTexture(image) ;
}


KCL::KCL_Status CompressedFillTest_Metal::init()
{
    KCL::KCL_Status error = CompressedFillTest_Base::init() ;
    
    if (error != KCL::KCL_TESTERROR_NOERROR)
    {
        return error ;
    }
    
    
    InitMetalContext(m_gte->GetGraphicsContext(), m_settings) ;
    m_CommandQueue = m_Context->getMainCommandQueue();
    
    m_dynamic_data_buffer_pool = new MetalRender::DynamicDataBufferPool(MetalRender::METAL_MAX_FRAME_LAG) ;
    m_dynamic_data_buffer = m_dynamic_data_buffer_pool->GetNewBuffer(1024*1024) ;
    
    
    //
    //  Load shaders
    //
    const char* compressedfill_shader_source_filename = "shaders_mtl/lowlevel/compressedfill.metal" ;
    
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
    
    KCL::KCL_Status status = LoadLibrariesFromFile(compressedfill_shader_source_filename,compressedfill_shader_source_filename, vs_shader_header.c_str(), fs_shader_header.c_str()) ;
    
    if (status != KCL::KCL_TESTERROR_NOERROR)
    {
        return status ;
    }
    
    // load the fragment and vertex program into the library
    id <MTLFunction> fragment_program = [m_FragmentShaderLibrary newFunctionWithName:@"CompressedFillFragment"];
    id <MTLFunction> vertex_program = [m_VertexShaderLibrary newFunctionWithName:@"CompressedFillVertex"];
    
    if(!fragment_program || !vertex_program)
    {
        return KCL::KCL_TESTERROR_SHADER_ERROR;
    }

    
    
    //
    //  Upload vertex buffer
    //
    static const float screenBillboard[] =
    {
        -1, -1, 0.5f, 0, 1,
        -1,  1, 0.5f, 0, 0,
        1, -1, 0.5f, 1, 1,
        1,  1, 0.5f, 1, 0
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

    
    //
    // Depth State
    //
    MTLDepthStencilDescriptor *pDepthStateDesc = [MTLDepthStencilDescriptor new];
    pDepthStateDesc.depthWriteEnabled    = false;
    pDepthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
    m_DepthState = [m_Device newDepthStencilStateWithDescriptor:pDepthStateDesc];
    
    
    MTLRenderPipelineDescriptor *pPipelineStateDescriptor = [MTLRenderPipelineDescriptor new];
    
    
    pPipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pPipelineStateDescriptor.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
    
    [pPipelineStateDescriptor setVertexFunction:vertex_program];
    [pPipelineStateDescriptor setFragmentFunction:fragment_program];
    
    
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
    
    
    [pPipelineStateDescriptor setVertexDescriptor:vertexDesc] ;
    releaseObj(vertexDesc) ;
    
    
    //
    // Blending
    //
    
    /*
     GLB::OpenGLStateManager::GlEnable(GL_BLEND);
     GLB::OpenGLStateManager::GlBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
    */
    
    pPipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
    
    pPipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    pPipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    
    MTLBlendFactor SourceFactor = MTLBlendFactorBlendAlpha ;
    MTLBlendFactor DestFactor = MTLBlendFactorOneMinusBlendAlpha ;
    
    pPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = SourceFactor;
    pPipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = SourceFactor;
    
    pPipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = DestFactor;
    pPipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = DestFactor;
    
    
    NSError *pError = nil;
    m_PipelineState = [m_Device newRenderPipelineStateWithDescriptor:pPipelineStateDescriptor
                                                               error:&pError];
    
    // glClearColor(0, 0, 0, 0);
    m_ClearColor = MTLClearColorMake(0.0f, 0.0f, 0.0f, 0.0f);
    
    return KCL::KCL_TESTERROR_NOERROR;
}


void CompressedFillTest_Metal::FreeResources()
{
    id <MTLCommandBuffer> finishBuffer = [m_CommandQueue commandBuffer];
    [finishBuffer commit] ;
    [finishBuffer waitUntilCompleted] ;
    
    CompressedFillTest_Base::FreeResources() ;
    
    releaseObj(m_IndexBuffer) ;
    releaseObj(m_VertexBuffer) ;
    
    releaseObj(m_DepthState) ;
    releaseObj(m_PipelineState) ;
    
    FreeShaderLibraries();
    
    delete m_dynamic_data_buffer_pool ;
}


MTLRenderPassDescriptor * CompressedFillTest_Metal::getRenderPassDescriptor(id<MTLTexture> texture)
{
    MTLRenderPassDescriptor * m_pClearPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    
    //glClear(GL_COLOR_BUFFER_BIT);
    
    m_pClearPassDescriptor.colorAttachments[0].texture = texture ;
    m_pClearPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear ;
    m_pClearPassDescriptor.colorAttachments[0].clearColor = m_ClearColor;
    
    m_pClearPassDescriptor.depthAttachment.texture = nil ;
    m_pClearPassDescriptor.depthAttachment.loadAction = MTLLoadActionDontCare ;
    m_pClearPassDescriptor.depthAttachment.clearDepth = m_ClearDepth ;
    
    return m_pClearPassDescriptor;
}


bool CompressedFillTest_Metal::render()
{
    @autoreleasepool {
        
        id <MTLTexture> frameBufferTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
        MTLRenderPassDescriptor *ClearRenderPassDescriptor = getRenderPassDescriptor (frameBufferTexture);

        
        if (m_aspectRatio == 0.0f)
        {
            m_aspectRatio = (float)(m_Viewport.width) / m_Viewport.height;
        }
        
        m_dynamic_data_buffer_pool->InitFrame() ;
        id <MTLCommandBuffer> commandBuffer = [m_CommandQueue commandBuffer];
        
        // Get a render encoder
        id <MTLRenderCommandEncoder>  renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:ClearRenderPassDescriptor];
        
        // Encode into a renderer
        [renderEncoder setViewport:m_Viewport];
        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        
        
        [renderEncoder setDepthStencilState:m_DepthState];
        
        [renderEncoder setRenderPipelineState:m_PipelineState];
        
        [renderEncoder setVertexBuffer:m_VertexBuffer
                                offset:0
                               atIndex:0 ];
        
        MetalRender::Texture* mtl_colorTexture = static_cast<MetalRender::Texture*>(m_colorTexture) ;
        mtl_colorTexture->Set(renderEncoder, 0) ;
        
        MetalRender::Texture* mtl_lightTexture = static_cast<MetalRender::Texture*>(m_lightTexture) ;
        mtl_lightTexture->Set(renderEncoder, 1) ;
        
        VertexUniforms vertexUniforms ;
        
        //glUniform1f(m_uniAspectRatio, m_aspectRatio);
        vertexUniforms.u_aspectRatio = m_aspectRatio ;
        
        const float exposureTime = 150;
        for (int i = 0; i < m_displayedElementCount; i++)
        {
            //glBlendColor(1, 1, 1, 1.0f / (1 + i));
            [renderEncoder setBlendColorRed:1.0f green:1.0f blue:1.0f alpha:1.0f / (1 + i)] ;
            
            float rotation = (m_time + exposureTime * i / m_displayedElementCount) * 0.0003;
            
            //glUniform1f(m_rotationLocation, rotation);
            vertexUniforms.u_rotation = rotation ;
            
            m_dynamic_data_buffer->WriteAndSetData<true, false>(renderEncoder, 1, &vertexUniforms, sizeof(VertexUniforms)) ;
            
            [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                      indexCount:6
                                       indexType:MTLIndexTypeUInt16
                                     indexBuffer:m_IndexBuffer
                               indexBufferOffset:0];
        }
        
        const unsigned int texelsPerScreen =  m_settings->m_viewport_width * m_settings->m_viewport_height * 4;
        m_transferredTexels += texelsPerScreen * m_displayedElementCount;
        
        
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


CompressedFillTest_Base *CreateCompressedFillTestMetal(const GlobalTestEnvironment* const gte)
{
    return new CompressedFillTest_Metal(gte) ;
}


