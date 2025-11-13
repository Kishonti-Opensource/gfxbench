/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_cubemap.h"

#include "platform.h"
#include "ng/log.h"

#include <cassert>

#include "mtl_texture.h"



CubeEnvMap_Metal::CubeEnvMap_Metal( int size)
{
    
}


CubeEnvMap_Metal* CubeEnvMap_Metal::Load( KCL::uint32 idx, const char* path, bool mipmap )
{
    char name[1024] = {0};
    sprintf(name, "%senvmap%03d", path, idx);
    
    KCL::uint32 flags = KCL::TC_Clamp | KCL::TC_Commit;
    if(!mipmap)
    {
        flags |= KCL::TC_NoMipmap;
    }
    
    MetalRender::TextureFactory f;
    
    KCL::Texture* tex = f.CreateAndSetup( KCL::Texture_Cube, name, flags);
    
    if( tex)
    {
        CubeEnvMap_Metal *cem = new CubeEnvMap_Metal( -1);
        cem->m_texture = tex;
        
        return cem;
    }
    
    return 0;
}




CubeEnvMap_Metal::~CubeEnvMap_Metal()
{
    delete m_texture ;
}



ParaboloidEnvMap::ParaboloidEnvMap(int size) : m_texture(nil)
{
}


ParaboloidEnvMap* ParaboloidEnvMap::Load( KCL::uint32 idx, const char* path, bool mipmap )
{
	return 0;
}


void ParaboloidEnvMap::Save( KCL::uint32 idx, KCL::uint32 target, KCL::uint32 size, const char* path )
{

}


ParaboloidEnvMap::~ParaboloidEnvMap()
{
	releaseObj(m_texture);
}



FboEnvMap::FboEnvMap(id<MTLDevice> device, int cubemapSize) :
	m_device(device),
	m_cubemapSize(cubemapSize)
{
	// Create the shadow array texture
	MTLTextureDescriptor * tex_desc = [[MTLTextureDescriptor alloc] init];

	tex_desc.textureType = MTLTextureType2D;
	tex_desc.pixelFormat = MTLPixelFormatDepth32Float;
	tex_desc.width = m_cubemapSize;
	tex_desc.height = m_cubemapSize;
	tex_desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_EMBEDDED
	tex_desc.storageMode = MTLStorageModePrivate;
#endif

	m_depth_texture = [device newTextureWithDescriptor:tex_desc];
}

FboEnvMap::~FboEnvMap()
{
	releaseObj(m_depth_texture);
	m_cube_texture = NULL;
}

id<MTLRenderCommandEncoder> FboEnvMap::AttachCubemap(id<MTLCommandBuffer> command_buffer, CubeEnvMap_Metal * const cubemap, size_t face)
{
	assert(face < 6);
	// TODO
	return nil;
}

id<MTLRenderCommandEncoder> FboEnvMap::AttachParaboloid(id<MTLCommandBuffer> command_buffer, ParaboloidEnvMap * const paraboloidmap, size_t layer)
{
	assert(layer < 2);
	MTLRenderPassDescriptor * desc = [[MTLRenderPassDescriptor alloc] init];

	desc.colorAttachments[0].texture = paraboloidmap->m_texture;
	desc.colorAttachments[0].loadAction = MTLLoadActionClear;
	desc.colorAttachments[0].storeAction = MTLStoreActionStore;
    desc.colorAttachments[0].clearColor = MTLClearColorMake( 0.0f, 0.0f, 0.0f, 0.0f );
	desc.colorAttachments[0].slice = layer;
	desc.depthAttachment.texture = m_depth_texture;
	desc.depthAttachment.loadAction = MTLLoadActionClear;
	desc.depthAttachment.storeAction = MTLStoreActionDontCare;

	id<MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:desc];
	[encoder setFrontFacingWinding:MTLWindingCounterClockwise];

	return encoder;

}

CubeEnvMap_Metal* FboEnvMap::CreateCubeEnvMapRGBA()
{
	NGLOG_FATAL("CreateCubeEnvMapRGBA not implemented");

	CubeEnvMap_Metal *result = new CubeEnvMap_Metal(m_cubemapSize);

	return result;
}


CubeEnvMap_Metal* FboEnvMap::CreateCubeEnvMapWithFormat(MTLPixelFormat texture_format)
{
	CubeEnvMap_Metal * result = new CubeEnvMap_Metal(m_cubemapSize);

	NGLOG_FATAL("CreateCubeEnvMapRGBA not implemented");

	return result;
}

ParaboloidEnvMap* FboEnvMap::CreateParaboloidEnvMapWithFormat(MTLPixelFormat texture_format)
{
	ParaboloidEnvMap *result = new ParaboloidEnvMap(m_cubemapSize);

	KCL::uint32 m_uiMipMapCount = 1;

	KCL::uint32 kk = m_cubemapSize;
	while( kk > 1)
	{
		m_uiMipMapCount++;
		kk /= 2;
	}

	// Create the shadow array texture
	MTLTextureDescriptor * tex_desc = [[MTLTextureDescriptor alloc] init];

	tex_desc.textureType = MTLTextureType2DArray;
	tex_desc.pixelFormat = texture_format;
	tex_desc.width = m_cubemapSize;
	tex_desc.height = m_cubemapSize;
	tex_desc.mipmapLevelCount = m_uiMipMapCount;
	tex_desc.arrayLength = 2;
	tex_desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_EMBEDDED
	tex_desc.storageMode = MTLStorageModePrivate;
#endif

	result->m_texture = [m_device newTextureWithDescriptor:tex_desc];
	releaseObj(tex_desc);

	return result;
}

