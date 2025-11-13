/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#include <kcl_io.h>

#include "mtl_framebuffer_30.h"
#include "mtl_globals.h"
#include "kcl_scene_version.h"

#include "mtl_types.h"
#import "graphics/metalgraphicscontext.h"

using namespace MetalRender;

Framebuffer::Framebuffer(id<MTLTexture> finalColorBuffer,
                               NSUInteger width,
                               NSUInteger height,
                               NSUInteger sampleCount,
                               MTLPixelFormat main_buffer_format,
                               KCL::uint32 scene_version) :
m_width(width),
m_height(height),
m_drawBuffer(),
m_depthBuffer(nil),
m_lightSkyColorBuffer(nil),
m_mainColorBuffer(nil),
m_finalColorBuffer(finalColorBuffer),
m_GBuffer(nil),
m_lightSkyFrameBuffer(nil),
m_mainFrameBuffer(nil),
m_finalFrameBuffer(nil),
m_linearSampler(nil),
m_mipLinearSampler(nil),
m_quadBuffer(NULL),
m_Device(MetalRender::GetContext()->getDevice())
{
	__builtin_printf("Framebuffer width %lu height %lu\n", m_width, m_height);

    ////////////////////////////////////////////////////////////////
    // Set up depth buffer used in both GBuffer and finalBuffer
    ////////////////////////////////////////////////////////////////
    MTLTextureDescriptor *depthDesc =
    [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                      width:m_width
                                                     height:m_height
                                                  mipmapped:NO];

    depthDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_IPHONE
    depthDesc.storageMode = MTLStorageModePrivate;
#endif

    m_depthBuffer =  [m_Device newTextureWithDescriptor:depthDesc];

    releaseObj(depthDesc);


    ///%%%%%%%%%%%%%
    // Setup GBuffer
    ///%%%%%%%%%%%%%
    {
        m_GBuffer = [[MTLRenderPassDescriptor alloc] init];

        // Set up color buffer attachements
        ///////////////////////////////////
        MTLTextureDescriptor *colorDesc =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                          width:m_width
                                                         height:m_height
                                                      mipmapped:NO];
        
        colorDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_IPHONE
		colorDesc.storageMode = MTLStorageModePrivate;
#endif

        for(uint buffer=0; buffer < NUM_GBUFFER_COLOR_BUFFERS; buffer++)
        {
            m_drawBuffer[buffer] =  [m_Device newTextureWithDescriptor:colorDesc];

			MTLRenderPassColorAttachmentDescriptor *drawBufferAttachment = m_GBuffer.colorAttachments[buffer];
			drawBufferAttachment.texture = m_drawBuffer[buffer];
			drawBufferAttachment.loadAction = MTLLoadActionClear;
            drawBufferAttachment.clearColor = MTLClearColorMake(0.0,0.0,0.0,0.0);
			drawBufferAttachment.storeAction = MTLStoreActionStore;
        }

        releaseObj(colorDesc);
		
		MTLRenderPassDepthAttachmentDescriptor *depthBufferAttachment = m_GBuffer.depthAttachment;
		depthBufferAttachment.texture = m_depthBuffer;

        // Depth buffer must be cleared at beginning of GBuffer pass
        depthBufferAttachment.loadAction = MTLLoadActionClear;

        // Clear depth to 1.0 (generally use less depth test)
        depthBufferAttachment.clearDepth = 1.0;

        // Depth buffer must be stored after GBuffer pass
        depthBufferAttachment.storeAction = MTLStoreActionStore;
    }

    //%%%%%%%%%%%%%%%%%%%%%%%
    // Setup Light Sky Buffer
    //%%%%%%%%%%%%%%%%%%%%%%%
    {
        m_lightSkyFrameBuffer = [[MTLRenderPassDescriptor alloc] init];

        MTLTextureDescriptor *lightSkyTexDesc =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:main_buffer_format
                                                          width:m_width
                                                         height:m_height
                                                      mipmapped:NO];
        
        lightSkyTexDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_IPHONE
		lightSkyTexDesc.storageMode = MTLStorageModePrivate;
#endif

        m_lightSkyColorBuffer =  [m_Device newTextureWithDescriptor:lightSkyTexDesc];

        releaseObj(lightSkyTexDesc);

		MTLRenderPassColorAttachmentDescriptor *lightSkyColorAttachment = m_lightSkyFrameBuffer.colorAttachments[0];
		lightSkyColorAttachment.texture = m_lightSkyColorBuffer;
		lightSkyColorAttachment.loadAction = MTLLoadActionClear;
        lightSkyColorAttachment.clearColor = MTLClearColorMake(0.0,0.0,0.0,0.0);
        lightSkyColorAttachment.storeAction = MTLStoreActionStore;

		MTLRenderPassDepthAttachmentDescriptor *lightSkyDepthAttachment = m_lightSkyFrameBuffer.depthAttachment;
        lightSkyDepthAttachment.texture = m_depthBuffer;
        lightSkyDepthAttachment.loadAction = MTLLoadActionLoad;
        lightSkyDepthAttachment.storeAction = MTLStoreActionDontCare;
    }
    
    //%%%%%%%%%%%%%%%%%%
    // Setup Main Buffer
    //%%%%%%%%%%%%%%%%%%
    {
        m_mainFrameBuffer = [[MTLRenderPassDescriptor alloc] init];

        bool mipmapped = (scene_version == KCL::SV_30) ;
        
        MTLTextureDescriptor *mainTexDesc =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:main_buffer_format
                                                          width:m_width
                                                         height:m_height
                                                      mipmapped:mipmapped];
        
        if (mipmapped)
        {
            mainTexDesc.mipmapLevelCount=4 ;
        }
        
        mainTexDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_IPHONE
		mainTexDesc.storageMode = MTLStorageModePrivate;
#endif

        m_mainColorBuffer =  [m_Device newTextureWithDescriptor:mainTexDesc];

        releaseObj(mainTexDesc);

		MTLRenderPassColorAttachmentDescriptor *mainColorAttachment = m_mainFrameBuffer.colorAttachments[0];
        mainColorAttachment.texture = m_mainColorBuffer;
        mainColorAttachment.storeAction = MTLStoreActionStore;
        mainColorAttachment.loadAction = MTLLoadActionDontCare;

        MTLRenderPassDepthAttachmentDescriptor *mainDepthAttachment = m_mainFrameBuffer.depthAttachment;
        mainDepthAttachment.texture = m_depthBuffer;
        mainDepthAttachment.loadAction = MTLLoadActionLoad;
        mainDepthAttachment.storeAction = MTLStoreActionDontCare;
    }


    //%%%%%%%%%%%%%%%%%%%%
    // Setup Bloom Buffers
    //%%%%%%%%%%%%%%%%%%%%
    
    if (scene_version == KCL::SV_30)
    {
        for(int divisor = 2; divisor<=16; divisor *= 2)
        {
            BloomLevel level;

            for(int bloomDir = 0; bloomDir < NUM_BLOOM_DIRECTIONS; bloomDir++)
            {
                NSUInteger width = m_width/divisor;
                NSUInteger height = m_height/divisor;
                level.framebuffer[bloomDir] = [[MTLRenderPassDescriptor alloc] init];

                MTLTextureDescriptor *bloomTexDesc =
                [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                  width:width
                                                                 height:height
                                                              mipmapped:NO];
                
                bloomTexDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_IPHONE
				bloomTexDesc.storageMode = MTLStorageModePrivate;
#endif

                level.texture[bloomDir] = [m_Device newTextureWithDescriptor:bloomTexDesc];

				MTLRenderPassColorAttachmentDescriptor *bloomColorAttachment = level.framebuffer[bloomDir].colorAttachments[0];
                bloomColorAttachment.texture = level.texture[bloomDir];
				bloomColorAttachment.loadAction = MTLLoadActionDontCare;
                bloomColorAttachment.storeAction = MTLStoreActionStore;

                releaseObj(bloomTexDesc);
            }

            m_bloomLevels.push_back(level);
        }
    }

    //%%%%%%%%%%%%%%%%%%%%%%%%
    // Setup final framebuffer
    //%%%%%%%%%%%%%%%%%%%%%%%%
    {
        m_finalFrameBuffer = [[MTLRenderPassDescriptor alloc] init];
		MTLRenderPassColorAttachmentDescriptor *finalColorAttachment = m_finalFrameBuffer.colorAttachments[0];

        // Set up color Attachement for final framebuffer
        /////////////////////////////////////////////////
        finalColorAttachment.loadAction = MTLLoadActionDontCare;
        finalColorAttachment.storeAction = MTLStoreActionStore;
        finalColorAttachment.texture = m_finalColorBuffer;

        // Set up depth Attachement for final framebuffer
        /////////////////////////////////////////////////
        MTLRenderPassDepthAttachmentDescriptor *finalDepthAttachement = m_finalFrameBuffer.depthAttachment;

        // Final frame buffer has not depth attachment
		finalDepthAttachement.texture = nil;
        finalDepthAttachement.loadAction = MTLLoadActionDontCare;
        finalDepthAttachement.storeAction = MTLStoreActionDontCare;

        if(sampleCount)
        {
            assert(!"MTL_TODO: Implemment multisample");
        }
    }
	
	{
		MTLSamplerDescriptor *samplerDesc = [[MTLSamplerDescriptor alloc] init];
		samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
		samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
		samplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;
        
        
		samplerDesc.maxAnisotropy = 1;
		samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
		samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
		samplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
		samplerDesc.normalizedCoordinates = YES;
		samplerDesc.lodMinClamp = 0;
		samplerDesc.lodMaxClamp = FLT_MAX;
		
		m_linearSampler = [m_Device newSamplerStateWithDescriptor:samplerDesc];

		samplerDesc.mipFilter = MTLSamplerMipFilterLinear;
		samplerDesc.lodMaxClamp = 4;
		
		m_mipLinearSampler = [m_Device newSamplerStateWithDescriptor:samplerDesc];
		
		releaseObj(samplerDesc);
	}
	
#if defined(BLIT_GBUFFER) || defined(BLIT_MAINBUFFER)
    SetupBlitBuffer();
#endif //  defined(BLIT_GBUFFER) || defined(BLIT_MAINBUFFER)

    SetupBlitObjects();
}


void Framebuffer::CreateOcclusionQueryFrameBuffer(OcclusionQueryBuffer* occlusionQueries, NSUInteger queryCount)
{
#pragma mark Occlusion Query framebuffers
    {
        MTLRenderPassDepthAttachmentDescriptor* occlusionDepthAttachment = [[MTLRenderPassDepthAttachmentDescriptor alloc] init];
        occlusionDepthAttachment.texture = m_depthBuffer;
        occlusionDepthAttachment.loadAction = MTLLoadActionLoad;
        occlusionDepthAttachment.storeAction = MTLStoreActionDontCare;
        
        for(int i = 0; i < queryCount; i++)
        {
            m_occlusionQueryFramebuffer[i] = [[MTLRenderPassDescriptor alloc] init];
            m_occlusionQueryFramebuffer[i].colorAttachments[0] = nil;
            m_occlusionQueryFramebuffer[i].depthAttachment = occlusionDepthAttachment;
            m_occlusionQueryFramebuffer[i].visibilityResultBuffer = occlusionQueries[i].m_queryResults;
        }
        releaseObj(occlusionDepthAttachment);
    }
}


Framebuffer::~Framebuffer()
{
    delete m_quadBuffer;

    releaseObj(m_depthBuffer);

    for(uint buffer=0; buffer < NUM_GBUFFER_COLOR_BUFFERS; buffer++)
    {
        releaseObj(m_drawBuffer[buffer]);
    }

    releaseObj(m_lightSkyColorBuffer);
    releaseObj(m_mainColorBuffer);

    releaseObj(m_GBuffer);
    releaseObj(m_lightSkyFrameBuffer);
    releaseObj(m_mainFrameBuffer);
    releaseObj(m_finalFrameBuffer);

    releaseObj(m_linearSampler);
    releaseObj(m_mipLinearSampler);
	
#if defined(BLIT_GBUFFER) || defined(BLIT_MAINBUFFER)
    releaseObj(m_blitBuf);
    releaseObj(m_blitGbufferVS);
    releaseObj(m_blitGbufferFS);
    releaseObj(m_blitPipeline);
#endif // BLIT_GBUFFER

    for(uint bloomLevel = 0; bloomLevel < m_bloomLevels.size(); bloomLevel++)
    {
        for(int bloomDir = 0; bloomDir < NUM_BLOOM_DIRECTIONS; bloomDir++)
        {
            releaseObj(m_bloomLevels[bloomLevel].texture[bloomDir]);
            releaseObj(m_bloomLevels[bloomLevel].framebuffer[bloomDir]);
        }
    }
}

void Framebuffer::UpdateFinalFrameBufferColorBuffer(id <MTLTexture> finalColorBuffer)
{
	assert(finalColorBuffer);
	
    m_finalColorBuffer = finalColorBuffer;

    // Re-setup final framebuffer
    ////////////////////////////////////////////////////////////////
    m_finalFrameBuffer.colorAttachments[0].texture = m_finalColorBuffer;
}

void Framebuffer::SetupBlitObjects()
{
	@autoreleasepool
	{
		KCL::AssetFile shader_file("shaders_mtl/shaders.30/FlipBlit.metal");
		
		if(shader_file.GetLastError())
		{
			NSLog(@"ERROR: blit shader not found!\n");
			assert(0);
		}
			
		NSString* flipBlitSource = [NSString stringWithUTF8String:shader_file.GetBuffer()];
		
		NSError* err = nil;
		m_blitLib = [m_Device newLibraryWithSource:flipBlitSource options:nil error:&err];
		if(m_blitLib == nil)
		{
			NSLog(@"Error loading blit shader %@", [err localizedDescription]);
			assert(0);
		}
		
		m_blitGbufferVS = [m_blitLib newFunctionWithName:@"vertex_main"];
		m_blitGbufferFS = [m_blitLib newFunctionWithName:@"fragment_main"];

		assert(m_blitGbufferFS && m_blitGbufferVS);

		MTLRenderPipelineDescriptor* pipeDesc = [[MTLRenderPipelineDescriptor alloc] init];
		pipeDesc.label = @"TexturedQuad Pipeline";
		pipeDesc.vertexFunction = m_blitGbufferVS;
		pipeDesc.fragmentFunction = m_blitGbufferFS;
        

		pipeDesc.colorAttachments[0].pixelFormat = m_finalColorBuffer.pixelFormat;
			
		err = nil;
		m_blitPipeline = [m_Device newRenderPipelineStateWithDescriptor:pipeDesc error:&err];
		assert(m_blitPipeline);
		releaseObj(pipeDesc);

        m_quadBuffer = new QuadBuffer(QuadBuffer::kBlitQuadLandscape);
	}
}

void Framebuffer::BlitGBuffer(id <MTLCommandBuffer> commandBuffer)
{
    id <MTLRenderCommandEncoder> renderEncoder = this->SetFinalBufferAsTargetAndGetEncoder(commandBuffer);

    [renderEncoder setRenderPipelineState:m_blitPipeline];

    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
    depthStateDesc.depthWriteEnabled = NO;

    id<MTLDepthStencilState> depthState = [m_Device newDepthStencilStateWithDescriptor:depthStateDesc];
    releaseObj(depthStateDesc);


    [renderEncoder setDepthStencilState:depthState];

    releaseObj(depthState);

    [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:0];

    MTLViewport viewports[4] = {
        {            0,             0, m_width/2.0f, m_height/2.0f, 0.0, 1.0},
        { m_width/2.0f,             0, m_width/2.0f, m_height/2.0f, 0.0, 1.0},
        { m_width/2.0f, m_height/2.0f, m_width/2.0f, m_height/2.0f, 0.0, 1.0},
        {            0, m_height/2.0f, m_width/2.0f, m_height/2.0f, 0.0, 1.0}
    };

    for(int drawBufferIdx = 0; drawBufferIdx < 4; drawBufferIdx++)
    {
        [renderEncoder setFragmentTexture:m_drawBuffer[drawBufferIdx] atIndex:0];

        [renderEncoder setViewport:viewports[drawBufferIdx]];

        m_quadBuffer->Draw(renderEncoder);
    }

    [renderEncoder endEncoding];
}

void Framebuffer::BlitMainBuffer(id <MTLCommandBuffer> commandBuffer)
{
    id <MTLRenderCommandEncoder> renderEncoder = this->SetFinalBufferAsTargetAndGetEncoder(commandBuffer);
	
	MTLViewport viewport = { 0, 0, float(m_width), float(m_height), 0.0, 1.0 };
    
	[renderEncoder setViewport:viewport];

    [renderEncoder setRenderPipelineState:m_blitPipeline];

    [renderEncoder setVertexBuffer:m_blitBuf offset:0 atIndex:0];

    [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:0];
	
    [renderEncoder setFragmentTexture:m_mainColorBuffer atIndex:0];

    m_quadBuffer->Draw(renderEncoder);

    [renderEncoder endEncoding];

}


