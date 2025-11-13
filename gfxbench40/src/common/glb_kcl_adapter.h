/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_KCL_ADAPTER_H
#define GLB_KCL_ADAPTER_H

#include <kcl_base.h>

namespace GLB
{
	typedef KCL::uint64 uint64;
	typedef KCL::uint32 uint32;
	typedef KCL::uint16 uint16;
	typedef KCL::uint8 uint8;
	typedef KCL::int64 int64;
	typedef KCL::int32 int32;
	typedef KCL::int16 int16;
	typedef KCL::int8 int8;
}

#include <kcl_math3d.h>
namespace GLB
{
	typedef KCL::Vector2D Vector2D;
	typedef KCL::Vector3D Vector3D;
	typedef KCL::Vector4D Vector4D;
	typedef KCL::Matrix4x4 Matrix4x4;

	namespace Math = KCL::Math;
}

/*#include <kcl_os.h>
namespace GLB
{
	KCL::OS *&g_os = KCL::g_os;
}
*/

//#include <kcl_camera2.h>
namespace GLB
{
	typedef KCL::Camera2 Camera2;
}

// kcl_mesh
typedef KCL::Mesh Mesh;

// kcl_node
typedef KCL::Node Node;

// kcl_object
typedef KCL::Object Object;

// kcl_aab
typedef KCL::AABB AABB;

//kcl_room
typedef KCL::XRoom XRoom;
typedef KCL::XPortal XPortal;

//kcl_light
typedef KCL::Light Light;


//kcl_actor
typedef KCL::Actor Actor;

//kcl_animation4
typedef KCL::_key_node _key_node;

#endif
