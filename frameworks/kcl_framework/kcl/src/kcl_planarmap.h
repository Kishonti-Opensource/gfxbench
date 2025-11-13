/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_PLANARMAP_H
#define KCL_PLANARMAP_H

#include <kcl_base.h>
#include <kcl_math3d.h>
#include <kcl_camera2.h>
#include <kcl_aabb.h>
#include <kcl_mesh.h>
#include <kcl_room.h>

#include <vector>
#include <float.h>

#include <kcl_os.h>

namespace KCL
{

	/*struct Mesh;
	class Shader;
	struct XRoom;*/

	class PlanarMap
	{
	public:
		static KCL::PlanarMap* Create( int w, int h, const char *name);
		virtual ~PlanarMap() {}

		AABB m_aabb;
		Camera2 m_camera;
		Vector4D m_plane;
		XRoom *m_room;
		std::vector<Mesh*> m_receiver_meshes;
		std::vector<Mesh*> m_visible_meshes[2];

		int m_width;
		int m_height;

		KCL::uint32 m_frame_when_rendered;
		std::string m_name;

	protected:
		PlanarMap( int w, int h, const char *name) : m_room(NULL), m_width( w), m_height( h), m_frame_when_rendered( 0), m_name(name) {}

	private:
		PlanarMap(const PlanarMap&);
		PlanarMap& operator=(const PlanarMap&);
	};

}

#endif //__XX__PLANARMAP_H__
