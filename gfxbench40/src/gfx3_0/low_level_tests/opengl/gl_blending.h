/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "blending_base.h"
#include "kcl_base.h"
#include "kcl_math3d.h"
#include "kcl_texture.h"

class UITest : public UITest_Base
{
private:
	float m_scaleX;
	float m_scaleY;
	float m_aspectRatio;

	static const int m_stride = 5 * sizeof(float); // distance between the beginning of vertices
	KCL::uint32 m_vertexBuffer;
	KCL::uint32 m_indexBuffer;

	KCL::uint32 m_shader;
	int m_scaleLocation;
	int m_offsetLocation;
	int m_samplerLocation;

public:
	UITest(const GlobalTestEnvironment* const gte);
	~UITest();

protected:
	virtual const char* getUom() const { return "MB/s"; }
	virtual bool isWarmup() const { return false; }
	virtual KCL::uint32 indexCount() const { return 0; }
	
	virtual KCL::KCL_Status init ();
	virtual bool render();

	virtual KCL::Texture* createTexture(KCL::Image* image) ;
	
	virtual void FreeResources();
};

