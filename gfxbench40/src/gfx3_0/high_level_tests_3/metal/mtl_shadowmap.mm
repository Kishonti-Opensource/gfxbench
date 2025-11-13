/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_shadowmap.h"
#include "mtl_globals.h"

#include "mtl_types.h"
#import "graphics/metalgraphicscontext.h"

using namespace MetalRender;

ShadowMap::ShadowMap(unsigned int width, unsigned int height, const std::string &method_str) :
KRL_ShadowMap(),
m_width(width),
m_height(height),
m_method(ShadowMap::METHOD_UNKNOWN),
m_depthTexture(nil),
m_colorTexture(nil),
m_Device(MetalRender::GetContext()->getDevice())
{
    bool createColorTexture = false ;
    
    MTLStoreAction depthTextureStore = MTLStoreActionStore ;
    if (method_str == "depth map(depth)")
    {
        m_method = ShadowMap::METHOD_DEPTH_MAP_DEPTH ;
        createColorTexture = false ;
        depthTextureStore = MTLStoreActionStore ;
    }
    else if (method_str == "depth map(color)")
    {
        m_method = ShadowMap::METHOD_DEPTH_MAP_COLOR ;
        createColorTexture = true ;
        depthTextureStore = MTLStoreActionDontCare ;
    }
    else
    {
        // Unknown ShadowMap method ;
        assert(0) ;
    }

    m_framebuffer = [[MTLRenderPassDescriptor alloc] init];


    // Set up depth buffer used in both finalFramebuffer and gBuffer
    ////////////////////////////////////////////////////////////////
    MTLTextureDescriptor *depthDesc =
    [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                      width:m_width
                                                     height:m_height
                                                  mipmapped:NO];
    
    depthDesc.usage = MTLTextureUsageRenderTarget;
    if (!createColorTexture)
    {
        depthDesc.usage |= MTLTextureUsageShaderRead;
    }
#if !TARGET_OS_IPHONE
    depthDesc.storageMode = MTLStorageModePrivate;
#endif


    m_depthTexture = [m_Device newTextureWithDescriptor:depthDesc];

    releaseObj(depthDesc);

    MTLRenderPassDepthAttachmentDescriptor *depthBufferAttachment = m_framebuffer.depthAttachment;
    depthBufferAttachment.texture = m_depthTexture;
    depthBufferAttachment.loadAction = MTLLoadActionClear;
    depthBufferAttachment.clearDepth = 1.0;
	depthBufferAttachment.storeAction = depthTextureStore;
    
    
    if (createColorTexture)
    {
        MTLTextureDescriptor *colorDesc =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                          width:m_width
                                                         height:m_height
                                                      mipmapped:NO];
        
        colorDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_IPHONE
		colorDesc.storageMode = MTLStorageModePrivate;
#endif

        m_colorTexture = [m_Device newTextureWithDescriptor:colorDesc];

        releaseObj(colorDesc);

        MTLRenderPassColorAttachmentDescriptor *drawBufferAttachment = m_framebuffer.colorAttachments[0];
        drawBufferAttachment.texture = m_colorTexture;

        drawBufferAttachment.loadAction = MTLLoadActionClear;
        drawBufferAttachment.clearColor = MTLClearColorMake(1.0,1.0,1.0,1.0);
        drawBufferAttachment.storeAction = MTLStoreActionStore;
 
        
        
        MTLSamplerDescriptor *colorSamplerDesc = [[MTLSamplerDescriptor alloc] init];
        colorSamplerDesc.minFilter = MTLSamplerMinMagFilterNearest;
        colorSamplerDesc.magFilter = MTLSamplerMinMagFilterNearest;
        colorSamplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;
        

        colorSamplerDesc.maxAnisotropy = 1;
        colorSamplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
        colorSamplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
        colorSamplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
        colorSamplerDesc.normalizedCoordinates = YES;
        colorSamplerDesc.lodMinClamp = 0;
        colorSamplerDesc.lodMaxClamp = FLT_MAX;
        m_colorSampler = [m_Device newSamplerStateWithDescriptor:colorSamplerDesc];
        
        releaseObj(colorSamplerDesc);
    }
    
}

ShadowMap::~ShadowMap()
{
    releaseObj(m_framebuffer);
    releaseObj(m_depthTexture);
    if (m_colorTexture != nil) releaseObj(m_colorTexture);
    if (m_colorSampler != nil) releaseObj(m_colorSampler);
}
