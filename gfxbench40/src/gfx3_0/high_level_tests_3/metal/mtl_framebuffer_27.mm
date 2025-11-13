/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#import "mtl_framebuffer_27.h"

#include "mtl_globals.h"
#include "mtl_types.h"
#include "graphics/metalgraphicscontext.h"

using namespace MetalRender ;

    
Framebuffer27::Framebuffer27(KCL::uint32 width, KCL::uint32 height, bool motion_blur_enabled)
{
    m_Device = MetalRender::GetContext()->getDevice() ;
    
    m_width  = width ;
    m_height = height ;
    
    ////////////////////////////////////////////////////////////////
    // Set up depth buffer used in both GBuffer and finalBuffer
    ////////////////////////////////////////////////////////////////
    MTLTextureDescriptor *depthDesc =
    [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                       width:m_width
                                                      height:m_height
                                                   mipmapped:NO];
    
    depthDesc.usage = MTLTextureUsageRenderTarget;
#if !TARGET_OS_IPHONE
    depthDesc.storageMode = MTLStorageModePrivate;
#endif
    
    m_mainDepthBuffer =  [m_Device newTextureWithDescriptor:depthDesc];
    
    releaseObj(depthDesc);
    
    
    
    //%%%%%%%%%%%%%%%%%%%%%%%%
    // Setup final framebuffer
    //%%%%%%%%%%%%%%%%%%%%%%%%
    {
        m_finalFrameBuffer = [[MTLRenderPassDescriptor alloc] init];
        MTLRenderPassColorAttachmentDescriptor *finalColorAttachment = m_finalFrameBuffer.colorAttachments[0];
        
        // Set up color Attachement for final framebuffer
        /////////////////////////////////////////////////
        finalColorAttachment.texture = nil;
        finalColorAttachment.loadAction = MTLLoadActionDontCare;
        finalColorAttachment.storeAction = MTLStoreActionStore;
        //finalColorAttachment.clearColor = MTLClearColorMake(0.0,0.0,0.0,1.0);

       
        
        // Set up depth Attachement for final framebuffer
        /////////////////////////////////////////////////
        MTLRenderPassDepthAttachmentDescriptor *finalDepthAttachment = m_finalFrameBuffer.depthAttachment;
        
        if (motion_blur_enabled)
        {
            finalDepthAttachment.texture = nil ;
            finalDepthAttachment.loadAction = MTLLoadActionDontCare ;
            finalDepthAttachment.storeAction = MTLStoreActionDontCare;
        }
        else
        {
            finalDepthAttachment.texture = m_mainDepthBuffer ;
            finalDepthAttachment.loadAction = MTLLoadActionClear ;
            finalDepthAttachment.storeAction = MTLStoreActionDontCare;
            finalDepthAttachment.clearDepth = 1.0;
        }
    }
    
    //%%%%%%%%%%%%%%%%%%%%%%%%
    // Motion Blur framebuffer
    //%%%%%%%%%%%%%%%%%%%%%%%%
    
    m_mainColorBuffer = nil ;
    
    if (motion_blur_enabled)
    {
        MTLTextureDescriptor *motionBlurColorDesc =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MAIN_FRAME_BUFFER_FORMAT
                                                           width:m_width
                                                          height:m_height
                                                       mipmapped:NO];

        motionBlurColorDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_IPHONE
		motionBlurColorDesc.storageMode = MTLStorageModePrivate;
#endif
        
        m_mainColorBuffer =  [m_Device newTextureWithDescriptor:motionBlurColorDesc];
        
        releaseObj(motionBlurColorDesc);

        
        m_mainBuffer = [[MTLRenderPassDescriptor alloc] init];
        MTLRenderPassColorAttachmentDescriptor *mainBufferColorAttachment = m_mainBuffer.colorAttachments[0];
        

        mainBufferColorAttachment.texture = m_mainColorBuffer;
        mainBufferColorAttachment.loadAction = MTLLoadActionClear;
        mainBufferColorAttachment.storeAction = MTLStoreActionStore;
        mainBufferColorAttachment.clearColor = MTLClearColorMake(0.0,0.0,0.0,1.0);
        
        
        MTLRenderPassDepthAttachmentDescriptor *mainDepthAttachment = m_mainBuffer.depthAttachment;
        
        mainDepthAttachment.texture = m_mainDepthBuffer ;
        mainDepthAttachment.loadAction = MTLLoadActionClear ;
        mainDepthAttachment.storeAction = MTLStoreActionDontCare;
        mainDepthAttachment.clearDepth = 1.0;
    }
    
    //%%%%%%%%%%%%%%%%%%%%%%%%
    // Velocity buffer
    //%%%%%%%%%%%%%%%%%%%%%%%%
    
    m_velocityColorBuffer = nil ;
    
    if (motion_blur_enabled)
    {
        MTLTextureDescriptor *velocityColorDesc =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MAIN_FRAME_BUFFER_FORMAT
                                                           width:m_width
                                                          height:m_height
                                                       mipmapped:NO];
        
        velocityColorDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_IPHONE
		velocityColorDesc.storageMode = MTLStorageModePrivate;
#endif
        
        m_velocityColorBuffer =  [m_Device newTextureWithDescriptor:velocityColorDesc];
        
        releaseObj(velocityColorDesc);
        
        
        MTLTextureDescriptor *velocityDepthDesc =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                           width:m_width
                                                          height:m_height
                                                       mipmapped:NO];
        
        velocityDepthDesc.usage = MTLTextureUsageRenderTarget;
#if !TARGET_OS_IPHONE
        velocityDepthDesc.storageMode = MTLStorageModePrivate;
#endif
        
        m_velocityDepthBuffer =  [m_Device newTextureWithDescriptor:velocityDepthDesc];
        
        releaseObj(velocityDepthDesc);

        
        m_velocityBuffer = [[MTLRenderPassDescriptor alloc] init];
        MTLRenderPassColorAttachmentDescriptor *velocityBufferColorAttachment = m_velocityBuffer.colorAttachments[0];
        

        velocityBufferColorAttachment.texture = m_velocityColorBuffer;
        velocityBufferColorAttachment.loadAction = MTLLoadActionClear;
        velocityBufferColorAttachment.storeAction = MTLStoreActionStore;
        velocityBufferColorAttachment.clearColor = MTLClearColorMake(0.5,0.5,0.0,1.0);
        
        
        MTLRenderPassDepthAttachmentDescriptor *velocityDepthAttachment = m_velocityBuffer.depthAttachment;
        
        velocityDepthAttachment.texture = m_velocityDepthBuffer ;
        velocityDepthAttachment.loadAction = MTLLoadActionClear ;
        velocityDepthAttachment.storeAction = MTLStoreActionDontCare;
        velocityDepthAttachment.clearDepth = 1.0;
    }

}


Framebuffer27::~Framebuffer27()
{
    releaseObj(m_finalFrameBuffer);
    releaseObj(m_mainBuffer);
    releaseObj(m_velocityBuffer) ;
    
    releaseObj(m_mainDepthBuffer);
    releaseObj(m_velocityDepthBuffer);
    
    releaseObj(m_mainColorBuffer);
    releaseObj(m_velocityColorBuffer);
}


id <MTLRenderCommandEncoder> Framebuffer27::SetFinalBufferAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer, id <MTLTexture> finalColorBuffer)
{
    m_finalFrameBuffer.colorAttachments[0].texture = finalColorBuffer ;
    
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:m_finalFrameBuffer];
    
    [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    
    return renderEncoder;
}


id <MTLRenderCommandEncoder> Framebuffer27::SetMainBufferAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer)
{
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:m_mainBuffer];
    
    [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    
    return renderEncoder;
}


id <MTLRenderCommandEncoder> Framebuffer27::SetVelocityBufferAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer)
{
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:m_velocityBuffer];
    
    [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    
    return renderEncoder;
}


