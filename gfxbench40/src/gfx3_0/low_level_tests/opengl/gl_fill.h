/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "fill_base.h"
#include "kcl_base.h"
#include "kcl_texture.h"

class CompressedFillTest : public CompressedFillTest_Base
{
public:
	CompressedFillTest(const GlobalTestEnvironment* const gte);
	virtual ~CompressedFillTest();

protected:
	virtual KCL::KCL_Status init ();
	virtual bool render();
	virtual KCL::Texture* CreateTexture(KCL::Image* img) ;
	
	virtual void FreeResources();

private:
	float m_aspectRatio;

	static const int m_stride = 5 * sizeof(float); // distance between the beginning of vertices
	KCL::uint32 m_vertexBuffer;
	KCL::uint32 m_indexBuffer;

	KCL::uint32 m_shader;
	int m_rotationLocation;
	int m_sampler0Location;
	int m_sampler1Location;
	int m_uniAspectRatio;
};

