/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl_metal.h"


MTLLoadAction NGLLoadToMTLLoad(NGL_attachment_load_op lo)
{
	switch (lo)
	{
		case NGL_LOAD_OP_CLEAR: return MTLLoadActionClear;
		case NGL_LOAD_OP_DONT_CARE: return MTLLoadActionDontCare;
		case NGL_LOAD_OP_LOAD: return MTLLoadActionLoad;
			
		default:
			assert(0);
			break;
	}
	return MTLLoadActionLoad;
}


MTLStoreAction NGLStoreToMTLStore(NGL_attachment_store_op so)
{
	switch (so)
	{
		case NGL_STORE_OP_DONT_CARE: return MTLStoreActionDontCare;
		case NGL_STORE_OP_STORE: return MTLStoreActionStore;
			
		default:
			assert(0);
			break;
	}
	return MTLStoreActionStore;
}


bool mtl_GetTextureContent(uint32_t texture_, uint32_t level, uint32_t layer, uint32_t face, NGL_format format, NGL_resource_state state, uint32_t &width, uint32_t &height, std::vector<uint8_t> &data)
{
	if ((format != NGL_R8_G8_B8_UNORM) && (format != NGL_R8_G8_B8_A8_UNORM))
	{
		return false;
	}
	
	id <MTLTexture> texture = nil;
	
	if (texture_ == 0)
	{
		texture = Metal_instance::This->backbuffer_provider->GetBackBuffer();
	}
	else
	{
		texture = Metal_instance::This->m_textures[texture_].texture;
	}
	
	MTLPixelFormat pixel_format = [texture pixelFormat];
	if (pixel_format != MTLPixelFormatBGRA8Unorm && pixel_format != MTLPixelFormatRGBA8Unorm)
	{
		return false;
	}
	
	id <MTLCommandBuffer> finishBuffer = [Metal_instance::This->commandQueue commandBuffer];
	
	width = (uint32_t)texture.width / (1 << level);
	height = (uint32_t)texture.height / (1 << level);
	
	MTLTextureDescriptor* textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixel_format
																								 width:width
																								height:height
																							 mipmapped:false];
	
	id <MTLTexture> texture_managed = [Metal_instance::This->device newTextureWithDescriptor:textureDescriptor];
	
	id <MTLBlitCommandEncoder> blit_command_encoder = [finishBuffer blitCommandEncoder];
	[blit_command_encoder copyFromTexture:texture
							  sourceSlice:layer
							  sourceLevel:level
							 sourceOrigin:MTLOriginMake(0, 0, 0)
							   sourceSize:MTLSizeMake(width, height, 1)
								toTexture:texture_managed
						 destinationSlice:0
						 destinationLevel:0
						destinationOrigin:MTLOriginMake(0, 0, 0)];
#if !TARGET_OS_IPHONE && !defined(__arm64__)
	[blit_command_encoder synchronizeResource:texture_managed];
#endif
	[blit_command_encoder endEncoding];
	
	[finishBuffer commit] ;
	[finishBuffer waitUntilCompleted] ;
	finishBuffer = nil ;
	
	
	// MTL_TODO only works for 32bpp textures now
	uint32_t srcRowBytes = (uint32_t)width * 4;
	uint32_t imageBytes = srcRowBytes * (uint32_t)height;
	
	uint8_t* rgbaPixelBuffer = new uint8_t[imageBytes] ;
	
	[texture_managed getBytes:rgbaPixelBuffer
		  bytesPerRow:srcRowBytes
		bytesPerImage:imageBytes
		   fromRegion:MTLRegionMake2D(0, 0, width, height)
		  mipmapLevel:level
				slice:layer];
	
	bool rgba = format == NGL_R8_G8_B8_A8_UNORM;
	uint32_t cc = rgba?4:3;
	
	data.resize(cc*width*height) ;
	
	uint32_t dstRowBytes = (uint32_t)texture.width * cc;
	
	// MTL_TODO use util methods if possible
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			uint32_t dst_id = i*dstRowBytes+cc*j ;
			uint32_t src_id = ( height-(i+1) )*srcRowBytes+4*j ;
			
			if (pixel_format == MTLPixelFormatRGBA8Unorm)
			{
				data[dst_id+0] = rgbaPixelBuffer[src_id+0] ;
				data[dst_id+1] = rgbaPixelBuffer[src_id+1] ;
				data[dst_id+2] = rgbaPixelBuffer[src_id+2] ;
			}
			else
			{
				data[dst_id+0] = rgbaPixelBuffer[src_id+2] ;
				data[dst_id+1] = rgbaPixelBuffer[src_id+1] ;
				data[dst_id+2] = rgbaPixelBuffer[src_id+0] ;
			}
			
			if (rgba)
			{
				data[dst_id+3] = rgbaPixelBuffer[src_id+3] ;
			}
		}
	}
	
	delete[] rgbaPixelBuffer ;
 
	return true ;
}

