/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MESH_TOOL_H
#define MESH_TOOL_H

#include <kcl_math3d.h>
#include <kcl_camera2.h>
#include <vector>
#include <map>

class KCL_Scene;

namespace KCL
{
	class Light;

	struct PickMeshArg
	{
		std::vector<KCL::Vector3D> grid_positions;

		std::vector<KCL::Mesh*> meshes;
		std::vector<KCL::Light*> lights;
		std::vector<KCL::Node*> emitters;
		std::vector< std::vector<KCL::Mesh*> > mesh_instance;
		KCL::Camera2 *camera;
		KCL::Vector2D viewport;
		KCL::Vector2D pick_pos;
	};

	class MeshTool
	{
	public:
		static KCL::Node* PickLightOrEmitter(PickMeshArg &pick_arg);
		static KCL::Mesh* PickMesh(PickMeshArg &pick_arg);
		static int PickByPositions(KCL::PickMeshArg pick_arg);

	private:

		static KCL::Vector3D CalculateRayDirection(KCL::Camera2 *camera, float viewport_width, float viewport_height, float x, float y);
		static bool IntersectMesh(const KCL::Vector3D &ray_from, const KCL::Vector3D &ray_to, const KCL::Vector3D &ray_dir, KCL::Mesh *mesh, float &distance);
		static bool calcQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1);

		static bool PickSphere(KCL::Vector3D &ray_from, KCL::Vector3D &ray_dir, KCL::Vector3D &center, float radius2, float *distance = nullptr);
	};
}

#endif
