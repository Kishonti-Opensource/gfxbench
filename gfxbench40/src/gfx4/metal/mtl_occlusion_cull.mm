/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_occlusion_cull.h"
#include "platform.h"

#include <kcl_camera2.h>
#include <kcl_mesh.h>
#include <kcl_scene_handler.h>

#include "mtl_scene_40.h"
#include "mtl_scene_40_support.h"
#include "metal/mtl_pipeline_builder.h"
#include "metal/mtl_dynamic_data_buffer.h"
#include "mtl_quadBuffer.h"


#include "mtl_mesh4.h"

#include <ng/log.h>


#define COMPATIBLE_HI_Z_MIPMAP_GEN 1
#define COMPATIBLE_OCCLUDERS 1


using namespace MetalRender;

OcclusionCull::OcclusionCull(id<MTLDevice> device)
	: m_device(device)
{
    m_is_draw_counter_enabled = false;

    m_width = 0;
    m_height = 0;
    m_mesh_count = 0;
    m_static_mesh_count = 0;
    m_actor_data_offset = 0;
    m_max_instance_count = 0;
    m_draw_count = 0;

    m_depth_levels = 0;
    m_depth_texture = 0;
    m_depth_sampler = 0;

    m_work_group_size = 0;
	m_occluder_shader = NULL;

	m_hi_z_shader = NULL;

	m_stream_compact_shader = NULL;

    m_frame_counter = 100.0f;

    m_mesh_id_buffer = nil;
    m_mesh_data_buffer = nil;
    m_matrix_data_buffer = nil;
    m_indirect_draw_buffer = nil;
    m_draw_counter_buffer = nil;
    m_instanced_matrices_buffer = nil;
    m_instanced_matrices_stream_buffer = nil;

    m_instanced_draw_commands_offset = 0;

    m_dispatch_count = 0;
}

OcclusionCull::~OcclusionCull()
{
}

KCL::KCL_Status OcclusionCull::Init(id<MTLDevice> device, DynamicDataBufferPool* pool, WarmUpHelper *warm_up, MTL_Scene_40 *scene, KCL::uint32 width, KCL::uint32 height, MTLPixelFormat depth_format, QuadBuffer * quad_buffer, KCL::uint32 max_instance_count, bool cull_non_instanced_meshes, bool enable_draw_counters)
{
    m_scene = scene;
    m_width = KCL::Max(width, 1u);
    m_height = KCL::Max(height, 1u);
    m_max_instance_count = max_instance_count;
	m_is_draw_counter_enabled = enable_draw_counters;
    m_cull_non_instanced_meshes = cull_non_instanced_meshes;
	m_quad_buffer = quad_buffer;
    
    m_dynamic_data = pool->GetNewBuffer(3 * 1024 * 1024);
	m_dynamic_data_pool = pool;

    UpdateMeshData(scene);

    if (m_is_draw_counter_enabled)
    {
        m_draw_count = 0;
		m_draw_counter_buffer = [m_device newBufferWithBytes:&m_draw_count length:sizeof(KCL::uint32) options:STORAGE_MODE_MANAGED_OR_SHARED];
    }

    m_depth_levels = KCL::texture_levels(m_width, m_height);
	MTLTextureDescriptor * tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:depth_format
																						 width:m_width
																						height:m_height
																					 mipmapped:YES];
	tex_desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_EMBEDDED
	tex_desc.storageMode = MTLStorageModePrivate;
#endif

	m_depth_texture = [device newTextureWithDescriptor:tex_desc];

	// Create the sampler
	MTLSamplerDescriptor * sampler_desc = [[MTLSamplerDescriptor alloc] init];

	sampler_desc.sAddressMode = MTLSamplerAddressModeClampToEdge;
	sampler_desc.tAddressMode = MTLSamplerAddressModeClampToEdge;
	sampler_desc.minFilter = MTLSamplerMinMagFilterNearest;
	sampler_desc.magFilter = MTLSamplerMinMagFilterNearest;
	sampler_desc.mipFilter = MTLSamplerMipFilterNearest;

	m_depth_sampler = [device newSamplerStateWithDescriptor:sampler_desc];
	releaseObj(sampler_desc);


	m_occluder_pass_desc = [[MTLRenderPassDescriptor alloc] init];
	m_occluder_pass_desc.depthAttachment.texture = m_depth_texture;
	m_occluder_pass_desc.depthAttachment.clearDepth = 1.0f;
	m_occluder_pass_desc.depthAttachment.loadAction = MTLLoadActionClear;
	m_occluder_pass_desc.depthAttachment.storeAction = MTLStoreActionStore;

	m_hiz_pass_desc = [[MTLRenderPassDescriptor alloc] init];
	m_hiz_pass_desc.depthAttachment.texture = m_depth_texture;
	m_hiz_pass_desc.depthAttachment.loadAction = MTLLoadActionDontCare;
	m_hiz_pass_desc.depthAttachment.storeAction = MTLStoreActionStore;

	MTLDepthStencilDescriptor * depth_desc = [[MTLDepthStencilDescriptor alloc] init];

	depth_desc.depthWriteEnabled = YES;
	depth_desc.depthCompareFunction = MTLCompareFunctionLess;

	m_occluder_depth_state = [device newDepthStencilStateWithDescriptor:depth_desc];


	depth_desc.depthCompareFunction = MTLCompareFunctionAlways;

	m_hiz_depth_state = [device newDepthStencilStateWithDescriptor:depth_desc];
	releaseObj(depth_desc);

	// Load the shaders
    KCL::uint32 ws_size = 64;

    if (warm_up)
    {
        WarmUpHelper::WarmUpConfig *cfg = warm_up->GetConfig(WarmUpHelper::OCCLUSION_CULL);
        if (cfg)
        {
            NGLOG_INFO("OcclusionCull: using pre-defined workgroup size: %s", cfg->m_wg_config.size_x);
            ws_size = cfg->m_wg_config.size_x;
        }
        else
        {
            return Warmup(warm_up, pool);
        }
    }

    return LoadShaders(ws_size);
}

KCL::KCL_Status OcclusionCull::LoadShaders(KCL::uint32 ws_size)
{
    m_work_group_size = ws_size;

    KCL::KCL_Status build_result;
    MTLPipeLineBuilder sb;

    m_occluder_shader = sb
	.ShaderFile("occluder.shader")
	.HasDepth(true)
	.SetType(kShaderTypeNoColorAttachment)
	.SetVertexLayout(GFXB4::Mesh3::GetVertexDescriptor())
	.AddDefineInt("COMPATIBLE_OCCLUDERS", COMPATIBLE_OCCLUDERS)
	.Build(build_result);

    if (build_result != KCL::KCL_TESTERROR_NOERROR)
    {
        return build_result;
    }

	m_hi_z_shader = sb
	.ShaderFile("hi_z.shader")
	.AddDefineInt("COMPATIBLE_HI_Z_MIPMAP_GEN",COMPATIBLE_HI_Z_MIPMAP_GEN)
	.HasDepth(true)
	.SetType(kShaderTypeNoColorAttachment)
	.SetVertexLayout(MetalRender::QuadBuffer::GetVertexLayout())
	.Build(build_result);

    if (build_result != KCL::KCL_TESTERROR_NOERROR)
    {
        return build_result;
    }

    sb.ShaderFile("occlusion_cull.shader");
    sb.AddDefineInt("WORK_GROUP_SIZE", m_work_group_size);
    sb.AddDefineInt("STATIC_MESH_COUNT", m_static_mesh_count);
    sb.AddDefineInt("MESH_COUNT", m_mesh_count);
    sb.AddDefineInt("USE_MATRIX_STREAM", USE_MATRIX_STREAM ? 1 : 0);
	sb.ForceHighp(true);

    if (m_is_draw_counter_enabled)
    {
        sb.AddDefine("DRAW_COUNTER_ENABLED");
    }

    m_occlusion_cull_shader = sb.Build(build_result);

    if (build_result != KCL::KCL_TESTERROR_NOERROR)
    {
        return build_result;
    }

    sb.ShaderFile("stream_compact.shader");
    sb.AddDefineInt("MAX_MIO_COUNT", m_scene->m_mios2.size());
	sb.ForceHighp(true);
    m_stream_compact_shader = sb.Build(build_result);
    if (build_result != KCL::KCL_TESTERROR_NOERROR)
    {
        return build_result;
    }

    return KCL::KCL_TESTERROR_NOERROR;
}

void OcclusionCull::CreateHiZ(id<MTLCommandBuffer> command_buffer, const KCL::Camera2 *camera, const std::vector<KCL::Mesh*> &occluders, MetalRender::QuadBuffer * quad_buffer)
{
	id <MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:m_occluder_pass_desc];
	encoder.label = @"Render Occluders";

    const size_t mesh_count = occluders.size();
    if (mesh_count)
    {
        
		[encoder setDepthStencilState:m_occluder_depth_state];
		[encoder setFrontFacingWinding:MTLWindingCounterClockwise];
		[encoder setCullMode:MTLCullModeNone];

		m_occluder_shader->Set(encoder);

        for (KCL::uint32 i = 0; i < mesh_count; i++)
        {
            KCL::Mesh * mesh = occluders[i];
            
            assert(mesh->m_material->m_is_tesselated == false);

            GFXB4::Mesh3 * mtl_mesh = (GFXB4::Mesh3*)mesh->m_mesh;
			
#if !COMPATIBLE_OCCLUDERS
			KCL::Matrix4x4 mvp = mesh->m_world_pom * camera->GetViewProjection();
#else
			KCL::Matrix4x4 proj;
			KCL::Matrix4x4::PerspectiveGL(proj, camera->GetFov(), camera->GetAspectRatio(), camera->GetNear(),camera->GetFar());
			KCL::Matrix4x4 vp = camera->GetView() * proj;
			KCL::Matrix4x4 mvp = mesh->m_world_pom * vp;
#endif

			[encoder setVertexBuffer:mtl_mesh->GetVertexBuffer() offset:0 atIndex:0];
			[encoder setVertexBytes:&mvp length:sizeof(KCL::Matrix4x4) atIndex:1];

			[encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
								indexCount:KCL::uint32(mtl_mesh->getIndexCount(0))
									indexType:MTLIndexTypeUInt16
							   indexBuffer:mtl_mesh->GetIndexBuffer(0)
						 indexBufferOffset:(NSUInteger)mtl_mesh->m_ebo[0].m_offset];
        }
    }

	[encoder endEncoding];


    // Generate the Hierarchical-Z map
    KCL::uint32 width = m_width;
    KCL::uint32 height = m_height;

    for (KCL::uint32 i = 1; i < m_depth_levels; i++)
    {
		m_hiz_pass_desc.depthAttachment.level = i;

		id <MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:m_hiz_pass_desc];
		encoder.label = [NSString stringWithFormat:@"Create Hi-Z level %u", i];

		[encoder setDepthStencilState:m_hiz_depth_state];
		[encoder setFrontFacingWinding:MTLWindingCounterClockwise];

		m_hi_z_shader->Set(encoder);

		// Set the size of the source texture
		[encoder setFragmentTexture:m_depth_texture atIndex:0];
		[encoder setFragmentSamplerState:m_depth_sampler lodMinClamp:i-1 lodMaxClamp:i-1 atIndex:0];

        // Calculate the render target dimenensions
        width = KCL::Max(width / 2, 1u);
        height = KCL::Max(height / 2, 1u);
		
		if (COMPATIBLE_HI_Z_MIPMAP_GEN)
		{
			KCL::Vector2D inv_size = KCL::Vector2D(1.0f/float(width), 1.0f/float(height));
			[encoder setFragmentBytes:inv_size.v length:sizeof(KCL::Vector2D) atIndex:0];
		}

		MTLViewport viewport = { 0.0, 0.0, (double)width, (double)height, 0.0, 1.0 };
		[encoder setViewport:viewport];

		quad_buffer->Draw(encoder);
		[encoder endEncoding];
    }
}

void OcclusionCull::ExecuteOcclusionCull(id<MTLCommandBuffer> command_buffer, const KCL::Camera2 *camera, const std::vector<KCL::Mesh*> &meshes, const std::vector< std::vector<KCL::Mesh*> > &visible_instances)
{
    KCL::uint32 id_count = 0;
    KCL::uint32 actor_mesh_count = 0;

    // Collect the meshes for culling
    KCL::Mesh *kcl_mesh = NULL;
    GFXB4::Mesh3 *mtl_mesh = NULL;
    if (m_cull_non_instanced_meshes)
    {
        for (KCL::uint32 i = 0; i < meshes.size(); i++)
        {
            kcl_mesh = meshes[i];
            if (kcl_mesh->m_indirect_draw_id < 0)
            {
                continue;
            }
            
            if (kcl_mesh->m_owner && kcl_mesh->m_owner->m_type == KCL::ACTOR)
            {
                // Update the bounding volume for the actors
                mtl_mesh = (GFXB4::Mesh3*)kcl_mesh->m_mesh;
                kcl_mesh->m_owner_actor->m_aabb.CalculateVertices4D(m_actors[actor_mesh_count].aabb);
                m_actors[actor_mesh_count].draw_data.x = float(mtl_mesh->getIndexCount(0));
                m_actors[actor_mesh_count].draw_data.y = kcl_mesh->m_indirect_draw_id;

                m_mesh_ids[id_count++] = m_static_mesh_count + actor_mesh_count;
                actor_mesh_count++;
            }
            else
            {
                // Static mesh
                m_mesh_ids[id_count++] = kcl_mesh->m_indirect_draw_id;
            }
        }
    }

    // Collect the instanced meshes
    KCL::uint32 visible_mios = 0;
    KCL::uint32 instance_id_offset = id_count;
    for (KCL::uint32 i = 0; i < visible_instances.size(); i++)
    {
        if (visible_instances[i].size())
        {
            // We have max 128 instances, and every instance has 2 matrices, so we have to multiply by 256
            m_stream_offsets[visible_mios++] = (GetInstanceBufferOffset() + visible_instances[i][0]->m_mio2->m_indirect_draw_id ) * 256;
        }

        for (KCL::uint32 j = 0; j < visible_instances[i].size(); j++)
        {
            KCL::int32 draw_id = visible_instances[i][j]->m_indirect_draw_id;
            if (draw_id > -1)
            {
                m_mesh_ids[id_count++] = draw_id;
            }
        }
    }

    if (!id_count)
    {
        m_draw_count = 0;
        return;
    }

    m_frame_counter = m_frame_counter + 1.0f;

    // Update the id buffer
    m_mesh_id_buffer = m_dynamic_data->GetCurrentBuffer();
    auto mesh_id_offset = m_dynamic_data->WriteDataAndGetOffset(nil, m_mesh_ids.data(), id_count * sizeof(KCL::uint32));

	id<MTLBlitCommandEncoder> blitEncoder = [command_buffer blitCommandEncoder];
	blitEncoder.label = @"Update Occlusion Cull Data";

    // Update the dynamic mesh buffer (actors)
    if (actor_mesh_count)
    {
		auto actor_offset = m_dynamic_data->WriteDataAndGetOffset(nil, m_actors.data(), actor_mesh_count * sizeof(BoundingVolume));

		[blitEncoder copyFromBuffer:m_dynamic_data->GetCurrentBuffer()
					   sourceOffset:actor_offset
						   toBuffer:m_mesh_data_buffer
				  destinationOffset:m_actor_data_offset
							   size:actor_mesh_count * sizeof(BoundingVolume)];
    }

	[blitEncoder copyFromBuffer:m_original_draw_buffer
				   sourceOffset:m_instanced_draw_commands_offset
					   toBuffer:m_indirect_draw_buffer
			  destinationOffset:m_instanced_draw_commands_offset
						   size:m_instanced_indirect_draw_data.size() * sizeof(MTLDrawIndexedPrimitivesIndirectArguments)];

	[blitEncoder endEncoding];


    // Bind the buffers
	id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];
	encoder.label = @"Occlusion Cull";

    // Bind the hiZ depth texture
	[encoder setTexture:m_depth_texture atIndex:0];
	[encoder setSamplerState:m_depth_sampler atIndex:0];

	m_occlusion_cull_shader->SetAsCompute(encoder);

	OcclusionConstants consts;
	consts.vp = camera->GetViewProjection();
	consts.view_port_size = KCL::Vector2D(m_width,m_height);
	consts.near_far_ratio = -camera->GetProjection().v33;
	consts.id_count = id_count;
	consts.instance_id_offset = instance_id_offset;
	consts.frame_counter = m_frame_counter;

	auto consts_offs = m_dynamic_data->WriteDataAndGetOffset(nil, &consts, sizeof(consts));

	[encoder setBuffer:m_dynamic_data->GetCurrentBuffer() offset:consts_offs atIndex:0];
	[encoder setBuffer:m_mesh_data_buffer offset:0 atIndex:1];
	[encoder setBuffer:m_matrix_data_buffer offset:0 atIndex:2];
	[encoder setBuffer:m_mesh_id_buffer offset:mesh_id_offset atIndex:3];
	[encoder setBuffer:m_indirect_draw_buffer offset:0 atIndex:4];
	[encoder setBuffer:USE_MATRIX_STREAM ? m_instanced_matrices_stream_buffer : m_instanced_matrices_buffer offset:0 atIndex:5];

	if (m_is_draw_counter_enabled)
	{
		[encoder setBuffer:m_draw_counter_buffer offset:0 atIndex:6];
	}

    // Execute the occlusion cull
	MTLSize threadsPerGroup = { m_work_group_size, 1, 1 };
	MTLSize numThreadgroups = { (id_count + m_work_group_size - 1) / m_work_group_size, 1, 1};

	[encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];

    [encoder endEncoding];


    if (USE_MATRIX_STREAM && visible_mios)
    {
		id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];
		encoder.label = @"Stream Compaction";

		m_stream_compact_shader->SetAsCompute(encoder);
        
        auto stream_offs = m_dynamic_data->WriteDataAndGetOffset(nil, m_stream_offsets.data(), m_stream_offsets.size() * sizeof(m_stream_offsets[0]));

		[encoder setBuffer:m_dynamic_data->GetCurrentBuffer() offset:consts_offs atIndex:0];
		[encoder setBuffer:m_dynamic_data->GetCurrentBuffer() offset:stream_offs atIndex:1];
		[encoder setBuffer:m_instanced_matrices_stream_buffer offset:0 atIndex:2];
		[encoder setBuffer:m_instanced_matrices_buffer offset:0 atIndex:3];

		MTLSize threadsPerGroup = { MTL_Scene_40::MAX_INSTANCES, 1, 1 };
		MTLSize numThreadgroups = { visible_mios, 1, 1 };

		[encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];

		[encoder endEncoding];
    }
}

void OcclusionCull::UpdateMeshData(const MTL_Scene_40 *scene)
{
    std::vector<KCL::Matrix4x4> mesh_matrices;
    std::vector<BoundingVolume> bounding_volumes;
    std::vector<MTLDrawIndexedPrimitivesIndirectArguments> draw_commands;

    m_static_mesh_count = 0;
    KCL::uint32 id_counter = 0;

    for (KCL::uint32 i = 0; i < scene->m_rooms.size(); i++)
    {
        //m_static_mesh_count += scene->m_rooms[i]->m_meshes.size();
        for (KCL::uint32 j = 0; j < scene->m_rooms[i]->m_meshes.size(); j++)
        {
            KCL::Mesh *kcl_mesh = scene->m_rooms[i]->m_meshes[j];
            GFXB4::Mesh3 *glb_mesh3 = (GFXB4::Mesh3*)kcl_mesh->m_mesh;

            if (!m_cull_non_instanced_meshes && !kcl_mesh->m_mio2)
            {
                continue;
            }

            m_static_mesh_count++;

            // Set the model and the model matrix inverse
            KCL::Matrix4x4 model_matrix;
            if (kcl_mesh->m_material->m_is_billboard) //billboards shall only have the translation part
            {
                model_matrix.v41 = kcl_mesh->m_world_pom.v41; model_matrix.v42 = kcl_mesh->m_world_pom.v42; model_matrix.v43 = kcl_mesh->m_world_pom.v43;
            }
            else
            {
                model_matrix = kcl_mesh->m_world_pom;
            }

            KCL::Matrix4x4 inverse_model_matrix;
            KCL::Matrix4x4::InvertModelView(model_matrix, inverse_model_matrix);

            mesh_matrices.push_back(model_matrix);
            mesh_matrices.push_back(inverse_model_matrix);

            // Set the AABB
            BoundingVolume bounding_volume;
            kcl_mesh->m_aabb.CalculateVertices4D(bounding_volume.aabb);
            bounding_volume.draw_data.x = float(glb_mesh3->getIndexCount(0));
            bounding_volume.draw_data.y = id_counter;

            // Set the indirect draw command
            MTLDrawIndexedPrimitivesIndirectArguments draw_command;
            draw_command.indexCount = glb_mesh3->getIndexCount(0);
            draw_command.instanceCount = 1;
            draw_command.indexStart = *((KCL::uint32*)(&glb_mesh3->m_ebo[0].m_offset)) / 2; // Convert the byte offset to unsigned short index
			draw_command.baseVertex = 0;
			draw_command.baseInstance = 0;

            bounding_volumes.push_back(bounding_volume);
            draw_commands.push_back(draw_command);

            kcl_mesh->m_indirect_draw_id = id_counter;
            id_counter++;
        }
    }


    // Upload the actor meshes. We only upload the draw commands because the bound volume is updated render time.
    KCL::uint32 actor_mesh_count = 0;
    if (m_cull_non_instanced_meshes)
    {
        for (KCL::uint32 i = 0; i < scene->m_actors.size(); i++)
        {
            actor_mesh_count += scene->m_actors[i]->m_meshes.size();
            KCL::AABB &actor_aabb = scene->m_actors[i]->m_aabb;
            for (KCL::uint32 j = 0; j < scene->m_actors[i]->m_meshes.size(); j++)
            {
                KCL::Mesh *kcl_mesh = scene->m_actors[i]->m_meshes[j];
                GFXB4::Mesh3 *glb_mesh3 = (GFXB4::Mesh3*)kcl_mesh->m_mesh;

                MTLDrawIndexedPrimitivesIndirectArguments draw_command;
                draw_command.indexCount = glb_mesh3->getIndexCount(0);
                draw_command.instanceCount = 1;
                draw_command.indexStart = *((KCL::uint32*)(&glb_mesh3->m_ebo[0].m_offset)) / 2; // Convert the byte offset to unsigned short index
				draw_command.baseVertex = 0;
				draw_command.baseInstance = 0;

                draw_commands.push_back(draw_command);

                kcl_mesh->m_indirect_draw_id = id_counter;
                id_counter++;
            }
        }
    }

    m_mesh_count = m_static_mesh_count + actor_mesh_count;

    // Instancing
    KCL::uint32 mio_count = scene->m_mios2.size();
    m_instanced_indirect_draw_data.resize(mio_count);
    for (KCL::uint32 i = 0; i < mio_count; i++)
    {
        KCL::MeshInstanceOwner2 *mio = scene->m_mios2[i];
        KCL::Mesh3 *mesh3 = mio->m_mesh;

        mio->m_indirect_draw_id = id_counter++;
        MTLDrawIndexedPrimitivesIndirectArguments &draw_command = m_instanced_indirect_draw_data[i];
        draw_command.indexCount = mesh3->getIndexCount(0);
        draw_command.instanceCount = 0;
        draw_command.indexStart = *((KCL::uint32*)(&mesh3->m_ebo[0].m_offset)) / 2; // Convert the byte offset to unsigned short index
		draw_command.baseVertex = 0;
		draw_command.baseInstance = 0;
    }

    // Set the mesh render commands for instancing
    for (KCL::uint32 i = 0; i < scene->m_rooms.size(); i++)
    {
        for (KCL::uint32 j = 0; j < scene->m_rooms[i]->m_meshes.size(); j++)
        {
            KCL::Mesh *kcl_mesh = scene->m_rooms[i]->m_meshes[j];
            if (kcl_mesh->m_mio2)
            {
                bounding_volumes[kcl_mesh->m_indirect_draw_id].draw_data.z = float(kcl_mesh->m_mio2->m_indirect_draw_id);
            }
        }
    }

    // Set the index of the instance within the instance group
    for (KCL::uint32 i = 0; i < scene->m_mios2.size(); i++)
    {
        for (KCL::uint32 j = 0; j < scene->m_mios2[i]->m_instances.size(); j++)
        {
            bounding_volumes[m_scene->m_mios2[i]->m_instances[j]->m_indirect_draw_id].draw_data.w = float(j);
        }
    }

	m_matrix_data_buffer = [m_device newBufferWithBytes:mesh_matrices.data()
												 length:mesh_matrices.size() * sizeof(KCL::Matrix4x4)
												 options:STORAGE_MODE_MANAGED_OR_SHARED];

	m_mesh_data_buffer = [m_device newBufferWithBytes:bounding_volumes.data()
											   length:bounding_volumes.size() * sizeof(BoundingVolume)
											   options:STORAGE_MODE_MANAGED_OR_SHARED];

	m_actor_data_offset = m_static_mesh_count * sizeof(BoundingVolume);
	m_mesh_ids.resize(m_mesh_count);
	m_actors.resize(actor_mesh_count);

	KCL::uint32 draw_command_buffer_size = (m_mesh_count + mio_count) * sizeof(MTLDrawIndexedPrimitivesIndirectArguments);
	m_instanced_draw_commands_offset = m_mesh_count * sizeof(MTLDrawIndexedPrimitivesIndirectArguments);

	m_indirect_draw_buffer = [m_device newBufferWithLength:draw_command_buffer_size options:MTLResourceStorageModePrivate];

	{
		m_original_draw_buffer = [m_device newBufferWithLength:draw_command_buffer_size options:STORAGE_MODE_MANAGED_OR_SHARED];
		
		uint8_t* data = (uint8_t*)[m_original_draw_buffer contents];
		memcpy(data, draw_commands.data(),draw_commands.size() * sizeof(MTLDrawIndexedPrimitivesIndirectArguments) );
		memcpy(data + m_instanced_draw_commands_offset, m_instanced_indirect_draw_data.data(), m_instanced_indirect_draw_data.size() * sizeof(MTLDrawIndexedPrimitivesIndirectArguments));
		
#if !TARGET_OS_EMBEDDED
		if(m_original_draw_buffer.storageMode & MTLStorageModeManaged)
		{
			[m_original_draw_buffer didModifyRange:NSMakeRange(0, draw_command_buffer_size)];
		}
#endif		
	}
	
	{
		id <MTLCommandBuffer> commandBuffer = [scene->GetCommandQueue() commandBuffer];
		id <MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
		
		[blitEncoder copyFromBuffer:m_original_draw_buffer
					   sourceOffset:0
						   toBuffer:m_indirect_draw_buffer
				  destinationOffset:0
							   size:draw_command_buffer_size];
		[blitEncoder endEncoding];
		[commandBuffer commit];
	}

    // Init the instance matrixes with zeros
	KCL::uint32 instance_matrix_buffer_size = scene->m_mios2.size() * sizeof(KRL::Mesh3::InstanceData) * m_max_instance_count;
    std::vector<float> zero_floats(instance_matrix_buffer_size / sizeof(float), 0.0f);

	m_instanced_matrices_buffer = [m_device newBufferWithBytes:zero_floats.data() length:instance_matrix_buffer_size options:STORAGE_MODE_MANAGED_OR_SHARED];

    if (USE_MATRIX_STREAM)
    {
		m_instanced_matrices_stream_buffer = [m_device newBufferWithBytes:zero_floats.data() length:instance_matrix_buffer_size options:0];
    }

    m_stream_offsets.resize(m_scene->m_mios2.size());
}

KCL::KCL_Status OcclusionCull::Warmup(WarmUpHelper *warm_up, DynamicDataBufferPool* pool)
{
    NGLOG_INFO("Warm up occlusion cull...");
    MTL_Scene_40 * scene = warm_up->GetScene();

    // Animation time 31840ms is the closest to the average draw count
    scene->m_animation_time = 31840;
    scene->Animate();

    KCL::KCL_Status status = LoadShaders(128);
    if (status != KCL::KCL_TESTERROR_NOERROR)
    {
        return status;
    }

	id<MTLCommandBuffer> command_buffer = [scene->GetCommandQueue() commandBuffer];

    CreateHiZ(command_buffer, scene->m_active_camera, scene->m_occluders, m_quad_buffer);

	[command_buffer commit];
	[command_buffer waitUntilCompleted];

    double best_time = INT_MAX;
    KCL::uint32 best_wg_size = 0;
    KCL::uint32 sizes[] = { 8, 16, 32, 64, 128, 256, 512, 1024};
    for (KCL::uint32 i = 0; i < COUNT_OF(sizes); i++)
    {
		id<MTLCommandBuffer> command_buffer = [scene->GetCommandQueue() commandBuffer];

        KCL::uint32 wg_size = sizes[i];

        // Check if the workgroup size is supported
        if (!warm_up->GetValidator()->Validate(wg_size))
        {
            continue;
        }
        KCL::KCL_Status status = LoadShaders(wg_size);
        if (status != KCL::KCL_TESTERROR_NOERROR)
        {
            continue;
        }

		if (wg_size > m_occlusion_cull_shader->GetMaxThreadCount())
		{
			continue;
		}

        NGLOG_INFO("Workgroup size: %s", wg_size);

        // First try with 5 iterations
        KCL::uint32 iterations = 5;
        KCL::uint64 dt = 0;
        double avg_time = 0.0;
		m_dynamic_data_pool->InitFrame();
        warm_up->BeginTimer();
        for (KCL::uint32 j = 0; j < iterations; j++)
        {
            ExecuteOcclusionCull(command_buffer, scene->m_active_camera, scene->m_visible_meshes[0], scene->m_visible_instances);
        }
		[command_buffer commit];
		[command_buffer waitUntilCompleted];

        dt = warm_up->EndTimer();
		m_dynamic_data_pool->MarkSlotUnused(m_dynamic_data_pool->GetCurrentSlot());
        avg_time = double(dt) / double(iterations);

        NGLOG_INFO("  result after %s interations: sum: %sms, avg time: %sms", iterations, float(dt), float(avg_time));

        if (dt < 50)
        {
			id<MTLCommandBuffer> command_buffer = [scene->GetCommandQueue() commandBuffer];

            // Warm up until 200ms but maximalize the max iteration count
            iterations = avg_time > 0.01 ? KCL::uint32(200.0 / avg_time) : 200;
            iterations = KCL::Max(iterations, 5u);
            iterations = KCL::Min(iterations, 200u);

            NGLOG_INFO("  warmup %s iterations...", iterations);
			m_dynamic_data_pool->InitFrame();
            warm_up->BeginTimer();
            for (KCL::uint32 j = 0; j < iterations; j++)
            {
                ExecuteOcclusionCull(command_buffer, scene->m_active_camera, scene->m_visible_meshes[0], scene->m_visible_instances);
            }
			[command_buffer commit];
			[command_buffer waitUntilCompleted];
            dt = warm_up->EndTimer();
			m_dynamic_data_pool->MarkSlotUnused(m_dynamic_data_pool->GetCurrentSlot());

            avg_time = double(dt) / double(iterations);

            NGLOG_INFO("  result: sum: %sms, avg time: %sms", float(dt), float(avg_time));
        }

        if (avg_time < best_time)
        {
            best_time = avg_time;
            best_wg_size = wg_size;
        }
    }

    NGLOG_INFO("Best result: %s -> %sms (avg)", best_wg_size, float(best_time));
    if (best_wg_size == 0)
    {
        return KCL::KCL_TESTERROR_SHADER_ERROR;
    }

    WarmUpHelper::WarmUpConfig *cfg = new WarmUpHelper::WarmUpConfig();
    cfg->m_wg_config.size_x = best_wg_size;
    warm_up->SetConfig(WarmUpHelper::OCCLUSION_CULL, cfg);

    return LoadShaders(best_wg_size);
}
