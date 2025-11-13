/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl_metal.h"


Metal_vertex_buffer::Metal_vertex_buffer()
: buffer(nil)
{
}


Metal_vertex_buffer::~Metal_vertex_buffer()
{
	buffer = nil;
}



Metal_index_buffer::Metal_index_buffer()
: buffer(nil)
{
}


Metal_index_buffer::~Metal_index_buffer()
{
	buffer = nil;
}



Metal_texture::Metal_texture()
: texture(nil)
, pixel_format(MTLPixelFormatInvalid)
, sampler(nil)
{
}


Metal_texture::~Metal_texture()
{
	texture = nil;
	sampler = nil;
}



static void RGB888toRGBA8888( uint32_t width, uint32_t height, uint8_t *src, uint8_t *dst)
{
	for( uint32_t i=0; i<width*height; i++)
	{
		dst[i*4+0] = src[i*3+0];
		dst[i*4+1] = src[i*3+1];
		dst[i*4+2] = src[i*3+2];
		dst[i*4+3] = 255;
	}
}


bool mtl_UploadTextureSlice(Metal_texture &texture, uint32_t slice, uint32_t level, uint32_t slice_width, uint32_t slice_height, uint32_t bytes_per_row, uint32_t bytes_per_image, void* data)
{
#if UPLOAD_TEXTURES_TO_PRIVATE_MEMORY
	id<MTLBuffer> temp_buffer = [Metal_instance::This->device newBufferWithBytes:data length:bytes_per_image options:MTLResourceStorageModeShared];
	
	id <MTLCommandBuffer> commandBuffer = [Metal_instance::This->commandQueue commandBuffer];
	id <MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
	
	MTLSize size = MTLSizeMake(slice_width, slice_height,1);
	
	[blitCommandEncoder copyFromBuffer:temp_buffer
						  sourceOffset:0
					 sourceBytesPerRow:bytes_per_row
				   sourceBytesPerImage:bytes_per_image
							sourceSize:size
							 toTexture:texture.texture
					  destinationSlice:slice
					  destinationLevel:level
					 destinationOrigin:MTLOriginMake(0, 0, 0)];
	
	[blitCommandEncoder endEncoding];
	[commandBuffer commit];
	
	[commandBuffer waitUntilScheduled];
	
	if ([commandBuffer error] != nil)
	{
		NSLog(@"%@\n",[commandBuffer error]); assert(0) ;
	}
	
	[commandBuffer waitUntilCompleted];
	
	if ([commandBuffer error] != nil)
	{
		NSLog(@"%@\n",[commandBuffer error]); assert(0) ;
	}
	
	blitCommandEncoder = nil;
	commandBuffer = nil;
#else
	MTLRegion region = MTLRegionMake2D(0, 0, slice_width, slice_height);
	
	[texture.texture replaceRegion:region
					   mipmapLevel:level
							 slice:slice
						 withBytes:data
					   bytesPerRow:bytes_per_row
					 bytesPerImage:bytes_per_image];
#endif

	
	return true;
}


bool mtl_GenTexture(uint32_t &buffer, NGL_texture_descriptor &texture_layout, std::vector<std::vector<uint8_t> > *datas)
{
	@autoreleasepool {
		if (texture_layout.m_is_renderable && datas)
		{
			_logf("GenTexture error: line: %d",__FILE__,__LINE__);
			return 0;
		}
		if (!(texture_layout.m_is_renderable || texture_layout.m_unordered_access) && !datas)
		{
			_logf("GenTexture error: line: %d",__FILE__,__LINE__);
			return 0;
		}
		
		if (buffer && buffer >= Metal_instance::This->m_textures.size())
		{
			_logf("GenTexture error: line: %d",__FILE__,__LINE__);
			buffer = 0xBADF00D;
			return false;
		}
		
		if (!buffer)
		{
			Metal_texture texture;
			
			Metal_instance::This->m_textures.push_back(texture);
			buffer = (uint32_t)Metal_instance::This->m_textures.size() - 1;
		}
		
		Metal_texture &texture = Metal_instance::This->m_textures[buffer];
		
		texture.m_texture_descriptor = texture_layout;
		texture.m_is_color = true;
		
		
		//
		//	Setup texture descriptor
		//
		uint32_t stride = 0;
		
		bool compressed = false;
		uint32_t block_dim_x, block_dim_y, block_dim_z,block_size;
		block_size = block_dim_x = block_dim_y = block_dim_z = 0;
		
		MTLTextureDescriptor *textureDescriptor = nullptr;
		
		switch( texture_layout.m_format)
		{
			case NGL_R8_G8_B8_UNORM:
			case NGL_R8_G8_B8_A8_UNORM:
			{
				stride = 4;
				texture.pixel_format = MTLPixelFormatRGBA8Unorm;
				texture.m_is_color = true;
				break;
			}
			case NGL_R8_G8_B8_UNORM_SRGB:
			case NGL_R8_G8_B8_A8_UNORM_SRGB:
			{
				stride = 4;
				texture.pixel_format = MTLPixelFormatRGBA8Unorm_sRGB;
				texture.m_is_color = true;
				break;
			}
			case NGL_R8_UNORM:
			{
				stride = 1;
				texture.pixel_format = MTLPixelFormatR8Unorm;
				texture.m_is_color = true;
				break;
			}
			case NGL_R10_G10_B10_A2_UNORM:
			{
				stride = 4;
				texture.pixel_format = MTLPixelFormatRGB10A2Unorm;
				break;
			}
			case NGL_R16_G16_B16_A16_FLOAT:
			{
				stride = 8;
				texture.pixel_format = MTLPixelFormatRGBA16Float;
				texture.m_is_color = true;
				break;
			}
			case NGL_R16_G16_FLOAT:
			{
				stride = 4;
				texture.pixel_format = MTLPixelFormatRG16Float;
				texture.m_is_color = true;
				break;
			}
			case NGL_R16_FLOAT:
			{
				stride = 2;
				texture.pixel_format = MTLPixelFormatR16Float;
				texture.m_is_color = true;
				break;
			}
			case NGL_R32_G32_B32_A32_FLOAT:
			{
				stride = 16;
				texture.pixel_format = MTLPixelFormatRGBA32Float;
				texture.m_is_color = true;
				break;
			}
			case NGL_R32_FLOAT:
			{
				stride = 4;
				texture.pixel_format = MTLPixelFormatR32Float;
				texture.m_is_color = true;
				break;
			}
			case NGL_R11_B11_B10_FLOAT:
			{
				stride = 4;
				texture.pixel_format = MTLPixelFormatRG11B10Float;
				texture.m_is_color = true;
				break;
			}
			case NGL_R9_G9_B9_E5_SHAREDEXP:
			{
				stride = 4;
				texture.pixel_format = MTLPixelFormatRGB9E5Float;
				texture.m_is_color = true;
				break;
			}

				
				// Compressed
			case NGL_R8_G8_B8_ETC2_UNORM:
			case NGL_R8_G8_B8_ETC2_UNORM_SRGB:
			{
				compressed = true;
				block_dim_x = block_dim_y = 4;
				block_dim_z = 1;
				block_size = 64;
				
                if (@available(macOS 11.0, iOS 9.0, *)) {
                    texture.pixel_format = (texture_layout.m_format == NGL_R8_G8_B8_ETC2_UNORM)?MTLPixelFormatETC2_RGB8:MTLPixelFormatETC2_RGB8_sRGB;
                }
				texture.m_is_color = true;
				break;
			}
			case NGL_R8_G8_B8_A8_ETC2_UNORM:
			case NGL_R8_G8_B8_A8_ETC2_UNORM_SRGB:
			{
				compressed = true;
				block_dim_x = block_dim_y = 4;
				block_dim_z = 1;
				block_size = 128;
				
                if (@available(macOS 11.0, iOS 9.0, *)) {
                    texture.pixel_format = (texture_layout.m_format == NGL_R8_G8_B8_A8_ETC2_UNORM)?MTLPixelFormatEAC_RGBA8:MTLPixelFormatEAC_RGBA8_sRGB;
                }
				texture.m_is_color = true;
				break;
			}
			case NGL_R8_G8_B8_A8_ASTC_4x4_UNORM:
			case NGL_R8_G8_B8_A8_ASTC_4x4_UNORM_SRGB:
			{
				compressed = true;
				block_dim_x = block_dim_y = 4;
				block_dim_z = 1;
				block_size = 128;
				
                if (@available(macOS 11.0, iOS 9.0, *)) {
                    texture.pixel_format = (texture_layout.m_format == NGL_R8_G8_B8_A8_ASTC_4x4_UNORM)?MTLPixelFormatASTC_4x4_LDR:MTLPixelFormatASTC_4x4_sRGB;
                }
				texture.m_is_color = true;
				break;
			}
			case NGL_R8_G8_B8_A8_ASTC_6x6_UNORM:
			case NGL_R8_G8_B8_A8_ASTC_6x6_UNORM_SRGB:
			{
				compressed = true;
				block_dim_x = block_dim_y = 6;
				block_dim_z = 1;
				block_size = 128;
				
                if (@available(macOS 11.0, iOS 9.0, *)) {
                    texture.pixel_format = (texture_layout.m_format == NGL_R8_G8_B8_A8_ASTC_6x6_UNORM)?MTLPixelFormatASTC_6x6_LDR:MTLPixelFormatASTC_6x6_sRGB;
                }
				texture.m_is_color = true;
				break;
			}
#if !TARGET_OS_IPHONE
			case NGL_R8_G8_B8_DXT1_UNORM:
			case NGL_R8_G8_B8_A1_DXT1_UNORM:
			{
				compressed = true;
				block_dim_x = block_dim_y = 4;
				block_dim_z = 1;
				block_size = 64;

				texture.pixel_format = MTLPixelFormatBC1_RGBA;
				texture.m_is_color = true;
				break;
			}
			case NGL_R8_G8_B8_DXT1_UNORM_SRGB:
			case NGL_R8_G8_B8_A1_DXT1_UNORM_SRGB:
			{
				compressed = true;
				block_dim_x = block_dim_y = 4;
				block_dim_z = 1;
				block_size = 64;

				texture.pixel_format = MTLPixelFormatBC1_RGBA_sRGB;
				texture.m_is_color = true;
				break;
			}
			case NGL_R8_G8_B8_A8_DXT5_UNORM:
			case NGL_R8_G8_B8_A8_DXT5_UNORM_SRGB:
			{
				compressed = true;
				block_dim_x = block_dim_y = 4;
				block_dim_z = 1;
				block_size = 128;
				
				texture.pixel_format = (texture_layout.m_format == NGL_R8_G8_B8_A8_DXT5_UNORM)?MTLPixelFormatBC3_RGBA:MTLPixelFormatBC3_RGBA_sRGB;
				texture.m_is_color = true;
				break;
			}
#endif
				// Depth
			case NGL_D16_UNORM:
			case NGL_D24_UNORM:
			{
				stride = 4;
				texture.pixel_format = MTLPixelFormatDepth32Float;
				texture.m_is_color = false;
				
				break;
			}
			default:
			{
				assert(0);
				printf("Unknown texture format\n") ;
			}
		}
		
		
		switch (texture_layout.m_type)
		{
			case NGL_TEXTURE_2D:
				textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:texture.pixel_format
																					   width:texture_layout.m_size[0]
																					  height:texture_layout.m_size[1]
																				   mipmapped:texture_layout.m_filter > 1];
				textureDescriptor.textureType = MTLTextureType2D;
				textureDescriptor.mipmapLevelCount = texture_layout.m_num_levels;
				break;
				
			case NGL_TEXTURE_CUBE:
				textureDescriptor = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:texture.pixel_format
																						  size:texture_layout.m_size[0]
																					 mipmapped:texture_layout.m_filter > 1];
				textureDescriptor.textureType = MTLTextureTypeCube;
				textureDescriptor.mipmapLevelCount = texture_layout.m_num_levels;
				break;
				
			case NGL_TEXTURE_2D_ARRAY:
				textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:texture.pixel_format
																					   width:texture_layout.m_size[0]
																					  height:texture_layout.m_size[1]
																				   mipmapped:texture_layout.m_filter > 1];
				textureDescriptor.textureType = MTLTextureType2DArray;
				textureDescriptor.mipmapLevelCount = (NSUInteger)texture_layout.m_num_levels;
				textureDescriptor.arrayLength = (NSUInteger)texture_layout.m_num_array;
				break;
				
			default:
				// unhandled format;
				assert(0);
				break;
		}
		
		
#if UPLOAD_TEXTURES_TO_PRIVATE_MEMORY
		textureDescriptor.storageMode = MTLStorageModePrivate;
#endif
		if (!texture.m_is_color)
		{
			textureDescriptor.storageMode = MTLStorageModePrivate;
		}
		
		
		textureDescriptor.usage = MTLTextureUsageShaderRead;
		if (texture_layout.m_unordered_access)
		{
			textureDescriptor.usage |= MTLTextureUsageShaderWrite;
		}
		if (texture_layout.m_is_renderable)
		{
			textureDescriptor.usage |= MTLTextureUsageRenderTarget;
			textureDescriptor.storageMode = MTLStorageModePrivate;
		}
		
#if TARGET_OS_IOS
		if (texture.m_texture_descriptor.m_memoryless)
		{
			textureDescriptor.storageMode = MTLStorageModeMemoryless;
		}
#endif
		
		texture.texture = [Metal_instance::This->device newTextureWithDescriptor:textureDescriptor];
		
		
		//
		//	Upload texture data
		//
		if (datas != nullptr)
		{
			std::vector<std::vector<uint8_t> > converted_data;
			std::vector<std::vector<uint8_t> > *texture_data = nullptr;
			
			// generate rgba data if necessary
			{
				bool need_rgb8_to_rgba8_conversion =    (texture_layout.m_format == NGL_R8_G8_B8_UNORM)
				|| (texture_layout.m_format == NGL_R8_G8_B8_UNORM_SRGB);
				
				if (need_rgb8_to_rgba8_conversion)
				{
					converted_data.resize(datas->size());
					
					for (uint32_t i = 0; i < datas->size(); i++)
					{
						// Calculate the dimensions of the mipmap
						uint32_t mipmap_width = texture_layout.m_size[0] / (1 << i);
						uint32_t mipmap_height = texture_layout.m_size[1] / (1 << i);
						
						assert( datas->at(i).size() % 3 == 0);
						converted_data[i].resize(4 * datas->at(i).size() / 3) ;
						
						uint8_t *src_ptr = (uint8_t *)&datas->at(i)[0];
						uint8_t *dst_ptr = (uint8_t *)&converted_data[i][0];
						
						RGB888toRGBA8888(mipmap_width, mipmap_height, src_ptr, dst_ptr );
					}
					
					texture_data = &converted_data;
				}
				else
				{
					texture_data = datas;
				}
			}
			
			if (texture_layout.m_type == NGL_TEXTURE_2D)
			{
				for( uint32_t i=0; i < texture_data->size(); i++)
				{
					uint32_t bytes_per_row = 0;
					uint32_t bytes_per_image = 0;
					
					// Calculate the dimensions of the mipmap
					uint32_t mipmap_width = texture_layout.m_size[0] / (1 << i);
					uint32_t mipmap_height = texture_layout.m_size[1] / (1 << i);
					mipmap_width = mipmap_width ? mipmap_width : 1;
					mipmap_height = mipmap_height ? mipmap_height : 1;
					
					if (compressed)
					{
						// Calculate the number of blocks for the mipmap
						uint32_t block_count_x = (mipmap_width + block_dim_x - 1) / block_dim_x;
						uint32_t block_count_y = (mipmap_height + block_dim_y - 1) / block_dim_y;
						
						// Calculate the compressed data size
						bytes_per_row = block_count_x * block_size / 8;
						bytes_per_image = block_count_y * bytes_per_row;
					}
					else
					{
						bytes_per_row = mipmap_width * stride;
						bytes_per_image = bytes_per_row * mipmap_height;
					}
					
					mtl_UploadTextureSlice(texture, 0, i, mipmap_width, mipmap_height, bytes_per_row, bytes_per_image, &texture_data->at(i)[0]);
				}
			}
			else if (texture_layout.m_type == NGL_TEXTURE_CUBE)
			{
				uint32_t w = texture_layout.m_size[0];
				uint32_t h = texture_layout.m_size[1];
				
				for (size_t i = 0; i < datas->size(); i++)
				{
					uint32_t level = (uint32_t)i / 6;
					uint32_t face = (uint32_t)i % 6;
					
					// Calculate the dimensions of the mipmap
					uint32_t mipmap_width = w / (1 << level);
					uint32_t mipmap_height = h / (1 << level);
					mipmap_width = mipmap_width ? mipmap_width : 1;
					mipmap_height = mipmap_height ? mipmap_height : 1;
					
					uint32_t bytes_per_row = 0;
					uint32_t bytes_per_image = 0;
					
					if (compressed)
					{
						// Calculate the number of blocks for the mipmap
						uint32_t block_count_x = (mipmap_width + block_dim_x - 1) / block_dim_x;
						uint32_t block_count_y = (mipmap_height + block_dim_y - 1) / block_dim_y;
						
						// Calculate the compressed data size
						bytes_per_row = block_count_x * block_size / 8;
						bytes_per_image = block_count_y * bytes_per_row;
					}
					else
					{
						bytes_per_row = mipmap_width * stride;
						bytes_per_image = bytes_per_row * mipmap_height;
					}
					
					mtl_UploadTextureSlice(texture, face, level, mipmap_width, mipmap_height, bytes_per_row, bytes_per_image, &texture_data->at(i)[0]);
				}
			}
			else
			{
				assert(datas == nullptr);
			}
			
			// generate mipmaps if necessary
			bool generate_mipmap = true;
			generate_mipmap &= (texture_layout.m_filter > 1) && !compressed && ( datas->size() / texture_layout.m_num_array <= 1 );
			generate_mipmap &= texture_layout.m_num_levels > 1;
			if ( generate_mipmap )
			{
				printf("WARNING!! Runtime generation of mipmaps!\n");
				
				id <MTLCommandBuffer> commandBuffer = [Metal_instance::This->commandQueue commandBuffer];
				id <MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
				[blitCommandEncoder generateMipmapsForTexture:texture.texture];
				[blitCommandEncoder endEncoding];
				[commandBuffer commit];
				
#if 0
				[commandBuffer waitUntilScheduled];
				if ([commandBuffer error] != nil)
				{
					NSLog(@"%@\n",[commandBuffer error]); assert(0) ;
				}
				
				[commandBuffer waitUntilCompleted];
				if ([commandBuffer error] != nil)
				{
					NSLog(@"%@\n",[commandBuffer error]); assert(0) ;
				}
#endif
				
				blitCommandEncoder = nil;
				commandBuffer = nil;
			}
		}
		
		
		//LOG_TEXTURE(num_data_layouts, data_layouts, stride, 0, false);
		
		
		
		//
		//	create sampler
		//
		MTLSamplerDescriptor *samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
		
		switch( texture_layout.m_filter)
		{
			case NGL_NEAREST:
			{
				samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
				samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
				break;
			}
			case NGL_NEAREST_MIPMAPPED:
			{
				samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
				samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
				samplerDescriptor.mipFilter = MTLSamplerMipFilterNearest;
				break;
			}
			case NGL_LINEAR:
			{
				samplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
				samplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
				break;
			}
			case NGL_LINEAR_MIPMAPPED:
			{
				samplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
				samplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
				samplerDescriptor.mipFilter = MTLSamplerMipFilterLinear;
				break;
			}
			case NGL_ANISO_4:
			{
				samplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
				samplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
				samplerDescriptor.mipFilter = MTLSamplerMipFilterLinear;
				samplerDescriptor.maxAnisotropy = 4;
				break;
				
			}
			default:
			{
				printf("Warning! Unhandled texture filter mode!!\n") ;
				assert(0);
			}
		}
		
		if( texture_layout.m_wrap_mode == NGL_REPEAT_ALL)
		{
			samplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
			samplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
			samplerDescriptor.rAddressMode = MTLSamplerAddressModeRepeat;
		}
		
		if( texture_layout.m_wrap_mode == NGL_CLAMP_ALL)
		{
			samplerDescriptor.sAddressMode = MTLSamplerAddressModeClampToEdge;
			samplerDescriptor.tAddressMode = MTLSamplerAddressModeClampToEdge;
			samplerDescriptor.rAddressMode = MTLSamplerAddressModeClampToEdge;
		}
		
		texture.sampler = [Metal_instance::This->device newSamplerStateWithDescriptor:samplerDescriptor];
		
		return true;
	}
}


bool mtl_GenVertexBuffer(uint32_t &buffer, NGL_vertex_descriptor &vertex_layout, uint32_t num, void *data)
{
	if (buffer && buffer >= Metal_instance::This->m_vertex_buffers.size())
	{
		buffer = 0xBADF00D;
		return false;
	}
	
	if (!buffer)
	{
		Metal_vertex_buffer vb;
		
		Metal_instance::This->m_vertex_buffers.push_back(vb);
		buffer = (uint32_t)Metal_instance::This->m_vertex_buffers.size() - 1;
	}
	
	Metal_vertex_buffer &vb = Metal_instance::This->m_vertex_buffers[buffer];
	
	vb.m_hash = GenerateHash(vertex_layout);
	vb.m_vertex_descriptor = vertex_layout;
	vb.m_datasize = num * vertex_layout.m_stride;
	
	if (Metal_instance::This->device == nil)
	{
		printf("Error! Device is nil at buffer creation!!\n") ;
	}
	
	if (data != nullptr)
	{
#if TARGET_OS_IPHONE || defined(__arm64__)
		vb.buffer = [Metal_instance::This->device newBufferWithBytes:data length:vertex_layout.m_stride*num options:MTLResourceOptionCPUCacheModeDefault];
#else
		vb.buffer = [Metal_instance::This->device newBufferWithBytes:data length:vertex_layout.m_stride*num options:MTLResourceStorageModeManaged];
#endif
	}
	else
	{
#if TARGET_OS_IPHONE || defined(__arm64__)
		vb.buffer = [Metal_instance::This->device newBufferWithLength:vertex_layout.m_stride*num options:MTLResourceOptionCPUCacheModeDefault];
#else
		vb.buffer = [Metal_instance::This->device newBufferWithLength:vertex_layout.m_stride*num options:MTLResourceStorageModeManaged];
#endif
	}
	
	//LOG_BUFFER(vertex_layout,num);
	
	return true;
}


bool mtl_GenIndexBuffer(uint32_t &buffer, NGL_format format, uint32_t num, void *data)
{
	if (buffer && buffer >= Metal_instance::This->m_index_buffers.size())
	{
		buffer = 0xBADF00D;
		return false;
	}
	
	if (!buffer)
	{
		Metal_index_buffer ib;
		
		Metal_instance::This->m_index_buffers.push_back(ib);
		buffer = (uint32_t)Metal_instance::This->m_index_buffers.size() - 1;
	}
	
	Metal_index_buffer &ib = Metal_instance::This->m_index_buffers[buffer];
	
	ib.m_format = format;
	ib.m_num_indices = num;
	
	uint32_t stride = 0;
	
	if( format == NGL_R16_UINT)
	{
		ib.m_Metal_data_type = MTLIndexTypeUInt16;
		stride = 2;
	}
	if( format == NGL_R32_UINT)
	{
		ib.m_Metal_data_type = MTLIndexTypeUInt32;
		stride = 4;
	}
	
	ib.buffer = [Metal_instance::This->device newBufferWithBytes:data length:stride*num options:MTLResourceOptionCPUCacheModeDefault];
	
	return true;
}


bool mtl_GetVertexBufferContent(uint32_t buffer_id, NGL_resource_state state, std::vector<uint8_t> &data)
{
	if (buffer_id >= Metal_instance::This->m_vertex_buffers.size())
	{
		_logf("MTL - GetVertexBufferContent: Illegal vertex buffer id: %d!\n", buffer_id);
		return false;
	}
	
	const Metal_vertex_buffer &ngl_buffer = Metal_instance::This->m_vertex_buffers[buffer_id];
	data.resize(ngl_buffer.m_datasize);
	
	if (ngl_buffer.m_datasize == 0)
	{
		_logf("MTL - GetVertexBufferContent: Warning! Buffer (%d) size is zero!: %d!\n", buffer_id);
		return true;
	}
	
	id <MTLBuffer> mtl_buffer = ngl_buffer.buffer;
	
	if ([mtl_buffer storageMode] == MTLStorageModePrivate)
	{
		// private buffers not supported yet
		assert(0);
	}
	
	for (uint32_t i = 0; i < Metal_instance::MAX_COMMAND_BUFFER_COUNT; i++)
	{
		id <MTLCommandBuffer> cb = Metal_instance::This->m_command_buffers[i];
		
		if (cb == nil) continue;
		
		if ((Metal_instance::This->m_command_buffer_status[i] == NGL_COMMAND_BUFFER_STARTED)
			|| (Metal_instance::This->m_command_buffer_status[i] == NGL_COMMAND_BUFFER_ENDED))
		{
			[cb commit];
		}

		Metal_instance::This->m_command_buffers[i] = [[cb commandQueue] commandBuffer];
	}
	
#if !TARGET_OS_IPHONE && !defined(__arm64__)
	id <MTLCommandBuffer> finishBuffer = [Metal_instance::This->commandQueue commandBuffer];
	if ([mtl_buffer storageMode] == MTLStorageModeManaged)
	{
		id <MTLBlitCommandEncoder> blitEncoder = [finishBuffer blitCommandEncoder];
		[blitEncoder synchronizeResource:mtl_buffer];
		[blitEncoder endEncoding];
	}
	[finishBuffer commit];
	[finishBuffer waitUntilCompleted];
#endif
	
	// Map the buffer and copy the contents
	void *src_ptr = [mtl_buffer contents];
	if (src_ptr)
	{
		memcpy(data.data(), src_ptr, ngl_buffer.m_datasize);
		return true;
	}
	else
	{
		_logf("MTL - GetVertexBufferContent: Can not map buffer: %d!\n", buffer_id);
		return false;
	}
}

