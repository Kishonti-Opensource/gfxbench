/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_planarmap.h"
#include "mtl_factories.h"

#include "mtl_types.h"
#import "graphics/metalgraphicscontext.h"

#include "mtl_globals.h"

using namespace MetalRender;

PlanarMap::PlanarMap(int w, int h, const char *name) :
KCL::PlanarMap(w, h, name),
m_Device(MetalRender::GetContext()->getDevice())
{
    
    m_framebuffer = [[MTLRenderPassDescriptor alloc] init];
    
    //
    //  Depth Texture
    //
    MTLTextureDescriptor *depthDesc =
    [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                       width:m_width
                                                      height:m_height
                                                   mipmapped:NO];
    
    depthDesc.usage       = MTLTextureUsageRenderTarget;
#if !TARGET_OS_IPHONE
    depthDesc.storageMode = MTLStorageModePrivate;
#endif
    
    
    m_depthTexture = [m_Device newTextureWithDescriptor:depthDesc];
    
    releaseObj(depthDesc);
    
    MTLRenderPassDepthAttachmentDescriptor *depthBufferAttachment = m_framebuffer.depthAttachment;
    depthBufferAttachment.texture = m_depthTexture;
    depthBufferAttachment.loadAction = MTLLoadActionClear;
    depthBufferAttachment.clearDepth = 1.0;
    depthBufferAttachment.storeAction = MTLStoreActionDontCare;
    
    
       
    //
    //  Color Texture
    //
    
    //  Use different texture format
    MTLTextureDescriptor *colorDesc =
    [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:PLANAR_FRAME_BUFFER_FORMAT
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
    drawBufferAttachment.clearColor = MTLClearColorMake(0.0,0.0,0.0,1.0);
    drawBufferAttachment.storeAction = MTLStoreActionStore;
    
    
    
    //
    //  Sampler
    //
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
    m_sampler = [m_Device newSamplerStateWithDescriptor:samplerDesc];
    
    releaseObj(samplerDesc);

}

PlanarMap::~PlanarMap()
{
}


KCL::PlanarMap* KCL::PlanarMap::Create( int w, int h, const char *name)
{
    return new MetalRender::PlanarMap( w, h, name);
}

