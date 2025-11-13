/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GL_DRIVER2_H
#define GL_DRIVER2_H

#include "test_base.h"
#include "kcl_base.h"
#include "opengl/glb_texture.h"

class DriverOverheadTest2 : public GLB::TestBase
{
private:
	KCL::uint32 m_global_vao;

	KCL::uint32 m_vbuf_sqr;
	KCL::uint32 m_ibuf_sqr;

	KCL::uint32 m_vbuf_tri;
	KCL::uint32 m_ibuf_tri;

	KCL::uint32 m_vbuf_dot;
	KCL::uint32 m_ibuf_dot;

	int m_score;
	KCL::uint32 m_shader;
	KCL::uint32 m_renderglowshader;
	int m_uniPosition;
	int m_uniGridSize;
	int m_uniColor;
	int m_uniMargin;
	int m_uniGlobalMargin;
	int m_uniTransMat;
	float m_transformmat[4];
	GLB::FBO* m_glowfbo;
	GLB::GLBTexture* m_normal_tex;
	GLB::GLBTexture* m_burnt_tex;
	GLB::GLBTexture* m_burnt2_tex;
	KCL::uint32 m_sampler;

	void RenderChannels(int RowCount, int ColumnCount, float* channels, float backgroundIntensity);
	void RenderGlow(int RowCount, int ColumnCount, float* channels);

public:
	DriverOverheadTest2(const GlobalTestEnvironment* const gte);
	~DriverOverheadTest2();

protected:
	virtual const char* getUom() const { return "frames"; }
	virtual bool isWarmup() const { return false; }
	virtual KCL::uint32 indexCount() const { return 0; }
	virtual float getScore () const { return m_score; }
	
	virtual KCL::KCL_Status init ();
	virtual bool animate (const int time);
	virtual bool render ();
	
	virtual void FreeResources();
};

#endif