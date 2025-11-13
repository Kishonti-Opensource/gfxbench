/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __TESS_BASE__
#define __TESS_BASE__

#include "test_base.h"
#include <kcl_math3d.h>

namespace KCL
{
	class Camera2;
}

class TessBase : public GLB::TestBase
{
public:
	TessBase(const GlobalTestEnvironment * const gte);
	virtual ~TessBase();

protected:
	struct Mesh
	{
		KCL::Matrix4x4 model;
		KCL::Matrix4x4 mv;
		KCL::Matrix4x4 mvp;
	};

	KCL::Camera2 *m_camera;
	KCL::uint32 m_score;

	std::vector<KCL::Vector3D> m_vertices;
	std::vector<KCL::uint16> m_indices;

	Mesh m_mesh1;
	Mesh m_mesh2;
	Mesh m_mesh3;

	virtual KCL::KCL_Status init();
	virtual bool animate(const int time);
};

#endif  // __TESS_BASE__

