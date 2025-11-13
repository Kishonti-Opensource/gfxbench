/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_base.h>

namespace GFXB
{
	class Shapes
	{
	public:
		Shapes();
		virtual ~Shapes();

		void Init();

		// Just keep them public for sake of simplicity
		KCL::uint32 m_fullscreen_vbid;
		KCL::uint32 m_fullscreen_ibid;
		KCL::uint32 m_sphere_vbid;
		KCL::uint32 m_sphere_ibid;
		KCL::uint32 m_cone_vbid;
		KCL::uint32 m_cone_ibid;
		KCL::uint32 m_cube_vbid;
		KCL::uint32 m_cube_ibid;
		KCL::uint32 m_cylinder_vbid;
		KCL::uint32 m_cylinder_ibid;
		KCL::uint32 m_line_vbid;
		KCL::uint32 m_line_ibid;

		static const KCL::Vector3D m_cube_vertices[8];

	private:
	};
}