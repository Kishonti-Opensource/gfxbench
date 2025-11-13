/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "newnotificationmanager_metal.h"
#include "kcl_os.h"
#include "mtl_texture.h"
#include "mtl_types.h"
#include "mtl_globals.h"
#include "fbo_metal.h"

#import "graphics/metalgraphicscontext.h"

namespace GLB {
    
    struct NNMVertexUniforms
    {
        KCL::Vector2D texScale ;
        
        float __pad1 ;
        float __pad2 ;
    };
		
	NewNotificationManager* NewNotificationManager::NewInstance()
	{
        return new NewNotificationManagerMetal();
	}
	
	NewNotificationManagerMetal::NewNotificationManagerMetal()
	{		
       
        m_Device = MetalRender::GetContext()->getDevice() ;
        m_CommandQueue = MetalRender::GetContext()->getMainCommandQueue() ;

        m_landscape_full_screen_quad = new MetalRender::QuadBuffer(MetalRender::QuadBuffer::kBlitQuadLandscape) ;
        m_portrait_full_screen_quad = new MetalRender::QuadBuffer(MetalRender::QuadBuffer::kBlitQuadPortrait) ;
        
        m_dynamic_data_buffer_pool = new MetalRender::DynamicDataBufferPool(2*MetalRender::METAL_MAX_FRAME_LAG) ;
        m_dynamic_data_buffer = m_dynamic_data_buffer_pool->GetNewBuffer(4*1024) ;
        
        
        //
        //  Load shaders
        //
        id <MTLLibrary> ShaderLibrary = MetalRender::LoadShaderLibraryFromFile(m_Device,"shaders_mtl/common/newnotificationmanager.metal") ;
        
        // load the fragment and vertex program into the library
        id <MTLFunction> fragment_program = [ShaderLibrary newFunctionWithName:@"NNMFragment"];
        id <MTLFunction> vertex_program = [ShaderLibrary newFunctionWithName:@"NNMVertex"];
        
        releaseObj(ShaderLibrary) ;
        
        
        //
        // Depth State
        //

        MTLDepthStencilDescriptor *pDepthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
        pDepthStateDesc.depthWriteEnabled    = false;
        pDepthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
        m_DepthState = [m_Device newDepthStencilStateWithDescriptor:pDepthStateDesc];
        
        
        MTLRenderPipelineDescriptor *pPipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        
        
        pPipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        pPipelineStateDescriptor.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
        
        [pPipelineStateDescriptor setVertexFunction:vertex_program];
        [pPipelineStateDescriptor setFragmentFunction:fragment_program];
        
        
        
        //
        // NoBlending
        //
        {
            pPipelineStateDescriptor.colorAttachments[0].blendingEnabled = NO;
            
            
            NSError *pError = nil;
            m_NoBlendPipelineState = [m_Device newRenderPipelineStateWithDescriptor:pPipelineStateDescriptor
                                                                       error:&pError];
        }
        
        //
        // Blending
        //
        {
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
            m_BlendPipelineState = [m_Device newRenderPipelineStateWithDescriptor:pPipelineStateDescriptor
                                                                       error:&pError];

        }

	}

	NewNotificationManagerMetal::~NewNotificationManagerMetal()
	{

        id <MTLCommandBuffer> finishBuffer = [m_CommandQueue commandBuffer];
        [finishBuffer commit] ;
        [finishBuffer waitUntilCompleted] ;
        
        releaseObj(m_CommandQueue) ;
        
        releaseObj(m_DepthState) ;
        releaseObj(m_BlendPipelineState) ;
        releaseObj(m_NoBlendPipelineState) ;
        
        delete m_portrait_full_screen_quad ;
        delete m_landscape_full_screen_quad ;
        delete m_dynamic_data_buffer_pool ;
	}
    
    
    MTLRenderPassDescriptor * NewNotificationManagerMetal::getRenderPassDescriptor(id<MTLTexture> texture)
    {
        MTLRenderPassDescriptor * m_pClearPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        
        m_pClearPassDescriptor.colorAttachments[0].texture = texture ;
        m_pClearPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear ;
        m_pClearPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0f, 0.0f, 0.0f, 0.0f);
        
        m_pClearPassDescriptor.depthAttachment.texture = nil ;
        m_pClearPassDescriptor.depthAttachment.loadAction = MTLLoadActionLoad ;
        m_pClearPassDescriptor.depthAttachment.clearDepth = 1.0 ;
        
        return m_pClearPassDescriptor;
    }
    

	void NewNotificationManagerMetal::ShowLogo(bool stretch, bool blend)
	{
		
        @autoreleasepool {
            
            int texHeight = m_texture->getHeight() ;
            int texWidth = m_texture->getWidth() ;
            
            id <MTLTexture> frameBufferTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
            MTLRenderPassDescriptor *NNMRenderPassDescriptor = getRenderPassDescriptor (frameBufferTexture);
            
            m_dynamic_data_buffer_pool->InitFrame() ;
            id <MTLCommandBuffer> commandBuffer = [m_CommandQueue commandBuffer];
            
            // Get a render encoder
            id <MTLRenderCommandEncoder>  renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:NNMRenderPassDescriptor];
            
            // Encode into a renderer
            MTLViewport viewport = {0.0, 0.0, (double)frameBufferTexture.width, (double)frameBufferTexture.height, 0.0, 1.0};
            
            NNMVertexUniforms vu ;
            
            vu.texScale.x = 0.5 ;
            vu.texScale.y = 0.5 ;
            
            if (!stretch)
            {
                if (frameBufferTexture.width >= frameBufferTexture.height)
                {
                    vu.texScale.x *= viewport.width/texWidth ;
                    vu.texScale.y *= viewport.height/texHeight ;
                }
                else
                {
                    vu.texScale.y *= viewport.width/texWidth ;
                    vu.texScale.x *= viewport.height/texHeight ;
                }
            }
            
            [renderEncoder setViewport:viewport];
            [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
            
            
            MetalRender::Texture* mtl_texture = static_cast<MetalRender::Texture*>(m_texture) ;
            mtl_texture->Set(renderEncoder, 0) ;
            
            m_dynamic_data_buffer->WriteAndSetData<true, false>(renderEncoder, 1, &vu, sizeof(NNMVertexUniforms)) ;
            
            [renderEncoder setDepthStencilState:m_DepthState];
            
            if (blend)
            {
                [renderEncoder setRenderPipelineState:m_BlendPipelineState];
            }
            else
            {
                [renderEncoder setRenderPipelineState:m_NoBlendPipelineState] ;
            }
                
            
            if (frameBufferTexture.width >= frameBufferTexture.height)
            {
                m_landscape_full_screen_quad->Draw(renderEncoder) ;
            }
            else
            {
                m_portrait_full_screen_quad->Draw(renderEncoder) ;
            }
            
            
            [renderEncoder endEncoding];
            renderEncoder = nil ;
            
            unsigned char slot = m_dynamic_data_buffer_pool->GetCurrentSlot();
            
            [commandBuffer addCompletedHandler:^(id <MTLCommandBuffer> completedCommandBuffer)
             {
                 m_dynamic_data_buffer_pool->MarkSlotUnused(slot);
             }];

            
            [commandBuffer commit];
            commandBuffer = nil;
            
        }
    }
	
	KCL::Texture *NewNotificationManagerMetal::CreateTexture(const KCL::Image* img, bool releaseUponCommit)
	{
		MetalRender::TextureFactory f;
		return f.CreateTexture(img, releaseUponCommit);
	}
}
