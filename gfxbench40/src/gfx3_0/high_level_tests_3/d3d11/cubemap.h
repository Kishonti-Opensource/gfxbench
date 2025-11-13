/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __XX__CUBEMAP_H__
#define __XX__CUBEMAP_H__

#include "krl_cubemap.h"

#include <kcl_math3d.h>
#include <kcl_os.h>
#include <glb_kcl_adapter.h>

#include <vector>
#include <cfloat>

#include "d3d11/DX.h"

namespace GLB
{
	class Image2D;
}

using namespace GLB;


class CubeEnvMap : public KRL_CubeEnvMap
{
	friend class FboEnvMap;
	CubeEnvMap(int size = 256);

public:
	~CubeEnvMap();

	static CubeEnvMap* Load( KCL::uint32 idx, const char* path);
	void Save( KCL::uint32 idx, KCL::uint32 target, KCL::uint32 size, const char* path);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> getD3D11Id() { return m_d3d11_id; }
private:
	CubeEnvMap(const CubeEnvMap&);
	CubeEnvMap& operator=(const CubeEnvMap&);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_d3d11_id;
};


class FboEnvMap
{
public:
	FboEnvMap(int cubemapSize = 256);

	~FboEnvMap();
	
	void Bind();
	
	void Unbind();
	
	void AttachCubemap(CubeEnvMap *const cubemap, size_t face);

	void DetachCubemap(size_t face);

	int GetSize() const { return m_cubemapSize; }

	CubeEnvMap* CreateCubeEnvMap(const Vector3D& position = Vector3D())
	{
		CubeEnvMap *result = new CubeEnvMap(m_cubemapSize);
		result->SetPosition(position);
		return result;
	}

private:
	FboEnvMap(const FboEnvMap&);
	FboEnvMap& operator=(const FboEnvMap&);
	
	unsigned int m_id;
	unsigned int m_rboid;
	const int m_cubemapSize;
};

bool GenerateNormalisationCubeMap( KCL::uint32 &texture_object, KCL::uint32 size);


#endif //__XX__CUBEMAP_H__
