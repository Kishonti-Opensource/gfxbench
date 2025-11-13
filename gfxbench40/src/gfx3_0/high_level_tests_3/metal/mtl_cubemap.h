/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXBench_mtl_cubemap_h
#define GFXBench_mtl_cubemap_h


#include "krl_cubemap.h"
#include "kcl_scene_version.h"
#include "mtl_globals.h"


class CubeEnvMap_Metal : public KRL_CubeEnvMap
{
    CubeEnvMap_Metal( int size);
	friend class FboEnvMap;
    
public:
    ~CubeEnvMap_Metal();
    
    static CubeEnvMap_Metal* Load( KCL::uint32 idx, const char* path, bool mipmap);

private:
    CubeEnvMap_Metal(const CubeEnvMap_Metal&);
    CubeEnvMap_Metal& operator=(const CubeEnvMap_Metal&);
    
};


class ParaboloidEnvMap : public KRL_ParaboloidMap
{
	friend class FboEnvMap;
	ParaboloidEnvMap(int size);

public:
	~ParaboloidEnvMap();

	static ParaboloidEnvMap* Load( KCL::uint32 idx, const char* path, bool mipmap = false);
	void Save( KCL::uint32 idx, KCL::uint32 target, KCL::uint32 size, const char* path);
	id<MTLTexture> GetTexture()
	{
		return m_texture;
	}
private:
	ParaboloidEnvMap(const ParaboloidEnvMap&);
	ParaboloidEnvMap& operator=(const ParaboloidEnvMap&);

	id<MTLTexture> m_texture;
};


class FboEnvMap
{
public:
	FboEnvMap(id<MTLDevice> device, int cubemapSize = 256);

	~FboEnvMap();

	id<MTLRenderCommandEncoder> AttachCubemap(id<MTLCommandBuffer> command_buffer, CubeEnvMap_Metal * const cubemap, size_t face);
	id<MTLRenderCommandEncoder> AttachParaboloid(id<MTLCommandBuffer> command_buffer, ParaboloidEnvMap * const paraboloidmap, size_t layer);

	int GetSize() const { return m_cubemapSize; }

	CubeEnvMap_Metal * CreateCubeEnvMapRGBA();
	CubeEnvMap_Metal * CreateCubeEnvMapWithFormat(MTLPixelFormat textureformat);
	ParaboloidEnvMap * CreateParaboloidEnvMapWithFormat(MTLPixelFormat textureformat);

private:
	FboEnvMap(const FboEnvMap&);
	FboEnvMap& operator=(const FboEnvMap&);

	id<MTLDevice> m_device;
	id<MTLTexture> m_depth_texture;
	id<MTLTexture> m_cube_texture;

	const int m_cubemapSize;
};

#endif
