/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "blending_base.h"

#import "mtl_low_level_base.h"

#include "mtl_texture.h"
#import "mtl_dynamic_data_buffer.h"

#include "fbo.h"
#include "fbo_metal.h"


struct VertexUniforms
{
    KCL::Vector4D u_scale;
    KCL::Vector4D u_offset;
};

class UITest_Metal : public UITest_Base, public MetalLowLevelTestBase
{
public:
    UITest_Metal(const GlobalTestEnvironment* const gte);
    virtual ~UITest_Metal();
    
protected:
    virtual const char* getUom() const { return "MB/s"; }
    virtual bool isWarmup() const { return false; }
    virtual KCL::uint32 indexCount() const { return 0; }
    
    virtual void FreeResources() ;
    virtual KCL::KCL_Status init() ;
    virtual bool render() ;
    
    virtual KCL::Texture* createTexture(KCL::Image* image) ;
    
private:
    
    MTLRenderPassDescriptor * getRenderPassDescriptor(id<MTLTexture> texture) ;
    
    id <MTLBuffer> m_VertexBuffer ;
    id <MTLBuffer> m_IndexBuffer ;
    
    MetalRender::DynamicDataBufferPool* m_dynamic_data_buffer_pool ;
    MetalRender::DynamicDataBuffer* m_dynamic_data_buffer ;
    
    id <MTLDepthStencilState> m_DepthState ;
    id <MTLRenderPipelineState> m_PipelineState ;
    
    float m_scaleX;
    float m_scaleY;
    float m_aspectRatio;
};


UITest_Metal::UITest_Metal(const GlobalTestEnvironment* const gte) : UITest_Base(gte),
    m_scaleX(0.0f),
    m_scaleY(0.0f),
    m_aspectRatio(1.0f)
{

    
}


UITest_Metal::~UITest_Metal()
{
    FreeResources() ;
}


KCL::Texture* UITest_Metal::createTexture(KCL::Image* image)
{
    MetalRender::TextureFactory f;
    
    return f.CreateTexture(image) ;
}


KCL::KCL_Status UITest_Metal::init()
{
    InitMetalContext(m_gte->GetGraphicsContext(), m_settings) ;
    m_CommandQueue = m_Context->getMainCommandQueue();
    
    m_dynamic_data_buffer_pool = new MetalRender::DynamicDataBufferPool(MetalRender::METAL_MAX_FRAME_LAG) ;
    m_dynamic_data_buffer = m_dynamic_data_buffer_pool->GetNewBuffer(1024*1024) ;
    
    
    //
    //  Load shaders
    //
    const char* ui_shader_source_filename = "shaders_mtl/lowlevel/ui.metal" ;
    
    
    std::string vs_shader_header = "" ;
    std::string fs_shader_header = "" ;
    
    bool force_highp = GetSetting().m_force_highp ;
    force_highp = true ;
        
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
    
    KCL::KCL_Status status = LoadLibrariesFromFile(ui_shader_source_filename,ui_shader_source_filename, vs_shader_header.c_str(), fs_shader_header.c_str()) ;
    
    if (status != KCL::KCL_TESTERROR_NOERROR)
    {
        return status ;
    }
    
    if(!m_FragmentShaderLibrary) { NSLog(@">> ERROR: Failed creating a shared library!"); return KCL::KCL_TESTERROR_SHADER_ERROR; }
    
    // load the fragment and vertex program into the library
    id <MTLFunction> fragment_program = [m_FragmentShaderLibrary newFunctionWithName:@"UIFragment"];
    id <MTLFunction> vertex_program = [m_VertexShaderLibrary newFunctionWithName:@"UIVertex"];
    
    if(!fragment_program || !vertex_program)
    {
        return KCL::KCL_TESTERROR_SHADER_ERROR;
    }
    

    //
    //  Upload vertex buffer
    //
    static const float screenBillboard[] =
    {
        -1, -1, 0.5f, 0, 0,
        -1,  1, 0.5f, 0, 1,
        1, -1, 0.5f, 1, 0,
        1,  1, 0.5f, 1, 1
    };
    
    KCL::uint32 vertex_buffer_size = sizeof(float) * 20 ;
#if TARGET_OS_EMBEDDED
    m_VertexBuffer = [m_Device newBufferWithLength:vertex_buffer_size
                                               options:0];
    m_VertexBuffer.label = @"VertexBuffer" ;
    float *pVertices = (float *)[m_VertexBuffer contents];
    memcpy(pVertices, screenBillboard, vertex_buffer_size);
#else
    m_VertexBuffer = [m_Device newBufferWithBytes:screenBillboard length:vertex_buffer_size options:MTLStorageModeManaged];
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
    m_IndexBuffer = [m_Device newBufferWithBytes:billboardIndices length:index_buffer_size options:MTLStorageModeManaged];
    m_IndexBuffer.label = @"IndexBuffer" ;
#endif
    if (!m_IndexBuffer)
    {
        return KCL::KCL_TESTERROR_VBO_ERROR;
    }
    
    
    /*
     GLB::OpenGLStateManager::GlBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
     GLB::OpenGLStateManager::GlEnable(GL_BLEND);
     GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
     GLB::OpenGLStateManager::GlBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     */
    
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
    pPipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
    
    pPipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    pPipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    
    MTLBlendFactor SourceFactor = MTLBlendFactorSourceAlpha ;
    MTLBlendFactor DestFactor = MTLBlendFactorOneMinusSourceAlpha ;
    
    pPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = SourceFactor;
    pPipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = SourceFactor;
    
    pPipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = DestFactor;
    pPipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = DestFactor;
    
    
    NSError *pError = nil;
    m_PipelineState = [m_Device newRenderPipelineStateWithDescriptor:pPipelineStateDescriptor
                                                               error:&pError];
    
    
    //
    //  cause measurment error if placed in the render method
    //
    if (m_scaleX == 0)
    {
        m_scaleX = 1.0f / m_Viewport.width;
        m_scaleY = 1.0f / m_Viewport.height;
        m_aspectRatio = (float)(m_Viewport.width) / m_Viewport.height;
        
        createItems(m_Viewport.width, m_Viewport.height);
    }
    
    return KCL::KCL_TESTERROR_NOERROR;
}


void UITest_Metal::FreeResources()
{
    // wait for completion
    id <MTLCommandBuffer> finishBuffer = [m_CommandQueue commandBuffer];
    [finishBuffer commit] ;
    [finishBuffer waitUntilCompleted] ;
    
    releaseObj(m_VertexBuffer) ;
    releaseObj(m_IndexBuffer) ;
    
    releaseObj(m_DepthState) ;
    releaseObj(m_PipelineState) ;
    
    FreeShaderLibraries();
    
    releaseObj(m_CommandQueue) ;
    releaseObj(m_Device) ;
    
    delete m_dynamic_data_buffer_pool ;
    
    NSLog(@"Blending Free Resource") ;
}


MTLRenderPassDescriptor * UITest_Metal::getRenderPassDescriptor(id<MTLTexture> texture)
{
    MTLRenderPassDescriptor * m_pClearPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    
    m_pClearPassDescriptor.colorAttachments[0].texture = texture ;
    m_pClearPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear ;
    m_pClearPassDescriptor.colorAttachments[0].clearColor = m_ClearColor;
    
    
    m_pClearPassDescriptor.depthAttachment.texture = nil ;
    m_pClearPassDescriptor.depthAttachment.loadAction = MTLLoadActionDontCare ;
    m_pClearPassDescriptor.depthAttachment.clearDepth = m_ClearDepth ;
    
    return m_pClearPassDescriptor;
}


bool UITest_Metal::render()
{
    @autoreleasepool {
        m_ClearColor = MTLClearColorMake(m_clearColor.x, m_clearColor.y, m_clearColor.z, 1.0f);
    
        id <MTLTexture> frameBufferTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
        MTLRenderPassDescriptor *ClearRenderPassDescriptor = getRenderPassDescriptor (frameBufferTexture);
        
        
        m_dynamic_data_buffer_pool->InitFrame() ;
        id <MTLCommandBuffer> commandBuffer = [m_CommandQueue commandBuffer];
        
        // Get a render encoder
        id <MTLRenderCommandEncoder>  renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:ClearRenderPassDescriptor];
        
        // Encode into a renderer
        [renderEncoder setViewport:m_Viewport];
        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        
        
        for (int i = 0; i < m_displayedElementCount; i++)
        {
            TestUIElement* e = m_uiElements[i % m_uiElements.size()] ;
            TestUIElement* element = dynamic_cast<TestUIElement*>( e );
            if (!element->m_texture)
            {
                element->LoadTexture();
            }
            
            float param = m_time * 0.0001 * (5 + (i & 11));
            float offsetX = 0.2f * m_scaleX * element->m_width * cos(param);
            float offsetY = 0.2f * m_scaleY * element->m_height * sin(param);
            
            MetalRender::Texture* mtl_texture = static_cast<MetalRender::Texture*>(element->m_texture) ;
            mtl_texture->Set(renderEncoder, 0) ;
            
            VertexUniforms vertexUniforms ;
            vertexUniforms.u_scale  = KCL::Vector4D(m_scaleX * element->m_width, m_scaleY * element->m_height,0.0,0.0) ;
            vertexUniforms.u_offset = KCL::Vector4D(offsetX + element->m_positionX, offsetY + element->m_positionY,0.0,0.0) ;
            
            
            m_dynamic_data_buffer->WriteAndSetData<true, false>(renderEncoder, 1, &vertexUniforms, sizeof(VertexUniforms)) ;
            
            [renderEncoder setDepthStencilState:m_DepthState];
            
            [renderEncoder setRenderPipelineState:m_PipelineState];
            
            [renderEncoder setVertexBuffer:m_VertexBuffer
                                    offset:0
                                   atIndex:0 ];
            
            [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                      indexCount:6
                                       indexType:MTLIndexTypeUInt16
                                     indexBuffer:m_IndexBuffer
                               indexBufferOffset:0];
            
            
            m_transferredBytes += (element->m_width * element->m_height) << 2;
            
            /*
            if (glGetError())
            {
                while (glGetError());
                break;
            }
            else
            {
                m_transferredBytes += (element->m_width * element->m_height) << 2;
            }
            */
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


UITest_Base *CreateUITestMetal(const GlobalTestEnvironment* const gte)
{
    return new UITest_Metal(gte) ;
}




