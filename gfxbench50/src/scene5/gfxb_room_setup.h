/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_ROOM_SETUP_H
#define GFXB_ROOM_SETUP_H

#include <vector>
#include <string>
#include <kcl_math3d.h>

namespace KCL
{
	class Mesh3;
	class Mesh;
	class XRoom;
};

namespace GFXB
{
	class Scene5;

	class RoomSetup
	{
	public:
		static void SetupRooms(Scene5 *scene);

		static void SetupPVS(Scene5 *scene);

		static void PlaceProbes(Scene5 *scene);

		static void CleanUp(Scene5 *scene);

	private:
		static KCL::Mesh *FindConnection(const std::vector<KCL::Mesh*> &partition_meshes, const KCL::Mesh *mesh);

		static void GetVertices(const KCL::Mesh3 *mesh3, std::vector<KCL::Vector3D> &vertices);

		static void ResetPVS(Scene5 *scene);

		static void ClearPVS(Scene5 *scene, bool clear_value);

		static KCL::XRoom *FindRoom(Scene5 *scene, const std::string &name);
	};
}

#endif