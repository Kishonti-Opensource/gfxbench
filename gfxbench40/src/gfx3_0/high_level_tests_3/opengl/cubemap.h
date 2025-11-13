/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __XX__CUBEMAP_H__
#define __XX__CUBEMAP_H__

#include "krl_cubemap.h"
#include "kcl_scene_version.h"


class CubeEnvMap : public KRL_CubeEnvMap
{
	friend class FboEnvMap;
	CubeEnvMap( int size);

public:
	~CubeEnvMap();

	static CubeEnvMap* Load( KCL::uint32 idx, const char* path, bool mipmap = false);
	void Save( KCL::uint32 idx, KCL::uint32 target, KCL::uint32 size, const char* path);
	unsigned int GetId()
	{
		return m_id;
	}
private:
	CubeEnvMap(const CubeEnvMap&);
	CubeEnvMap& operator=(const CubeEnvMap&);

	unsigned int m_id;
};

class ParaboloidEnvMap : public KRL_ParaboloidMap
{
	friend class FboEnvMap;
	ParaboloidEnvMap( int size);

public:
	~ParaboloidEnvMap();

	static ParaboloidEnvMap* Load( KCL::uint32 idx, const char* path, bool mipmap = false);
	void Save( KCL::uint32 idx, KCL::uint32 target, KCL::uint32 size, const char* path);
	unsigned int GetId()
	{
		return m_id;
	}
private:
	ParaboloidEnvMap(const ParaboloidEnvMap&);
	ParaboloidEnvMap& operator=(const ParaboloidEnvMap&);

	unsigned int m_id;
};

class FboEnvMap
{
public:
	FboEnvMap(int cubemapSize = 256);

	~FboEnvMap();
	
	void Bind();
	
	void Unbind();
	
	void AttachCubemap(CubeEnvMap *const cubemap, size_t face);
    void AttachParaboloid(ParaboloidEnvMap *const paraboloidmap, size_t layer);

	void DetachCubemap(size_t face);
    void DetachParaboloid(size_t layer);

	int GetSize() const { return m_cubemapSize; }
	
    CubeEnvMap* CreateCubeEnvMapRGBA();
	CubeEnvMap* CreateCubeEnvMapWithFormat(KCL::uint32 textureformat);
	ParaboloidEnvMap* CreateParaboloidEnvMapWithFormat(KCL::uint32 textureformat);

private:
	FboEnvMap(const FboEnvMap&);
	FboEnvMap& operator=(const FboEnvMap&);
	
	unsigned int m_id;
	unsigned int m_rboid;
	const int m_cubemapSize;
};

bool GenerateNormalisationCubeMap( KCL::uint32 &texture_object, KCL::uint32 size);


#endif //__XX__CUBEMAP_H__
