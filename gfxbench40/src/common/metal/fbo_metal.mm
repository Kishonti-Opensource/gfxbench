/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_os.h"

#include "stdc.h"
#include "fbo.h"
#include "fbo_metal.h"
#include "kcl_image.h"

#include "graphics/metalgraphicscontext.h"
#include "mtl_globals.h"

using namespace GLB;

GLB::FBO* FBO::m_lastBound = 0;
GLB::FBO* FBO::m_originalGlobal = 0;
GLB::FBO* FBO::m_currentGlobal = 0;


//****************************************************************
//
// Internal classes
//
//****************************************************************


class DefaultFBOMetal : public FBOMetalBase
{
public:
    DefaultFBOMetal(const GlobalTestEnvironment* const gte) ;
    virtual ~DefaultFBOMetal() ;
    
    virtual id <MTLTexture> GetTexture() ;
    
    virtual id <MTLRenderCommandEncoder> SetAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer) ;
    
private:
    
};


//****************************************************************
//
//  FBO implementation
//
//****************************************************************



FBO::FBO(const GlobalTestEnvironment* const gte) :
    m_gte(gte),
	m_width (0),
	m_height (0)
{
	
}

FBO::~FBO ()
{
    
}


void FBO::InvalidateLastBound()
{
	m_lastBound = 0;
}


void FBO::SetGlobalFBO( FBO *fbo)
{
	InvalidateLastBound();
	if( !fbo)
	{
		fbo = m_originalGlobal;
	}
	m_currentGlobal = fbo;
}


FBO* FBO::GetGlobalFBO()
{
	return m_currentGlobal;
}

FBO* FBO::GetLastBind()
{
    return m_lastBound?m_lastBound:m_currentGlobal;
}

void FBO::CreateGlobalFBO(const GlobalTestEnvironment* const gte)
{
    m_originalGlobal = new DefaultFBOMetal(gte);
    m_currentGlobal = m_originalGlobal;
    INFO("CreateGlobal FBO: %p", m_originalGlobal);
    
    FBO::bind(0) ;
}


void FBO::DeleteGlobalFBO()
{
    INFO("DeleteGlobal FBO: %p", m_originalGlobal);
    delete m_originalGlobal;
}


FBO* FBO::CreateFBO(const GlobalTestEnvironment* const gte, KCL::uint32 width, KCL::uint32 height, int samples, FBO_COLORMODE color_mode, FBO_DEPTHMODE depth_mode, const char *debug_label)
{
    return new FBOMetal(gte,width,height,samples,color_mode,depth_mode,debug_label) ;
}


void FBO::bind( FBO *fbo)
{
    FBO *new_fbo = fbo;
    
    if( fbo == 0)
    {
        new_fbo = m_currentGlobal;
    }
    
    //if (m_lastBound != new_fbo)
    {
        m_lastBound = new_fbo;
    }
}

void FBO::ResetInternalState()
{
    InvalidateLastBound();
    bind( 0);
}


void FBO::Destroy()
{
    if( m_lastBound == this)
    {
        InvalidateLastBound();
    }
}


KCL::uint32 FBO::GetScreenshotImage(KCL::Image& img)
{
    FBOMetalBase* current_fbo = dynamic_cast<FBOMetalBase*>(m_currentGlobal) ;
    
    MetalGraphicsContext* metal_context = dynamic_cast<MetalGraphicsContext*>(current_fbo->m_gte->GetGraphicsContext()) ;
    id <MTLCommandQueue> commandQueue = metal_context->getMainCommandQueue() ;
    id <MTLCommandBuffer> finishBuffer = [commandQueue commandBuffer];
	
	id <MTLTexture> fbotexture = current_fbo->GetTexture() ;
	
	MTLPixelFormat pixel_format = [fbotexture pixelFormat];
	if (pixel_format != MTLPixelFormatBGRA8Unorm && pixel_format != MTLPixelFormatRGBA8Unorm)
	{
		return 0;
	}
	
	MTLTextureDescriptor* textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixel_format
																								 width:fbotexture.width
																								height:fbotexture.height
																							 mipmapped:false];
	
	id <MTLTexture> backBufferTexture = [[commandQueue device] newTextureWithDescriptor:textureDescriptor];
	
	id <MTLBlitCommandEncoder> blit_command_encoder = [finishBuffer blitCommandEncoder];
	[blit_command_encoder copyFromTexture:fbotexture
							  sourceSlice:0
							  sourceLevel:0
							 sourceOrigin:MTLOriginMake(0, 0, 0)
							   sourceSize:MTLSizeMake(fbotexture.width, fbotexture.height, 1)
								toTexture:backBufferTexture
						 destinationSlice:0
						 destinationLevel:0
						destinationOrigin:MTLOriginMake(0, 0, 0)];
#if !TARGET_OS_IPHONE
    [blit_command_encoder synchronizeResource:backBufferTexture];
#endif
    [blit_command_encoder endEncoding];
    [finishBuffer commit] ;
    [finishBuffer waitUntilCompleted] ;
    releaseObj(finishBuffer) ;
    releaseObj(commandQueue) ;
    
    img.Allocate2D(backBufferTexture.width, backBufferTexture.height, KCL::Image_RGB888);
    
    uint32_t srcRowBytes = backBufferTexture.width * 4;
    uint32_t imageBytes = srcRowBytes * backBufferTexture.height;
    
    KCL::uint8* pixelBuffer = new KCL::uint8[imageBytes] ;
    
    [backBufferTexture getBytes:pixelBuffer
                    bytesPerRow:srcRowBytes
                  bytesPerImage:imageBytes
                     fromRegion:MTLRegionMake2D(0, 0, backBufferTexture.width, backBufferTexture.height)
                    mipmapLevel:0
                          slice:0];
    
    uint32_t dstRowBytes = 3*backBufferTexture.width ;
	
	KCL::uint8* data = (KCL::uint8*)img.getData();
	for (int i = 0; i < backBufferTexture.height; i++)
	{
		for (int j = 0; j < backBufferTexture.width; j++)
		{
			uint32_t dst_id = i*dstRowBytes+3*j ;
			uint32_t src_id = i*srcRowBytes+4*j ;
			
			if (pixel_format == MTLPixelFormatRGBA8Unorm)
			{
				data[dst_id+0] = pixelBuffer[src_id+0] ;
				data[dst_id+1] = pixelBuffer[src_id+1] ;
				data[dst_id+2] = pixelBuffer[src_id+2] ;
			}
			else
			{
				data[dst_id+0] = pixelBuffer[src_id+2] ;
				data[dst_id+1] = pixelBuffer[src_id+1] ;
				data[dst_id+2] = pixelBuffer[src_id+0] ;
			}
		}
	}
    
    delete[] pixelBuffer ;
    
    return 0;
}


//****************************************************************
//
//  FBO Metal Base implementation
//
//****************************************************************


FBOMetalBase::FBOMetalBase(const GlobalTestEnvironment* const gte) : FBO(gte), m_Device(nil), m_DepthTexture(nil)
{
    m_FrameBufferDescriptor = [[MTLRenderPassDescriptor alloc] init];
    
    // Set up color Attachement for framebuffer
    /////////////////////////////////////////////////
    MTLRenderPassColorAttachmentDescriptor *colorAttachment0 =  m_FrameBufferDescriptor.colorAttachments[0];
    
    colorAttachment0.texture = nil;
    colorAttachment0.loadAction = MTLLoadActionClear;
    colorAttachment0.storeAction = MTLStoreActionStore;
    colorAttachment0.clearColor = MTLClearColorMake(0.0,0.0,0.0,1.0);
    
    
    // Set up depth Attachement for framebuffer
    /////////////////////////////////////////////////
    MTLRenderPassDepthAttachmentDescriptor *depthAttachment = m_FrameBufferDescriptor.depthAttachment;
    
    // Final frame buffer will load depth buffer produced in GBufferPass
    depthAttachment.texture = nil ;
    depthAttachment.loadAction = MTLLoadActionClear ;
    depthAttachment.storeAction = MTLStoreActionStore;
    depthAttachment.clearDepth = 1.0;
}

FBOMetalBase::~FBOMetalBase()
{
    releaseObj(m_DepthTexture) ;
    releaseObj(m_FrameBufferDescriptor) ;
}


void FBOMetalBase::AppendDepthBuffer(FBO_DEPTHMODE depth_mode)
{
    if (m_DepthTexture != nil)
    {
        NSLog(@"Depth Texture already exists!") ;
        assert(0);
    }
    
    // Metal only supports 32 bit depth textures
    
    MTLTextureDescriptor *fboDepthTexDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                       width:m_width
                                                      height:m_height
                                                   mipmapped:NO];

#if !TARGET_OS_EMBEDDED
    fboDepthTexDesc.storageMode = MTLStorageModePrivate;
#endif
    
    m_DepthTexture =  [m_Device newTextureWithDescriptor:fboDepthTexDesc];

    
    MTLRenderPassDepthAttachmentDescriptor *velocityDepthAttachment = m_FrameBufferDescriptor.depthAttachment;
    velocityDepthAttachment.texture = m_DepthTexture ;
    
    releaseObj(fboDepthTexDesc) ;
}


//****************************************************************
//
//  Default FBO Metal implementation
//
//****************************************************************


DefaultFBOMetal::DefaultFBOMetal(const GlobalTestEnvironment* const gte) : FBOMetalBase(gte)
{
    MetalGraphicsContext* metal_context = dynamic_cast<MetalGraphicsContext*>(gte->GetGraphicsContext()) ;
    
    m_Device = metal_context->getDevice() ;
    
    id <MTLTexture> backBufferTexture = metal_context->getBackBufferTexture() ;
    
    m_width  = backBufferTexture.width ;
    m_height = backBufferTexture.height ;
}

DefaultFBOMetal::~DefaultFBOMetal()
{
    Destroy() ;
}

id <MTLTexture> DefaultFBOMetal::GetTexture()
{
    MetalGraphicsContext* context = dynamic_cast<MetalGraphicsContext*>(m_gte->GetGraphicsContext()) ;
    
    return context->getBackBufferTexture() ;
}


id <MTLRenderCommandEncoder> DefaultFBOMetal::SetAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer)
{
    m_FrameBufferDescriptor.colorAttachments[0].texture = GetTexture() ;
    
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:m_FrameBufferDescriptor];
    
    return renderEncoder;

}


//****************************************************************
//
//  FBO Metal implementation
//
//****************************************************************


id <MTLRenderCommandEncoder> FBOMetal::SetAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer)
{
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:m_FrameBufferDescriptor];
    
    return renderEncoder;
}


FBOMetal::FBOMetal(const GlobalTestEnvironment* const gte, KCL::uint32 width, KCL::uint32 height, int samples, FBO_COLORMODE color_mode, FBO_DEPTHMODE depth_mode, const char *debug_label) : FBOMetalBase(gte)
{
    m_debug_label = debug_label;
    m_width = width ;
    m_height = height ;
    
    MetalGraphicsContext* metal_context = dynamic_cast<MetalGraphicsContext*>(gte->GetGraphicsContext()) ;
 
    m_Device = metal_context->getDevice() ;
    
    if( samples != 0 )
    {
        Destroy();
        throw "Multisampling not supported.";
        assert(0) ;
    }

    
    if( color_mode)
    {
        MTLPixelFormat pixelFormat = MTLPixelFormatInvalid ;
        
        
        // MTL_TODO
        switch( color_mode)
        {
            case RGB565_Linear:
            {
#if TARGET_OS_EMBEDDED
                pixelFormat = MTLPixelFormatB5G6R5Unorm ;
                
                NSLog(@"WARNING! Metal doesn't support RGB565. Using BGR565.") ;
#else
                assert(0);
#endif
                
                //m_texture->setFiltering (Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
                
                break;
            }
            case RGB888_Linear:
            {
                pixelFormat = MTLPixelFormatBGRA8Unorm ;
                
                NSLog(@"WARNING! Metal doesn't support RGB8. Using RGBA8.") ;
                NSLog(@"WARNING! Force Hardware Layer Format!") ;
                
                //m_texture->setFiltering (Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
                
                break;
            }
            case RGB888_MipMap:
            {
                pixelFormat = MTLPixelFormatBGRA8Unorm ;
                
                NSLog(@"WARNING! Metal doesn't support RGB8. Using RGBA8.") ;
                NSLog(@"WARNING! Force Hardware Layer Format!") ;
                
                //m_texture->setFiltering (Texture::FILTER_LINEAR, Texture::FILTER_LINEAR);
                
                break;
            }
            case RGB565_Nearest:
            {
#if TARGET_OS_EMBEDDED
                pixelFormat = MTLPixelFormatB5G6R5Unorm ;
                
                NSLog(@"WARNING! Metal doesn't support RGB565. Using BGR565.") ;
#else
                assert(0);
#endif
                
                //m_texture->setFiltering (Texture::FILTER_BASE_LEVEL, Texture::FILTER_NEAREST);
                
                break;
            }
            case RGBA8888_Linear:
            {
                pixelFormat = MTLPixelFormatBGRA8Unorm ;
                
                NSLog(@"WARNING! Force Hardware Layer Format!") ;
                
                //m_texture->setFiltering (Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
                
                break;
            }
            case RGBA8888_Nearest:
            {
                pixelFormat = MTLPixelFormatBGRA8Unorm ;
                
                NSLog(@"WARNING! Force Hardware Layer Format!") ;
                
                //m_texture->setFiltering (Texture::FILTER_BASE_LEVEL, Texture::FILTER_NEAREST);
                
                break;
            }
            case RGBA5551_Linear:
            {
#if TARGET_OS_EMBEDDED
                pixelFormat = MTLPixelFormatA1BGR5Unorm ;
                
                NSLog(@"WARNING! Metal doesn't support RGBA5551. Using A1BGR5.") ;
#else
                assert(0);
                
#endif

                //m_texture->setFiltering (Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
                
                break;
            }
        }
        
        
        //m_texture->setWrapping (Texture::WRAP_CLAMP, Texture::WRAP_CLAMP);
        
        MTLTextureDescriptor *fboColorTexDesc =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat
                                                           width:m_width
                                                          height:m_height
                                                       mipmapped:NO];
        
        fboColorTexDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_IPHONE
		fboColorTexDesc.storageMode = MTLStorageModePrivate;
#endif
        
        m_ColorTexture =  [m_Device newTextureWithDescriptor:fboColorTexDesc];
        
        releaseObj(fboColorTexDesc) ;


        MTLRenderPassColorAttachmentDescriptor *colorAttachment0 =  m_FrameBufferDescriptor.colorAttachments[0];
        colorAttachment0.texture = m_ColorTexture;
        
        
        if( samples)
        {
            assert(0);
        }
        
    }
    
    if( depth_mode)
    {
        AppendDepthBuffer(depth_mode) ;
    }
    
}

FBOMetal::~FBOMetal()
{
    releaseObj(m_ColorTexture) ;
    
}

id <MTLTexture> FBOMetal::GetTexture()
{
    return m_ColorTexture ;
}



