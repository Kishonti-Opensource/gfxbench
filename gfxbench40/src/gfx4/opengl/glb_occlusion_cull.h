/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef OCCLUSION_CULL_H
#define OCCLUSION_CULL_H

#include <kcl_base.h>
#include <kcl_math3d.h>

#include <vector>

class GLB_Scene4;
class WarmUpHelper;
namespace GLB
{
    class GLBShader2;
    class OcclusionCull
    {
    private:
        static const bool USE_MATRIX_STREAM = true;

    public:
        struct DrawElementsIndirectCommand
        {
            KCL::uint32 count;
            KCL::uint32 instanceCount;
            KCL::uint32 firstIndex;
            KCL::int32 baseVertex;
            KCL::uint32 reservedMustBeZero;

            DrawElementsIndirectCommand()
            {
                count = 0;
                instanceCount = 0;
                firstIndex = 0;
                baseVertex = 0;
                reservedMustBeZero = 0;
            }
        };

        OcclusionCull();
        ~OcclusionCull();

        KCL::KCL_Status Init(WarmUpHelper *warm_up, GLB_Scene4 *scene, KCL::uint32 quad_vao, KCL::uint32 width, KCL::uint32 height, KCL::uint32 gl_depth_format, KCL::uint32 max_instance_count, bool cull_non_instanced_meshes, bool enable_draw_counters);
        void CreateHiZ(const KCL::Camera2 *camera, const std::vector<KCL::Mesh*> &occluders);
        void ExecuteOcclusionCull(const KCL::Camera2 *camera, const std::vector<KCL::Mesh*> &meshes, const std::vector< std::vector<KCL::Mesh*> > &visible_instances);

        KCL::uint32 GetHiZDepthTexture() const
        {
            return m_depth_texture;
        }

        KCL::uint32 GetHiZDepthSampler() const
        {
            return m_depth_sampler;
        }

        KCL::uint32 GetIndirectCommandBuffer() const
        {
            return m_indirect_draw_buffer;
        }

        KCL::uint32 GetInstanceBuffer() const
        {
            return m_instanced_matrices_buffer;
        }

        KCL::int32 GetInstanceBufferOffset() const
        {
            return -KCL::int32(m_mesh_count);
        }

        KCL::uint32 GetDrawCount() const
        {
            return m_draw_count;
        }

        void DebugHiZMap(KCL::Camera2 *camera);

 private:
        void UpdateMeshData(const KCL::SceneHandler *scene);
        KCL::KCL_Status LoadShaders(KCL::uint32 ws_size);

        KCL::KCL_Status Warmup(WarmUpHelper *warm_up);

        struct BoundingVolume
        {
            KCL::Vector4D aabb[8];
            // x - index count
            // y - indirect draw buffer index
            // z - indirect draw buffer index for instanced meshes
            // w - index of the instance within the instance group
            KCL::Vector4D draw_data;
        };

        bool m_is_draw_counter_enabled;
        bool m_cull_non_instanced_meshes;

        KCL::uint32 m_width;
        KCL::uint32 m_height;
        KCL::uint32 m_mesh_count;
        KCL::uint32 m_static_mesh_count;
        KCL::uint32 m_actor_data_offset;
        KCL::uint32 m_max_instance_count;
        KCL::uint32 m_draw_count;

        KCL::uint32 m_quad_vao;
        KCL::uint32 *m_fbos;

        KCL::uint32 m_depth_levels;
        KCL::uint32 m_depth_texture;
        KCL::uint32 m_depth_sampler;

        GLBShader2 *m_occlusion_cull_shader;
        KCL::int32 m_id_count_pos;
        KCL::int32 m_instance_id_offset_pos;

        GLBShader2 *m_hi_z_shader;
        KCL::int32 m_texture_size_pos;

        KCL::uint32 m_work_group_size;
        GLBShader2 *m_occluder_shader;

        KCL::uint32 m_mesh_buffer;
        KCL::uint32 m_indirect_draw_buffer;
        KCL::uint32 m_mesh_id_buffer;
        KCL::uint32 m_draw_counter_buffer;

        KCL::uint32 m_dispatch_count;

        KCL::uint32 m_instanced_draw_commands_offset;

        // Instancing
        std::vector<DrawElementsIndirectCommand> m_instanced_indirect_draw_data;
        std::vector<KCL::int32> m_stream_offsets;
        KCL::uint32 m_instanced_matrices_stream_buffer;
        KCL::uint32 m_instanced_matrices_buffer;

        std::vector<KCL::uint32> m_mesh_ids;
        std::vector<BoundingVolume> m_actors;

        float m_frame_counter;
        KCL::int32 m_frame_counter_pos1; // Pos in occlusion cull shader
        KCL::int32 m_frame_counter_pos2; // Pos in stream compaction shader
        KCL::int32 m_stream_offsets_pos;
        GLBShader2 *m_stream_compact_shader;

        // Debug
        GLB::GLBShader2 *m_debug_shader;

        GLB_Scene4 *m_scene;
    };
}

#endif