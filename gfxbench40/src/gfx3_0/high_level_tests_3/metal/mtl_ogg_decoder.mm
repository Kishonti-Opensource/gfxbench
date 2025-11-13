/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_ogg_decoder.h"
#include "platform.h"
#include "mtl_globals.h"

#include "mtl_types.h"
#import "graphics/metalgraphicscontext.h"

MTL_ogg_decoder::MTL_ogg_decoder(const char *filename) :
_ogg_decoder( filename), m_filename(filename),
m_Device(MetalRender::GetContext()->getDevice())
{
    MTLTextureDescriptor *texDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                     width:width
                                                                                    height:height
                                                                                 mipmapped:YES];

	texDesc.usage = MTLTextureUsageShaderRead;
#if !TARGET_OS_IPHONE
	texDesc.storageMode = MTLStorageModeManaged;
#endif

	m_mtlTexture = [m_Device newTextureWithDescriptor:texDesc];

    m_level_count = KCL::texture_levels(width, height) ;
    m_layers.resize(m_level_count) ;
    
    unsigned int _width = width;
    unsigned int _height = height;
    
    for (int i = 0; i < m_level_count; i++)
    {
        unsigned char* layer_ptr = new unsigned char[_width*_height*4] ;
        m_layers[i] = layer_ptr ;
        
        NSUInteger localHeight = _height;
        NSUInteger localWidth = _width;
        NSUInteger rowBytes = _width * 4;
        NSUInteger imageBytes = rowBytes * _height;
        
        memset( m_layers[i], 0, imageBytes);
        
        
        [m_mtlTexture replaceRegion:MTLRegionMake3D(0, 0, 0, localWidth, localHeight, 1) mipmapLevel:i slice:0 withBytes:m_layers[i] bytesPerRow:rowBytes bytesPerImage:imageBytes];
        
        _width /= 2;
        _height /= 2;
    }
    
    releaseObj(texDesc);

    MTLSamplerDescriptor *samplerDesc = [[MTLSamplerDescriptor alloc] init];
    samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
    samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;

    //samplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;
    samplerDesc.mipFilter = MTLSamplerMipFilterLinear;
    

    samplerDesc.maxAnisotropy = 1;
    samplerDesc.sAddressMode = MTLSamplerAddressModeRepeat;
    samplerDesc.tAddressMode = MTLSamplerAddressModeRepeat;
    samplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
    samplerDesc.normalizedCoordinates = YES;
    samplerDesc.lodMinClamp = 0;
    samplerDesc.lodMaxClamp = FLT_MAX;

    m_mtlSampler = [m_Device newSamplerStateWithDescriptor:samplerDesc];

    releaseObj(samplerDesc);
}

MTL_ogg_decoder::~MTL_ogg_decoder()
{
    for (int i = 0; i < m_level_count; i++)
    {
        delete[] m_layers[i];
        m_layers[i] = nullptr;
    }

    releaseObj(m_mtlTexture);
    releaseObj(m_mtlSampler);
}


inline bool isPowerOfTwo(int i)
{
    return (i > 0) && !(i & (i-1)) ;
}




#define BPP 4
void GenerateMipMapLayer(KCL::uint32 width, KCL::uint32 height, unsigned char* src, unsigned char* dst)
{
    unsigned char* src_row_ptr = src ;
    unsigned char* dst_row_ptr = dst ;
    
    for (KCL::uint32 i = 0 ; i < height; i+=2)
    {
        for (KCL::uint32 j = 0; j < width; j+=2)
        {
            // the alpha channel not used, so it is skipped
            for(KCL::uint32 k = 0; k < 3; k++)
            {
                unsigned int a = src_row_ptr[k+BPP*j] ;
                unsigned int b = src_row_ptr[k+BPP*j+BPP] ;
                unsigned int c = src_row_ptr[k+BPP*(j+width)] ;
                unsigned int d = src_row_ptr[k+BPP*(j+width)+BPP] ;
            
                dst_row_ptr[k+2*j] = (a+b+c+d)/4 ; // k+BPP*j/2
            }
        }
        
        src_row_ptr += 2*BPP*width; // 2 row 4 byte per pixel full resolution
        dst_row_ptr += 2*width; // 1 row BPP byte per pixel half resolution (BPP*width/2)
    }
}


void MTL_ogg_decoder::GenerateMipMapLayers()
{
    unsigned int _width = width;
    unsigned int _height = height;
    
    for (int i = 1; i < m_level_count; i++)
    {
        assert(isPowerOfTwo(_width)) ;
        assert(isPowerOfTwo(_height)) ;
        
        GenerateMipMapLayer(_width, _height, m_layers[i-1], m_layers[i]);
        
        _width /= 2;
        _height /= 2;
    }
}


bool MTL_ogg_decoder::DecodeDirect(float time)
{
    if(m_need_refresh > 0)
	{
        Decode(m_layers[0]);
        
        assert(isPowerOfTwo(height)) ;
        assert(isPowerOfTwo(width)) ;
        
        GenerateMipMapLayers();
        
        unsigned int _width = width;
        unsigned int _height = height;
        
        for (int i = 0; i < m_level_count; i++)
        {
            NSUInteger localHeight = _height;
            NSUInteger localWidth = _width;
            NSUInteger rowBytes = _width * 4;
            NSUInteger imageBytes = rowBytes * _height;
            
            [m_mtlTexture replaceRegion:MTLRegionMake3D(0, 0, 0,localWidth, localHeight, 1) mipmapLevel:i slice:0 withBytes:m_layers[i] bytesPerRow:rowBytes bytesPerImage:imageBytes];
            
            _width /= 2;
            _height /= 2;
        }
    }
	return true;
}
