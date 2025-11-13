/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_SCENE_40_SUPPORT_H
#define MTL_SCENE_40_SUPPORT_H

#include <kcl_math3d.h>
#include <kcl_camera2.h>

#include "mtl_mesh4.h"

#include <sstream>
#include <vector>
#include <map>

class MTL_Scene_Base;
class MTL_Scene_40;

class MTLTexture;
class MTLDevice;

namespace MTLRenderer
{
    class Material4;
}

namespace GFXB4
{
	id<MTLTexture> CreateRenderTarget(id<MTLDevice> device, KCL::uint32 width, KCL::uint32 height, MTLPixelFormat format);
}

class MTL_Scene4Tools
{
public:
    static void SortMeshes(std::vector<KCL::Mesh*> &visible_meshes, const KCL::Camera2 *camera, bool alpha_blend);
    static KCL::Mesh* PickMesh(MTL_Scene_Base *scene, KCL::Camera2 *camera, float x, float y);

private:
    struct mesh_sort_info
    {
        KCL::Mesh *mesh;
		KCL::int32 sort_order;
        float vertexCenterDist;
    };

    // Front to back
    static bool depth_compare(const mesh_sort_info* A, const mesh_sort_info* B);

    // Back to front
    static bool reverse_depth_compare(const mesh_sort_info* A, const mesh_sort_info* B);

    // Material sort
    static bool material_compare(const mesh_sort_info* A, const mesh_sort_info* B);

    // Material sort
    static bool reverse_material_compare(const mesh_sort_info* A, const mesh_sort_info* B);

    // Material + depth sort (front to back)
    static bool material_depth_compare(const mesh_sort_info* A, const mesh_sort_info* B);    
   
    // RC2 sort function
    static bool displacement_depth_compare(const mesh_sort_info* A, const mesh_sort_info* B);

    static std::vector<mesh_sort_info*> sort_pointers;
    static std::vector<mesh_sort_info> sorted_meshes;

    static KCL::Vector3D CalculateRayDirection(KCL::Camera2 *camera, float viewport_width, float viewport_height, float x, float y);
    static bool IntersectMesh(const KCL::Vector3D &ray_from, const KCL::Vector3D &ray_to, const KCL::Vector3D &ray_dir, KCL::Mesh *mesh, float &distance);
};

class ParaboloidCulling : public KCL::CullingAlgorithm
{
public:
	ParaboloidCulling(KCL::uint32 size_x, KCL::uint32 size_y, float limit);
	virtual ~ParaboloidCulling() {}

	void SetLimit(float limit);
	virtual bool CullMesh(KCL::Camera2 *camera, KCL::Mesh *mesh);

private:
	float m_limit;
	float m_size_x;
	float m_size_y;
};

class PerspectiveCulling : public KCL::CullingAlgorithm
{
public:
	PerspectiveCulling(float limit);
	virtual ~PerspectiveCulling() {}
	virtual bool CullMesh(KCL::Camera2 *camera, KCL::Mesh *mesh);

private:
	float m_limit;
};

class WorkGroupValidator
{
public:
	struct WGConfig
	{
		WGConfig(KCL::uint32 size_x = 1, KCL::uint32 size_y = 1, KCL::uint32 size_z = 1);
		~WGConfig();
		void Set(KCL::uint32 size_x, KCL::uint32 size_y = 1, KCL::uint32 size_z = 1);

		KCL::uint32 size_x;
		KCL::uint32 size_y;
		KCL::uint32 size_z;
	};

	WorkGroupValidator(id<MTLDevice> device);
	~WorkGroupValidator();

	bool Validate(const WGConfig &config) const;
	bool Validate(const std::vector<WGConfig> &configs, std::vector<WorkGroupValidator::WGConfig> &result) const;
	bool Validate(KCL::uint32 size_x, KCL::uint32 size_y = 1, KCL::uint32 size_z = 1) const;

	KCL::uint32 m_max_ws_size_x;
	KCL::uint32 m_max_ws_size_y;
	KCL::uint32 m_max_ws_size_z;

	KCL::uint32 m_max_invocations;

	KCL::uint32 m_max_shared_memory;
};

class WarmUpHelper
{
public:
	static const std::string OCCLUSION_CULL;
	static const std::string MOTION_BLUR_TILE_MAX;
	static const std::string MOTION_BLUR_NEIGHBOR_MAX;
	static const std::string PARTICLE_SYSTEM_EMIT;
	static const std::string PARTICLE_SYSTEM_SIMULATE;

	static const bool s_use_timer_query = false;

	class WarmUpConfig
	{
	public:
		WarmUpConfig() { }
		virtual ~WarmUpConfig() {}

		WorkGroupValidator::WGConfig m_wg_config;
	};

	WarmUpHelper(id<MTLDevice> device, MTL_Scene_40 *scene, const std::string &wg_sizes);
	~WarmUpHelper();

	WarmUpConfig *GetConfig(const std::string &name);
	void SetConfig(const std::string &name, WarmUpConfig *config);

	void BeginTimer();
	KCL::uint64 EndTimer();

	MTL_Scene_40 *GetScene() const { return m_scene; }
	const WorkGroupValidator *GetValidator() const { return &m_validator; }

private:
	std::map<std::string, WarmUpConfig*> m_configs;
	KCL::uint64 m_start_time;

	MTL_Scene_40 *m_scene;

	WorkGroupValidator m_validator;
};


template<typename T, typename W>
std::string PARTICLE_BUFFERS_FILENAME(T anim_time, W name)
{
    std::stringstream filename_stream;
    filename_stream << "particle_buffers_" << anim_time << "ms_"<<name;
    return filename_stream.str();
}


#endif
