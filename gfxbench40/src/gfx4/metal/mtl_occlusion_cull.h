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

#include <Metal/Metal.h>

class MTL_Scene_40;
class WarmUpHelper;

namespace MetalRender
{
	class QuadBuffer;
    class Pipeline;
    class DynamicDataBuffer;
    class DynamicDataBufferPool;
    
    class OcclusionCull
    {
    private:
        static const bool USE_MATRIX_STREAM = true;

    public:
        OcclusionCull(id<MTLDevice> device);
        ~OcclusionCull();

		KCL::KCL_Status Init(id<MTLDevice> device, DynamicDataBufferPool* pool, WarmUpHelper *warm_up, MTL_Scene_40 *scene, KCL::uint32 width, KCL::uint32 height, MTLPixelFormat depth_format, QuadBuffer * quad_buffer, KCL::uint32 max_instance_count, bool cull_non_instanced_meshes, bool enable_draw_counters);
		void CreateHiZ(id<MTLCommandBuffer> command_buffer, const KCL::Camera2 *camera, const std::vector<KCL::Mesh*> &occluders, MetalRender::QuadBuffer * quad_buffer);
        void ExecuteOcclusionCull(id<MTLCommandBuffer> command_buffer, const KCL::Camera2 *camera, const std::vector<KCL::Mesh*> &meshes, const std::vector< std::vector<KCL::Mesh*> > &visible_instances);

        id<MTLTexture> GetHiZDepthTexture() const
        {
            return m_depth_texture;
        }

        id<MTLSamplerState> GetHiZDepthSampler() const
        {
            return m_depth_sampler;
        }

        id<MTLBuffer> GetIndirectCommandBuffer() const
        {
            return m_indirect_draw_buffer;
        }

        id<MTLBuffer> GetInstanceBuffer() const
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
        void UpdateMeshData(const MTL_Scene_40 *scene);
        KCL::KCL_Status LoadShaders(KCL::uint32 ws_size);

        KCL::KCL_Status Warmup(WarmUpHelper *warm_up, DynamicDataBufferPool* pool);

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

        KCL::uint32 m_depth_levels;
        id<MTLTexture> m_depth_texture;
        id<MTLSamplerState> m_depth_sampler;
		id<MTLDepthStencilState> m_occluder_depth_state;
		id<MTLDepthStencilState> m_hiz_depth_state;
		MTLRenderPassDescriptor * m_occluder_pass_desc;
		MTLRenderPassDescriptor * m_hiz_pass_desc;

		Pipeline * m_occlusion_cull_shader;
        KCL::int32 m_id_count_pos;
        KCL::int32 m_instance_id_offset_pos;

        Pipeline * m_hi_z_shader;
        KCL::int32 m_texture_size_pos;

        KCL::uint32 m_work_group_size;
        Pipeline * m_occluder_shader;

        id<MTLBuffer> m_mesh_data_buffer;
        id<MTLBuffer> m_matrix_data_buffer;
        id<MTLBuffer> m_indirect_draw_buffer;
		id<MTLBuffer> m_original_draw_buffer;		
        id<MTLBuffer> m_mesh_id_buffer;
        id<MTLBuffer> m_draw_counter_buffer;

        KCL::uint32 m_dispatch_count;

        KCL::uint32 m_instanced_draw_commands_offset;

        // Instancing
        std::vector<MTLDrawIndexedPrimitivesIndirectArguments> m_instanced_indirect_draw_data;
        std::vector<KCL::int32> m_stream_offsets;
        id<MTLBuffer> m_instanced_matrices_stream_buffer;
        id<MTLBuffer> m_instanced_matrices_buffer;

        std::vector<KCL::uint32> m_mesh_ids;
        std::vector<BoundingVolume> m_actors;

        float m_frame_counter;
		Pipeline * m_stream_compact_shader;

        MTL_Scene_40 * m_scene;
		id<MTLDevice> m_device;

		QuadBuffer * m_quad_buffer;
        DynamicDataBuffer* m_dynamic_data;
		DynamicDataBufferPool *m_dynamic_data_pool;
    };
}

#endif
