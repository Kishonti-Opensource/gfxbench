/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_texture.h"
#include "mtl_globals.h"
#include <cassert>
#include <sstream>

#include "mtl_types.h"
#import "graphics/metalgraphicscontext.h"
#include <TargetConditionals.h>

#include "ng/log.h"

// Possible error codes for commit()
#define MTL_TEXERROR_NO_ERROR				 0
#define MTL_TEXERROR_ALREADY_COMMITTED		-1
#define MTL_TEXERROR_IMAGE_NOT_SET			-2
#define MTL_TEXERROR_INVALID_TYPE			-3
#define MTL_TEXERROR_NOT_IMPLEMENTED		-4
#define MTL_TEXERROR_INVALID_SLICE_COUNT	-5
#define MTL_TEXERROR_INVALID_FORMAT			-6

#define ENABLE_MIPMAPS

KCL::Texture* MetalRender::TextureFactory::CreateTexture(const KCL::Image* img, bool releaseUponCommit)
{
    return new MetalRender::Texture(img, releaseUponCommit);
}

bool commitWithNewMethod(KCL::ImageFormat format)
{
    switch(format)
    {
        // ETC2
        case KCL::Image_ETC2_RGBA8888:
            return true;
            
        //DXT
        case KCL::Image_DXT1:
        case KCL::Image_DXT1_RGBA:
        case KCL::Image_DXT3:
        case KCL::Image_DXT5:
            return true;
            
        // ASTC
        case KCL::Image_RGBA_ASTC_4x4:
        case KCL::Image_RGBA_ASTC_5x5:
        case KCL::Image_RGBA_ASTC_8x8:
            return true;
            
        default:
            // by default use the old method
            return false;
    }
    return false;
}

static bool MipmapRequired(KCL::TextureFilter filter)
{
	switch (filter)
	{
        case KCL::TextureFilter_Linear:
        case KCL::TextureFilter_Nearest:
            return true;
        default:
            break;
	}
    return false;
}

static MTLPixelFormat GetMTLTextureFormat(KCL::ImageFormat imgFormat, bool isSRGB)
{
	switch (imgFormat)
	{
        case KCL::Image_ALPHA_A8:
        case KCL::Image_LUMINANCE_L8:
            return MTLPixelFormatR8Unorm;

        case KCL::Image_LUMINANCE_ALPHA_LA88:
            return MTLPixelFormatRG8Unorm;

        case KCL::Image_RGB888:
        case KCL::Image_RGBA8888:
			return isSRGB ? MTLPixelFormatRGBA8Unorm_sRGB : MTLPixelFormatRGBA8Unorm;

        case KCL::Image_RGB565:
            return MTLPixelFormatB5G6R5Unorm;

        case KCL::Image_RGBA4444:
            return MTLPixelFormatABGR4Unorm;

        case KCL::Image_RGBA5551:
            return MTLPixelFormatA1BGR5Unorm;

        case KCL::Image_RGBA_32F:
            return MTLPixelFormatRGBA32Float;

        case KCL::Image_DEPTH_16:
            return MTLPixelFormatDepth32Float;
#if !TARGET_OS_IPHONE
        case KCL::Image_DXT1:
        case KCL::Image_DXT1_RGBA:
			return isSRGB ? MTLPixelFormatBC1_RGBA_sRGB : MTLPixelFormatBC1_RGBA;

        case KCL::Image_DXT3:
            return isSRGB ? MTLPixelFormatBC2_RGBA_sRGB : MTLPixelFormatBC2_RGBA;

        case KCL::Image_DXT5:
            return isSRGB ? MTLPixelFormatBC3_RGBA_sRGB : MTLPixelFormatBC3_RGBA;
#endif
        case KCL::Image_PVRTC2:
            return isSRGB ? MTLPixelFormatPVRTC_RGB_2BPP_sRGB : MTLPixelFormatPVRTC_RGB_2BPP;
             
        case KCL::Image_PVRTC4:
			return isSRGB ? MTLPixelFormatPVRTC_RGBA_4BPP_sRGB : MTLPixelFormatPVRTC_RGBA_4BPP;

        case KCL::Image_ETC1:
            assert(!"MTL doesn't handle ETC1. Is ETC1 a subset of ETC2?");

        case KCL::Image_ETC2_RGB:
			return isSRGB ? MTLPixelFormatETC2_RGB8_sRGB : MTLPixelFormatETC2_RGB8;
            
        case KCL::Image_ETC2_RGBA8888:
			return isSRGB ? MTLPixelFormatEAC_RGBA8_sRGB : MTLPixelFormatEAC_RGBA8;
            
        case KCL::Image_RGBA_ASTC_4x4:
			return isSRGB ? MTLPixelFormatASTC_4x4_sRGB : MTLPixelFormatASTC_4x4_LDR;

        case KCL::Image_RGBA_ASTC_5x5:
			return isSRGB ? MTLPixelFormatASTC_5x5_sRGB : MTLPixelFormatASTC_5x5_LDR;

        case KCL::Image_RGBA_ASTC_8x8:
			return isSRGB ? MTLPixelFormatASTC_8x8_sRGB : MTLPixelFormatASTC_8x8_LDR;

        default:
            break;
	}
    assert(!"Image format unhandled by MTL");

    return MTLPixelFormatInvalid;
}

static unsigned int GetMTLCompressedRowBytes(MTLPixelFormat compressedFormat, unsigned int width)
{
    switch(compressedFormat)
    {
        case MTLPixelFormatPVRTC_RGB_2BPP:
        case MTLPixelFormatPVRTC_RGBA_4BPP:
            return 0;
        case MTLPixelFormatETC2_RGB8:
            width /= 4; // block count
            width = (width < 1) ? 1 : width; // at least one block
            return width * 8; // 8 bytes per block     

        default:
            assert(!"Unhandled format");
    }
}

static unsigned int GetMTLCompressedImageBytes(MTLPixelFormat compressedFormat, unsigned int width)
{
	switch(compressedFormat)
	{
		case MTLPixelFormatPVRTC_RGB_2BPP:
		case MTLPixelFormatPVRTC_RGBA_4BPP:
			return 0;
		case MTLPixelFormatETC2_RGB8:
		{
			unsigned int blockCount = width / 4;
			blockCount = (blockCount < 1) ? 1 : blockCount;
			return blockCount * blockCount * 8; // 8 bytes per block
		}

		default:
			assert(!"Unhandled format");
	}
}

static unsigned int GetMTLFormatPixelByteSize(MTLPixelFormat format)
{
    switch (format)
	{
        case MTLPixelFormatR8Unorm:
            return 1;
        case MTLPixelFormatA1BGR5Unorm:
        case MTLPixelFormatB5G6R5Unorm:
        case MTLPixelFormatRG8Unorm:
            return 2;
        case MTLPixelFormatRGBA8Unorm:
        case MTLPixelFormatDepth32Float:
            return 4;
        case MTLPixelFormatRGBA32Float:
            return 16;
        default:
            break;
    }
    return 0;
}

static bool GetMTLRenderableFormat(MTLPixelFormat format)
{
	switch (format)
	{
        case MTLPixelFormatR8Unorm:
        case MTLPixelFormatRG8Unorm:
        case MTLPixelFormatRGBA8Unorm:
        case MTLPixelFormatB5G6R5Unorm:
        case MTLPixelFormatA1BGR5Unorm:
        case MTLPixelFormatRGBA32Float:
            return true;
        default:
            break;
    }
    return false;
}

static MTLSamplerMipFilter GetMTLMipTextureFilter(KCL::TextureFilter mipFilter)
{
#ifdef ENABLE_MIPMAPS
	switch (mipFilter)
	{
        case KCL::TextureFilter_Linear:
            return MTLSamplerMipFilterLinear;

        case KCL::TextureFilter_Nearest:
            return MTLSamplerMipFilterNearest;

        default:
            break;
    }
#endif // ENABLE_MIPMAPS

    return MTLSamplerMipFilterNotMipmapped;
}

static MTLSamplerMinMagFilter GetMTLTextureFilter(KCL::TextureFilter filter)
{
	switch (filter)
	{
        case KCL::TextureFilter_Linear:
            return MTLSamplerMinMagFilterLinear;

        case KCL::TextureFilter_Nearest:
            return MTLSamplerMinMagFilterNearest;

        default:
            break;
    }

    assert(!"Unsupported MTL Texture Filter");
    return MTLSamplerMinMagFilterLinear;
}


static MTLSamplerAddressMode GetMTLTextureWrap(KCL::TextureWrap wrap)
{
	switch (wrap)
	{
        case KCL::TextureWrap_Clamp:
            return MTLSamplerAddressModeClampToEdge;
            
        case KCL::TextureWrap_Mirror:
            return MTLSamplerAddressModeMirrorRepeat;
            
        case KCL::TextureWrap_Repeat:
            return MTLSamplerAddressModeRepeat;

        default:
            break;
	}

    assert(!"Unsupported wrap mode");
    return MTLSamplerAddressModeClampToEdge;
}


MetalRender::Texture::Texture() :
    KCL::Texture(),
	m_texture(nil),
    m_Device(MetalRender::GetContext()->getDevice())
{
}

MetalRender::Texture::Texture(const KCL::Image* image, bool releaseUponCommit) :
    KCL::Texture(image, releaseUponCommit),
    m_texture(nil),
    m_Device(MetalRender::GetContext()->getDevice())
{
}

MetalRender::Texture::Texture(const KCL::Image* image, KCL::TextureType type, bool releaseUponCommit) :
    KCL::Texture(image, type, releaseUponCommit),
    m_texture(nil),
    m_sampler(nil),
    m_Device(MetalRender::GetContext()->getDevice())
{
}

MetalRender::Texture::~Texture(void)
{
    releaseObj(m_texture);
    releaseObj(m_sampler);
}

static KCL::uint8* CreateRGBA8BufferFromRGB8Data(KCL::uint32 width, KCL::uint32 height, const void* origData)
{
    KCL::uint8 * dstData = new KCL::uint8[width*height*4];
    const KCL::uint8 * srcData = (const KCL::uint8 *) origData;

    for(KCL::uint32 h = 0; h < height; h++)
    {
        for(KCL::uint32 w = 0; w < width; w++)
        {
            KCL::uint32 pixelNum = w+h*width;
            dstData[pixelNum*4+0] = srcData[pixelNum*3 + 0];
            dstData[pixelNum*4+1] = srcData[pixelNum*3 + 1];
            dstData[pixelNum*4+2] = srcData[pixelNum*3 + 2];
            dstData[pixelNum*4+3] = 0xFF;
        }
    }

    return dstData;
}

static void DestroyRGBA8Buffer(KCL::uint8* data)
{
    delete [] data;
}



long MetalRender::Texture::bind(KCL::uint32 slotId)
{
    assert(!"Call Set Instead of bind()\n");
}

void MetalRender::Texture::setupSampler()
{
    MTLSamplerDescriptor *samplerDesc = [[MTLSamplerDescriptor alloc] init];
    samplerDesc.minFilter = GetMTLTextureFilter(m_minFilter);
    samplerDesc.magFilter = GetMTLTextureFilter(m_magFilter);
    samplerDesc.mipFilter = GetMTLMipTextureFilter(m_mipFilter);
    

    samplerDesc.maxAnisotropy = 1;
    samplerDesc.sAddressMode = GetMTLTextureWrap(m_wrapS);
    samplerDesc.tAddressMode = GetMTLTextureWrap(m_wrapT);
    samplerDesc.rAddressMode = GetMTLTextureWrap(m_wrapU);;
    samplerDesc.normalizedCoordinates = YES;
    samplerDesc.lodMinClamp = 0;
    samplerDesc.lodMaxClamp = FLT_MAX;

    if(samplerDesc.mipFilter != MTLSamplerMipFilterNotMipmapped)
    {
        assert(m_texture.mipmapLevelCount > 1);
    }

    m_sampler = [m_Device newSamplerStateWithDescriptor:samplerDesc];

    releaseObj(samplerDesc);
}

void MetalRender::
Texture::GenerateMipmaps()
{
#ifdef ENABLE_MIPMAPS
    @autoreleasepool {
        
    assert(m_texture);
    assert(GetMTLRenderableFormat(m_texture.pixelFormat));

    id <MTLCommandQueue> commandQueue = MetalRender::GetContext()->getMainCommandQueue();
    id <MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
    id <MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];

    [blitEncoder generateMipmapsForTexture:m_texture];

    [blitEncoder endEncoding];
	blitEncoder = nil;

    [commandBuffer commit];
    [commandBuffer waitUntilCompleted] ;

	commandBuffer = nil;
	commandQueue = nil;
        
    }
#endif // ENABLE_MIPMAPS
}

long MetalRender::
Texture::commit2D()
{
    BOOL mipmapsRequired = MipmapRequired(m_mipFilter);
	
    // Create  texture
    MTLPixelFormat pixFormat = GetMTLTextureFormat(m_image->getFormat(), m_isSRGB);
    MTLTextureDescriptor *texDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixFormat
                                                                                     width:m_image->getWidth()
                                                                                    height:m_image->getHeight()
                                                                                 mipmapped:mipmapsRequired];

    m_texture = [m_Device newTextureWithDescriptor:texDesc];

    releaseObj(texDesc);

    setupSampler();
    
    
    if (commitWithNewMethod(m_image->getFormat()))
    {
        // upload the texture with the new method
        return commit2DNew();
    }
    

    uint32_t rowBytes = 0;
	
	uint32_t blockWidth = m_image->getWidth();
	uint32_t blockHeight = m_image->getHeight();

    if(GetMTLRenderableFormat(pixFormat))
    {
        rowBytes = m_image->getWidth() * GetMTLFormatPixelByteSize(pixFormat);
    }
    else
    {
        rowBytes = GetMTLCompressedRowBytes(pixFormat, m_image->getWidth());

		if (pixFormat == MTLPixelFormatETC2_RGB8)
		{
			blockWidth /= 4;
			blockHeight /= 4;
		}

    }
	
    uint32_t imageBytes = rowBytes * blockHeight;

    KCL::uint8* data = (KCL::uint8*) m_image->getData(0);

    if(KCL::Image_RGB888 == m_image->getFormat())
    {
        data = CreateRGBA8BufferFromRGB8Data(blockWidth, blockHeight, data);
    }
	

	[m_texture replaceRegion:MTLRegionMake3D(0, 0, 0,m_image->getWidth(), m_image->getHeight(), 1)
				 mipmapLevel:0
					   slice:0
				   withBytes:data
				 bytesPerRow:rowBytes
			   bytesPerImage:imageBytes];
	
    if(KCL::Image_RGB888 == m_image->getFormat())
    {
        DestroyRGBA8Buffer(data);
    }

    if(!mipmapsRequired)
    {
        return MTL_TEXERROR_NO_ERROR;
    }

    if(GetMTLRenderableFormat(pixFormat) && mipmapsRequired)
    {
        GenerateMipmaps();
    }
    else
    {
		if(pixFormat == MTLPixelFormatETC2_RGB8)
		{
			//we've already uploaded the base level
			commitETC2D(data+imageBytes, m_image->getMipCount());
		}
		else
		{
			for (KCL::uint32 i = 1; i <= m_image->getMipCount(); ++i)
			{
				
				KCL::uint32 size =  1<<(m_image->getMipCount() - i);
				KCL::uint32 mipmapsize, w, h;
				KCL::uint8 *data = 0;
				m_image->getMipmapData (i, &data, &mipmapsize, &w, &h);
				
				assert((w == h) && (w == size));
				
				NSUInteger rowBytes = GetMTLCompressedRowBytes(pixFormat, w);
				

				[m_texture replaceRegion:MTLRegionMake3D(0, 0, 0, w, h, 1)
						 mipmapLevel:i
							   slice:0
						   withBytes:data
						 bytesPerRow:rowBytes
					   bytesPerImage:0];
			}
		}
    }
    
    return MTL_TEXERROR_NO_ERROR;
}

void MetalRender::Texture::commitETC2D(void* data, uint32_t mipmapCount)
{
	uint8_t* bytes = reinterpret_cast<uint8_t*>(data);
	for(int i = 1; i <= mipmapCount; i++)
	{
		uint32_t width = 1 << (mipmapCount - i); // height == width
		uint32_t blockCount = MAX(width / 4, 1);
		uint32_t rowBytes = blockCount * 8;
		uint32_t imageBytes = rowBytes * blockCount;
		

		[m_texture replaceRegion:MTLRegionMake3D(0, 0, 0, width, width, 1)
						 mipmapLevel:i
							   slice:0
						   withBytes:bytes
						 bytesPerRow:rowBytes
					   bytesPerImage:imageBytes];
		
		bytes += imageBytes;
	}
}


long MetalRender::Texture::commit2DNew()
{
    bool mipmaps_required = MipmapRequired(m_mipFilter);
    int width = m_image->getWidth();
    int height = m_image->getHeight();
    int lods = mipmaps_required ? KCL::texture_levels(width, height) : 1;
    
    if (mipmaps_required)
    {
        assert(lods == (m_image->getMipCount() + 1));
    }
    
    // Dimensions of a block in texels
    KCL::uint32 block_dim_x, block_dim_y, block_dim_z;
    m_image->getBlockDimensions(block_dim_x, block_dim_y, block_dim_z);
    
    // Size of a block in bits
    KCL::uint32 block_size = m_image->getBlockSize();
    
    KCL::uint32 offset = 0;
    for (KCL::uint32 i = 0; i < lods; i++)
    {
        // Calculate the dimensions of the mipmap
        KCL::uint32 mipmap_width = width / (1 << i);
        KCL::uint32 mipmap_height = height / (1 << i);
        mipmap_width = mipmap_width ? mipmap_width : 1;
        mipmap_height = mipmap_height ? mipmap_height : 1;
        
        // Calculate the number of blocks for the mipmap
        KCL::uint32 block_count_x = (mipmap_width + block_dim_x - 1) / block_dim_x;
        KCL::uint32 block_count_y = (mipmap_height + block_dim_y - 1) / block_dim_y;
        
        // Calculate the compressed data size
        KCL::uint32 bytes_per_row = block_count_x * block_size / 8;
        KCL::uint32 bytes_per_image = block_count_y * bytes_per_row;
        
        // Upload the mipmap level
        [m_texture replaceRegion:MTLRegionMake3D(0, 0, 0, mipmap_width, mipmap_height, 1)
                     mipmapLevel:i
                           slice:0
                       withBytes:(KCL::uint8*)m_image->getData(0) + offset
                     bytesPerRow:bytes_per_row
                   bytesPerImage:bytes_per_image];
        
        offset += bytes_per_image;
    }
    
    return MTL_TEXERROR_NO_ERROR;
}


long MetalRender::Texture::commit3D()
{
    BOOL mipmapsRequired = MipmapRequired(m_mipFilter);
    // Create  texture
    MTLPixelFormat pixFormat = GetMTLTextureFormat(m_image->getFormat(), m_isSRGB);
    MTLTextureDescriptor *texDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixFormat
                                                                                     width:m_image->getWidth()
                                                                                    height:m_image->getHeight()
                                                                                 mipmapped:mipmapsRequired];
    texDesc.textureType = MTLTextureType3D;
    texDesc.depth = m_image->getDepth();

    m_texture = [m_Device newTextureWithDescriptor:texDesc];

    releaseObj(texDesc);

    setupSampler();

    assert(GetMTLRenderableFormat(pixFormat));

    for (KCL::uint32 i = 0; i < m_image->getDepth(); i++)
    {
        KCL::uint8 * data = (KCL::uint8 *)m_image->getData(0) + m_image->getSlicePitch() * i;

        if(KCL::Image_RGB888 == m_image->getFormat())
        {
            data = CreateRGBA8BufferFromRGB8Data(m_image->getWidth() , m_image->getHeight(), data);
        }

        unsigned int rowBytes = m_image->getWidth() * GetMTLFormatPixelByteSize(pixFormat);
        unsigned int imageBytes = rowBytes * m_image->getHeight();


		[m_texture replaceRegion:MTLRegionMake3D(0, 0, i, m_image->getWidth(), m_image->getHeight(), 1)
					 mipmapLevel:0
						   slice:0
					   withBytes:data
					 bytesPerRow:rowBytes
				   bytesPerImage:imageBytes];
        
        if(KCL::Image_RGB888 == m_image->getFormat())
        {
            DestroyRGBA8Buffer(data);
        }

    }

    if(mipmapsRequired)
    {
        GenerateMipmaps();
    }

    return MTL_TEXERROR_NO_ERROR;
}

long MetalRender::Texture::commitArray()
{
    BOOL mipmapsRequired = MipmapRequired(m_mipFilter);
    // Create  texture
    MTLPixelFormat pixFormat = GetMTLTextureFormat(m_image->getFormat(), m_isSRGB);
    MTLTextureDescriptor *texDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixFormat
                                                                                     width:m_image->getWidth()
                                                                                    height:m_image->getHeight()
                                                                                 mipmapped:mipmapsRequired];
    texDesc.textureType = MTLTextureType2DArray;
    texDesc.arrayLength = m_image->getDepth();

    m_texture = [m_Device newTextureWithDescriptor:texDesc];

    releaseObj(texDesc);

    setupSampler();
    
    
    if (commitWithNewMethod(m_image->getFormat()))
    {
        // upload the texture with the new method
        return commitArrayNew();
    }
    

    if(!mipmapsRequired)
    {
        return MTL_TEXERROR_NO_ERROR;
    }

    if(GetMTLRenderableFormat(pixFormat))
    {
        for (KCL::uint32 i = 0; i < m_image->getDepth(); i++)
        {
            KCL::uint8 * data = (KCL::uint8 *)m_image->getData(0) + m_image->getSlicePitch() * i;

            KCL::uint32 bytesPerImage = m_image->getSlicePitch() ;
            if(KCL::Image_RGB888 == m_image->getFormat())
            {
                data = CreateRGBA8BufferFromRGB8Data(m_image->getWidth() , m_image->getHeight(), data);
                assert(bytesPerImage % 3 == 0) ;
                bytesPerImage = 4*bytesPerImage/3 ;
            }


			[m_texture replaceRegion:MTLRegionMake3D(0, 0, 0, m_image->getWidth(), m_image->getHeight(), 1)
						 mipmapLevel:0
							   slice:i
						   withBytes:data
						 bytesPerRow:m_image->getWidth() * GetMTLFormatPixelByteSize(pixFormat)
					     bytesPerImage:bytesPerImage];
            
            if(KCL::Image_RGB888 == m_image->getFormat())
            {
                DestroyRGBA8Buffer(data);
            }
        }

        if (mipmapsRequired)
        {
            GenerateMipmaps();
        }
    }
    else
    {

        assert(m_image->getWidth() == m_image->getHeight());

        int offset = 0;

        for( KCL::uint32 j=0; j<=m_image->getMipCount(); j++)
        {
            KCL::uint32 dim = 1 << (m_image->getMipCount() - j);

            uint32_t rowBytes   = GetMTLCompressedRowBytes(pixFormat, dim);
            uint32_t imageBytes = GetMTLCompressedImageBytes(pixFormat, dim);

            for (KCL::uint32 i = 0; i < m_image->getDepth(); i++)
            {
                void* slicePtr = (char*)m_image->getData(i) + offset;
				

				[m_texture replaceRegion:MTLRegionMake3D(0, 0, 0, dim, dim, 1)
							 mipmapLevel:j
								   slice:i
							   withBytes:slicePtr
							 bytesPerRow:rowBytes
						   bytesPerImage:imageBytes];
            }
            
            offset += imageBytes;
        }
    }


    return MTL_TEXERROR_NO_ERROR;
}


long MetalRender::Texture::commitArrayNew()
{
    bool mipmaps_required = MipmapRequired(m_mipFilter);
    int width = m_image->getWidth();
    int height = m_image->getHeight();
    int lods = mipmaps_required ? KCL::texture_levels(width, height) : 1;
    
    if (mipmaps_required)
    {
        assert(lods == (m_image->getMipCount() + 1));
    }
    
    // Dimensions of a block in texels
    KCL::uint32 block_dim_x, block_dim_y, block_dim_z;
    m_image->getBlockDimensions(block_dim_x, block_dim_y, block_dim_z);
    
    // Size of a block in bits
    KCL::uint32 block_size = m_image->getBlockSize();
    
    // Upload the mipmaps
    KCL::uint32 mipmap_level_offset = 0;
    for (KCL::uint32 i = 0; i < lods; i++)
    {
        // Calculate the dimensions of the mipmap
        KCL::uint32 mipmap_width = width / (1 << i);
        KCL::uint32 mipmap_height = height / (1 << i);
        mipmap_width = mipmap_width ? mipmap_width : 1;
        mipmap_height = mipmap_height ? mipmap_height : 1;
        
        // Calculate the number of blocks for the mipmap
        KCL::uint32 block_count_x = (mipmap_width + block_dim_x - 1) / block_dim_x;
        KCL::uint32 block_count_y = (mipmap_height + block_dim_y - 1) / block_dim_y;
        
        // Calculate the compressed data size
        KCL::uint32 bytes_per_row = block_count_x * block_size / 8;
        KCL::uint32 bytes_per_image = block_count_y * bytes_per_row;
        
        // Upload all surfaces for this lod level
        for (KCL::uint32 depth = 0; depth < m_image->getDepth(); depth++)
        {
            [m_texture replaceRegion:MTLRegionMake3D(0, 0, 0, mipmap_width, mipmap_height, 1)
                         mipmapLevel:i
                               slice:depth
                           withBytes:(KCL::uint8*)m_image->getData(0) + mipmap_level_offset
                         bytesPerRow:bytes_per_row
                       bytesPerImage:bytes_per_image];
        }
        mipmap_level_offset += bytes_per_image;		
    }
    
    return MTL_TEXERROR_NO_ERROR;
}



long MetalRender::Texture::commitCube()
{
    bool mip_mapped = MipmapRequired(m_mipFilter);
    assert(m_image->getWidth() == m_image->getHeight());

    // Create  texture
    MTLPixelFormat pixFormat = GetMTLTextureFormat(m_image->getFormat(), m_isSRGB);
    MTLTextureDescriptor *texDesc = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:pixFormat
                                                                                        size:m_image->getWidth()
                                                                                 mipmapped:mip_mapped];
    m_texture = [m_Device newTextureWithDescriptor:texDesc];

    releaseObj(texDesc);
    
    setupSampler();

    bool compressed = !GetMTLRenderableFormat(pixFormat);
    
    if (commitWithNewMethod(m_image->getFormat()) || (compressed && mip_mapped))
    {
        // upload the texture with the new method
        return commitCubeNew();
    }
    else // !(compressed && mip_mapped)
    {
        for(int face = 0; face < 6; face++)
        {
            unsigned int rowBytes;

            if(GetMTLRenderableFormat(pixFormat))
            {
                rowBytes = m_image->getWidth() * GetMTLFormatPixelByteSize(pixFormat);
            }
            else
            {
                rowBytes = GetMTLCompressedRowBytes(pixFormat, m_image->getWidth() );
            }

            KCL::uint32 imageBytes = rowBytes * m_image->getHeight();
            

            [m_texture replaceRegion:MTLRegionMake3D(0, 0, 0, m_image->getWidth(), m_image->getHeight(), 1)
                             mipmapLevel:0
                                   slice:face
                               withBytes:m_image->getData(face)
                             bytesPerRow:rowBytes
                           bytesPerImage:imageBytes];
        }
        
        if (mip_mapped && !compressed)
        {
            GenerateMipmaps();
        }
    }

    return MTL_TEXERROR_NO_ERROR;
}


long MetalRender::Texture::commitCubeNew()
{
    bool mip_mapped = MipmapRequired(m_mipFilter);
    int width = m_image->getWidth();
    int height = m_image->getHeight();
    int lods = mip_mapped ? KCL::texture_levels(width, height) : 1;
    
    // Check if the compressed texture contains enough lod levels
    if (mip_mapped)
    {
        assert(lods == (m_image->getMipCount() + 1));
    }
    
    // Dimensions of a block in texels
    KCL::uint32 block_dim_x, block_dim_y, block_dim_z;
    m_image->getBlockDimensions(block_dim_x, block_dim_y, block_dim_z);
    
    // Size of a block in bits
    KCL::uint32 block_size = m_image->getBlockSize();
    
    // Upload the mipmaps
    KCL::uint32 mipmap_level_offset = 0;
    for (KCL::uint32 i = 0; i < lods; i++)
    {
        // Calculate the dimensions of the mipmap
        KCL::uint32 mipmap_width = width / (1 << i);
        KCL::uint32 mipmap_height = height / (1 << i);
        mipmap_width = mipmap_width ? mipmap_width : 1;
        mipmap_height = mipmap_height ? mipmap_height : 1;
        
        // Calculate the number of blocks for the mipmap
        KCL::uint32 block_count_x = (mipmap_width + block_dim_x - 1) / block_dim_x;
        KCL::uint32 block_count_y = (mipmap_height + block_dim_y - 1) / block_dim_y;
        
        // Calculate the compressed data size
        KCL::uint32 bytes_per_row = block_count_x * block_size / 8;
        KCL::uint32 bytes_per_image = block_count_y * bytes_per_row;
        
        // Upload the faces	for this lod level
        for (KCL::uint32 face = 0; face < 6; face++)
        {
            [m_texture replaceRegion:MTLRegionMake3D(0, 0, 0, mipmap_width, mipmap_height, 1)
                         mipmapLevel:i
                               slice:face
                           withBytes:(KCL::uint8*)m_image->getData(face) + mipmap_level_offset
                         bytesPerRow:bytes_per_row
                       bytesPerImage:bytes_per_image];
        }
        mipmap_level_offset += bytes_per_image;
    }
    
    return MTL_TEXERROR_NO_ERROR;
}


long MetalRender::Texture::commitCubeArray()
{
#if TARGET_OS_EMBEDDED
	return MTL_TEXERROR_INVALID_TYPE;
#else
	bool mip_mapped = MipmapRequired(m_mipFilter);
	assert(m_image->getWidth() == m_image->getHeight());

	// Create  texture
	MTLPixelFormat pixFormat = GetMTLTextureFormat(m_image->getFormat(), m_isSRGB);
	MTLTextureDescriptor *texDesc = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:pixFormat
																						  size:m_image->getWidth()
																					 mipmapped:mip_mapped];

    texDesc.textureType = MTLTextureTypeCubeArray;
    texDesc.arrayLength = m_image->getDepth();
	m_texture = [m_Device newTextureWithDescriptor:texDesc];

	releaseObj(texDesc);

	setupSampler();

	bool compressed = !GetMTLRenderableFormat(pixFormat);

	if (commitWithNewMethod(m_image->getFormat()) || (compressed && mip_mapped))
	{
		// upload the texture with the new method
		return commitCubeArrayNew();
	}
	else // !(compressed && mip_mapped)
	{
		for(int slice = 0; slice < m_image->getDepth(); slice++)
		{
			unsigned int rowBytes;

			if(GetMTLRenderableFormat(pixFormat))
			{
				rowBytes = m_image->getWidth() * GetMTLFormatPixelByteSize(pixFormat);
			}
			else
			{
				rowBytes = GetMTLCompressedRowBytes(pixFormat, m_image->getWidth() );
			}

			KCL::uint32 imageBytes = rowBytes * m_image->getHeight();

			[m_texture replaceRegion:MTLRegionMake3D(0, 0, 0, m_image->getWidth(), m_image->getHeight(), 1)
						 mipmapLevel:0
							   slice:slice
						   withBytes:m_image->getData(slice)
						 bytesPerRow:rowBytes
					   bytesPerImage:imageBytes];
		}

		if (mip_mapped && !compressed)
		{
			GenerateMipmaps();
		}
	}

	return MTL_TEXERROR_NO_ERROR;
#endif
}


long MetalRender::Texture::commitCubeArrayNew()
{
#if TARGET_OS_EMBEDDED
	return MTL_TEXERROR_INVALID_TYPE;
#else
	bool mip_mapped = MipmapRequired(m_mipFilter);
	int width = m_image->getWidth();
	int height = m_image->getHeight();
	int lods = mip_mapped ? KCL::texture_levels(width, height) : 1;

	// Check if the compressed texture contains enough lod levels
	if (mip_mapped)
	{
		assert(lods == (m_image->getMipCount() + 1));
	}

	// Dimensions of a block in texels
	KCL::uint32 block_dim_x, block_dim_y, block_dim_z;
	m_image->getBlockDimensions(block_dim_x, block_dim_y, block_dim_z);

	// Size of a block in bits
	KCL::uint32 block_size = m_image->getBlockSize();

	// Upload the mipmaps
	KCL::uint32 mipmap_level_offset = 0;
	for (KCL::uint32 i = 0; i < lods; i++)
	{
		// Calculate the dimensions of the mipmap
		KCL::uint32 mipmap_width = width / (1 << i);
		KCL::uint32 mipmap_height = height / (1 << i);
		mipmap_width = mipmap_width ? mipmap_width : 1;
		mipmap_height = mipmap_height ? mipmap_height : 1;

		// Calculate the number of blocks for the mipmap
		KCL::uint32 block_count_x = (mipmap_width + block_dim_x - 1) / block_dim_x;
		KCL::uint32 block_count_y = (mipmap_height + block_dim_y - 1) / block_dim_y;

		// Calculate the compressed data size
		KCL::uint32 bytes_per_row = block_count_x * block_size / 8;
		KCL::uint32 bytes_per_image = block_count_y * bytes_per_row;

		// Upload the faces	for this lod level
		for (KCL::uint32 slice = 0; slice < m_image->getDepth(); slice++)
		{
			[m_texture replaceRegion:MTLRegionMake3D(0, 0, 0, mipmap_width, mipmap_height, 1)
						 mipmapLevel:i
							   slice:slice
						   withBytes:(KCL::uint8*)m_image->getData(slice) + mipmap_level_offset
						 bytesPerRow:bytes_per_row
					   bytesPerImage:bytes_per_image];
		}
		mipmap_level_offset += bytes_per_image;
	}

	return MTL_TEXERROR_NO_ERROR;
#endif
}


long MetalRender::Texture::commit()
{
    if (m_isCommitted)
	{
		// Already committed
		return MTL_TEXERROR_ALREADY_COMMITTED;
	}

    assert(m_texture == nil);

    long result = MTL_TEXERROR_NO_ERROR;
	if (m_video)
	{
        assert(!"video texture no implmented");

        return MTL_TEXERROR_NOT_IMPLEMENTED;
	}
	else if (m_image)
	{
	    //bool supported = isFormatSupported(fmt);
	    //if (!supported)
        if((KCL::Texture::decodeTarget != KCL::ImageTypeAny) && (m_image->getFormat() == KCL::Texture::decodeSource))
	    {
            const KCL::Image* img = m_image->cloneTo(KCL::Texture::decodeTarget);  
            if(m_releaseUponCommit)
            {
                delete m_image;
            }
            m_image = img;
	    }

        switch (m_type)
		{
		case KCL::Texture_1D:
                assert(!"1D Texture not supported");
                break;
		case KCL::Texture_2D:
			result = commit2D();
            break;
		case KCL::Texture_3D:
			result = commit3D();
            break;
		case KCL::Texture_Array:
			result = commitArray();
            break;
		case KCL::Texture_Cube:
			result = commitCube();
            break;
#if !TARGET_OS_EMBEDDED
		case KCL::Texture_CubeArray:
			result = commitCubeArray();
			break;
#endif
		default:
			// Not supported texture type.
			result = MTL_TEXERROR_INVALID_TYPE;
            break;
		}
        
        if(result == MTL_TEXERROR_NO_ERROR)
        {
            m_isCommitted = true;
        }

        if (m_releaseUponCommit)
        {
            delete m_image;
            m_image = NULL;
        }

        return result;
	}
	return MTL_TEXERROR_IMAGE_NOT_SET;
}

long MetalRender::Texture::release()
{
	return MTL_TEXERROR_NO_ERROR;
}


long MetalRender::Texture::setVideoTime(float time)
{
    assert(!"MetalRender::Texture::setVideoTime");
	// MTL_TODO To be implemented.
	return MTL_TEXERROR_NOT_IMPLEMENTED;
}

