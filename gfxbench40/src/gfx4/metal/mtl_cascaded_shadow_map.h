/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_CASCADED_SHADOW_MAP_H
#define MTL_CASCADED_SHADOW_MAP_H

#include <kcl_math3d.h>
#include <kcl_camera2.h>

#include "mtl_material4.h"

#include <vector>

namespace MetalRender
{

#define CASCADE_COUNT 4

// Stable paralell split cascaded shadows
class CascadedShadowMap
{
public:
    enum FitMode
    {
        FIT_SPHERE = 0,
        FIT_OBB = 1,
    };

    static const bool m_use_map_based_selection = true;

	CascadedShadowMap(id<MTLDevice> device, KCL::uint32 map_size);
    ~CascadedShadowMap();

    void Init(KCL::uint32 width, KCL::uint32 height, MTLPixelFormat depth_component_format);
    void BuildFrustums(const KCL::Vector3D &light_dir, const KCL::Camera2 *active_camera);
    void FrustumCull(KCL::uint32 split, KRL_Scene * scene, std::vector<KCL::Mesh*> visible_meshes[3], std::vector<KCL::MeshInstanceOwner2*> &visible_mios, bool force_cast_shadows);

	id<MTLRenderCommandEncoder> GetRenderPassEncoder(id<MTLCommandBuffer> command_buffer, KCL::uint32 split);
    std::function<id<MTLRenderCommandEncoder>()> GetRenderPassCreator(id<MTLCommandBuffer> command_buffer, KCL::uint32 split);
    
    void BindTexture(KCL::uint32 texture_slot);

    KCL::Camera2 * GetCamera(KCL::uint32 split)
    {
        return &m_frustums[split].camera;
    }
    const KCL::Matrix4x4 * GetShadowMatrices() const
    {
        return m_shadow_matrices;
    }
    const KCL::Vector4D & GetFrustumDistances() const
    {
        return m_frustum_distances;
    }
    id<MTLTexture> GetTexture() const
    {
        return m_texture_array;
    }
    id<MTLSamplerState> GetSampler() const
    {
        return m_shadow_sampler;
    }
    id<MTLSamplerState> GetShadowSampler() const
    {
        return m_shadow_sampler;
    }
    id<MTLSamplerState> GetDepthSampler() const
    {
        return m_depth_sampler;
    }
    const float * GetPoissonDisk() const
    {
        return m_poisson_disk[0].v;
    }
    KCL::uint32 GetPoissonDiskSize() const
    {
        return m_poisson_disk.size();
    }

    void SetFitMode(FitMode mode)
    {
        m_fit_mode = mode;
    }
    FitMode GetFitMode() const
    {
        return m_fit_mode;
    }
    MTLPixelFormat GetDepthComponentFormat() const
    {
        return m_depth_component_format;
    }

    inline KCL::uint32 GetTextureMapSize()
    {
        return m_map_size;
    }
    
private:
    struct Frustum
    {
        // Z split planes (-view space)
        float near_distance;
        float far_distance;

        // Points of the frustum split
        KCL::Vector3D points[8];

        // View space AABB
        KCL::Vector3D view_space_aabb_min;
        KCL::Vector3D view_space_aabb_max;

        // Light space AABB
        KCL::Vector3D light_space_aabb_min;
        KCL::Vector3D light_space_aabb_max;

        KCL::Vector3D world_space_obb[8];
        KCL::Vector4D cull_planes[8];

        // Bounding sphere
        float radius;
        KCL::Vector3D target;

        // Shadow camera
        KCL::Camera2 camera;

        //  Debugging color
        KCL::Vector4D color;
    };

    void SetCascade(KCL::uint32 index, float near_distance, const KCL::Vector3D &color);
    void Update();
    void SplitFrustumsLogaritmic();
    static void ExpandBoundingBox(KCL::Vector3D &min, KCL::Vector3D &max, const KCL::Vector3D &p);
    static KCL::Vector4D CreatePlane(const KCL::Vector3D &a, const KCL::Vector3D &b);
    static KCL::Vector4D CreatePlane(const KCL::Vector3D &a, const KCL::Vector3D &b, const KCL::Vector3D &c);
    static void CreateCullPlanes(const KCL::Vector3D &min, const KCL::Vector3D &max, const KCL::Matrix4x4 &matrix, KCL::Vector4D *planes);

    void CreatePoissonDisk();
    static bool PoissonOffsetCompare(const KCL::Vector2D &A, const KCL::Vector2D &B);

	id<MTLDevice> m_device;

    FitMode m_fit_mode;

    float m_default_shadow_range;

    const KCL::uint32 m_map_size;

    float m_mesh_cull_size[CASCADE_COUNT];

    KCL::Matrix4x4 m_bias_matrix;

    Frustum m_frustums[CASCADE_COUNT];
    KCL::Vector3D m_view_frustum_points[8];

    KCL::Vector3D m_light_dir;

    KCL::uint32 m_viewport_width;
    KCL::uint32 m_viewport_height;
    const KCL::Camera2 *m_camera;

    KCL::Vector4D m_frustum_distances;
    KCL::Matrix4x4 m_shadow_matrices[4];

    MTLPixelFormat m_depth_component_format;
    KCL::uint32 m_fbos[CASCADE_COUNT];
    id<MTLTexture> m_texture_array;

	id<MTLSamplerState> m_shadow_sampler;
    id<MTLSamplerState> m_depth_sampler;

    std::vector<KCL::Vector2D> m_poisson_disk;

    KCL::uint32 m_frustum_cull_counter;
    KCL::uint32 m_frame_counter;
	
	MTLViewport m_viewport;
};
}

#endif