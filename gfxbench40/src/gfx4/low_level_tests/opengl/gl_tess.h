/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __GL_TESS__
#define __GL_TESS__

#include "low_level_tests/tess_base.h"

namespace GLB
{
class GLBShader2;
}


class TessTest : public TessBase
{
public:
	TessTest(const GlobalTestEnvironment * const gte);
	virtual ~TessTest();


private:
	KCL::uint32 m_vbo;
	KCL::uint32 m_ebo;
	KCL::uint32 m_vao;

	GLB::GLBShader2 *m_shader_geom;
	GLB::GLBShader2 *m_shader;

	virtual float getScore () const { return m_score; }
	virtual const char* getUom() const { return "frames"; }
	virtual bool isWarmup() const { return false; }
	virtual KCL::uint32 indexCount() const { return 0; }
	
	virtual KCL::KCL_Status init ();
	virtual bool animate (const int time);
	virtual bool render ();
	
	virtual void FreeResources();

	void RenderMesh(const Mesh &mesh, const GLB::GLBShader2 *shader);
	KCL::KCL_Status LoadShaders();
};

#endif
