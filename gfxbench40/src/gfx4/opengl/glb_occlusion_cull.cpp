/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_occlusion_cull.h"
#include "platform.h"

#include <kcl_camera2.h>
#include <kcl_mesh.h>
#include <kcl_scene_handler.h>

#include "glb_scene_opengl4.h"
#include "glb_scene_opengl4_support.h"
#include "opengl/fbo.h"
#include "opengl/glb_discard_functions.h"
#include "opengl/glb_opengl_state_manager.h"
#include "opengl/glb_shader2.h"

#include "glb_mesh.h"

#include <ng/log.h>

using namespace GLB;

OcclusionCull::OcclusionCull()
{
    m_is_draw_counter_enabled = false;

    m_width = 0;
    m_height = 0;
    m_mesh_count = 0;
    m_static_mesh_count = 0;
    m_actor_data_offset = 0;
    m_max_instance_count = 0;
    m_draw_count = 0;

    m_fbos = 0;
    m_depth_levels = 0;
    m_depth_texture = 0;
    m_depth_sampler = 0;
    m_quad_vao = 0;

    m_work_group_size = 0;
    m_occluder_shader = NULL;
    m_id_count_pos = -1;
    m_instance_id_offset_pos = -1;

    m_hi_z_shader = NULL;
    m_texture_size_pos = 0;

    m_stream_compact_shader = NULL;
    m_stream_offsets_pos = -1;
    m_frame_counter_pos1 = -1;
    m_frame_counter_pos2 = -1;

    m_frame_counter = 100.0f;

    m_mesh_id_buffer = 0;
    m_mesh_buffer = 0;
    m_indirect_draw_buffer = 0;
    m_draw_counter_buffer = 0;
    m_instanced_matrices_buffer = 0;
    m_instanced_matrices_stream_buffer = 0;

    m_instanced_draw_commands_offset = 0;

    m_dispatch_count = 0;

    m_debug_shader = NULL;
}

OcclusionCull::~OcclusionCull()
{
    if (m_fbos)
    {
        glDeleteFramebuffers(m_depth_levels, m_fbos);
        delete[] m_fbos;
    }
    glDeleteTextures(1, &m_depth_texture);
    glDeleteSamplers(1, &m_depth_sampler);

    glDeleteBuffers(1, &m_mesh_id_buffer);
    glDeleteBuffers(1, &m_mesh_buffer);
    glDeleteBuffers(1, &m_indirect_draw_buffer);
    glDeleteBuffers(1, &m_draw_counter_buffer);
    glDeleteBuffers(1, &m_instanced_matrices_buffer);
    glDeleteBuffers(1, &m_instanced_matrices_stream_buffer);
}

KCL::KCL_Status OcclusionCull::Init(WarmUpHelper *warm_up, GLB_Scene4 *scene, KCL::uint32 quad_vao, KCL::uint32 width, KCL::uint32 height, KCL::uint32 gl_depth_format, KCL::uint32 max_instance_count, bool cull_non_instanced_meshes, bool enable_draw_counters)
{
    m_scene = scene;

    m_width = KCL::Max(width, 1u);
    m_height = KCL::Max(height, 1u);
    m_quad_vao = quad_vao;
    m_max_instance_count = max_instance_count;
    m_is_draw_counter_enabled = enable_draw_counters;
    m_cull_non_instanced_meshes = cull_non_instanced_meshes;

    UpdateMeshData(scene);

    if (m_is_draw_counter_enabled)
    {
        m_draw_count = 0;
        glGenBuffers(1, &m_draw_counter_buffer);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_draw_counter_buffer);
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &m_draw_count, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    }

    m_depth_levels = KCL::texture_levels(m_width, m_height);
    glGenTextures(1, &m_depth_texture);
    glBindTexture(GL_TEXTURE_2D, m_depth_texture);
    glTexStorage2D(GL_TEXTURE_2D, m_depth_levels, gl_depth_format, m_width, m_height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenSamplers(1, &m_depth_sampler);
    glSamplerParameterf(m_depth_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameterf(m_depth_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameterf(m_depth_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameterf(m_depth_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    m_fbos = new KCL::uint32[m_depth_levels];
    glGenFramebuffers(m_depth_levels, m_fbos);
    for (KCL::uint32 i = 0; i < m_depth_levels; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, i);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, FBO::GetGlobalFBO()->getName());

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
            return Warmup(warm_up);
        }
    }
    return LoadShaders(ws_size);
}

KCL::KCL_Status OcclusionCull::LoadShaders(KCL::uint32 ws_size)
{
    m_work_group_size = ws_size;

    KCL::KCL_Status build_result;
    GLBShaderBuilder sb;
    m_occluder_shader = sb.ShaderFile("occluder.shader").Build(build_result);
    if (build_result != KCL::KCL_TESTERROR_NOERROR)
    {
        return build_result;
    }

    m_hi_z_shader = sb.ShaderFile("hi_z.shader").Build(build_result);
    if (build_result != KCL::KCL_TESTERROR_NOERROR)
    {
        return build_result;
    }

    sb.ShaderFile("occlusion_cull.shader");
    sb.AddDefineInt("WORK_GROUP_SIZE", m_work_group_size);
    sb.AddDefineInt("STATIC_MESH_COUNT", m_static_mesh_count);
    sb.AddDefineInt("MESH_COUNT", m_mesh_count);
    sb.AddDefineInt("USE_MATRIX_STREAM", USE_MATRIX_STREAM ? 1 : 0);

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
    m_stream_compact_shader = sb.Build(build_result);
    if (build_result != KCL::KCL_TESTERROR_NOERROR)
    {
        return build_result;
    }


    // Setup the shaders
    OpenGLStateManager::GlUseProgram(m_hi_z_shader->m_p);
    m_texture_size_pos = glGetUniformLocation(m_hi_z_shader->m_p, "texture_size");
    glUniform1i(m_hi_z_shader->m_uniform_locations[uniforms::texture_unit0], 0);

    OpenGLStateManager::GlUseProgram(m_occlusion_cull_shader->m_p);
    m_id_count_pos = glGetUniformLocation(m_occlusion_cull_shader->m_p, "id_count");
    m_instance_id_offset_pos = glGetUniformLocation(m_occlusion_cull_shader->m_p, "instance_id_offset");
    m_frame_counter_pos1 = glGetUniformLocation(m_occlusion_cull_shader->m_p, "frame_counter");
    glUniform2f(m_occlusion_cull_shader->m_uniform_locations[uniforms::view_port_size], m_width, m_height);
    glUniform1i(m_occlusion_cull_shader->m_uniform_locations[uniforms::hiz_texture], 0);

    OpenGLStateManager::GlUseProgram(m_stream_compact_shader->m_p);
    m_stream_offsets_pos = glGetUniformLocation(m_stream_compact_shader->m_p, "stream_offsets");
    m_frame_counter_pos2 = glGetUniformLocation(m_stream_compact_shader->m_p, "frame_counter");

    OpenGLStateManager::GlUseProgram(0);

    return KCL::KCL_TESTERROR_NOERROR;
}

void OcclusionCull::CreateHiZ(const KCL::Camera2 *camera, const std::vector<KCL::Mesh*> &occluders)
{
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // Render the occluder meshes
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[0]);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, 0);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_DEPTH_BUFFER_BIT);
    OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
    const KCL::uint32 mesh_count = occluders.size();
    if (mesh_count)
    {
        OpenGLStateManager::GlDisable(GL_CULL_FACE);

        OpenGLStateManager::GlUseProgram(m_occluder_shader->m_p);
        OpenGLStateManager::Commit();
        KCL::Matrix4x4 mvp;
        KCL::Mesh *mesh;
        for (KCL::uint32 i = 0; i < mesh_count; i++)
        {
            mesh = occluders[i];

            mvp = mesh->m_world_pom * camera->GetViewProjection();
            glUniformMatrix4fv(m_occluder_shader->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, mvp.v);

            GFXB4::Mesh3* glb_mesh3 = (GFXB4::Mesh3*)mesh->m_mesh;
            glBindVertexArray(glb_mesh3->m_vao_4);
            glDrawElements(GL_TRIANGLES, KCL::uint32(glb_mesh3->getIndexCount(0)), GL_UNSIGNED_SHORT, glb_mesh3->m_ebo[0].m_offset);
        }

        glBindVertexArray(0);
    }

    // Generate the Hierarchical-Z map
    KCL::uint32 width = m_width;
    KCL::uint32 height = m_height;
    OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_depth_texture);
    glBindSampler(0, m_depth_sampler);
    OpenGLStateManager::GlDepthFunc(GL_ALWAYS);
    OpenGLStateManager::GlUseProgram(m_hi_z_shader->m_p);
    OpenGLStateManager::Commit();

    glBindVertexArray(m_quad_vao);
    static const GLenum invalid_attachments[1] = { GL_DEPTH_ATTACHMENT };
    for (KCL::uint32 i = 1; i < m_depth_levels; i++)
    {
        // Set the size of the source texture
        glUniform2i(m_texture_size_pos, width, height);

        // Calculate the render target dimenensions
        width = KCL::Max(width / 2, 1u);
        height = KCL::Max(height / 2, 1u);

        glViewport(0, 0, width, height);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, i - 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i - 1);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[i]);
        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, i);

        glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &invalid_attachments[0]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // Cleanup
    glBindVertexArray(0);
    OpenGLStateManager::GlDepthFunc(GL_LESS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_depth_levels - 1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindSampler(0, 0);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void OcclusionCull::ExecuteOcclusionCull(const KCL::Camera2 *camera, const std::vector<KCL::Mesh*> &meshes, const std::vector< std::vector<KCL::Mesh*> > &visible_instances)
{
    KCL::uint32 id_count = 0;
    KCL::uint32 actor_mesh_count = 0;

    // Collect the meshes for culling
    KCL::Mesh *kcl_mesh = NULL;
    GLB::Mesh3 *glb_mesh3 = NULL;
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
                glb_mesh3 = (GLB::Mesh3*)kcl_mesh->m_mesh;
                kcl_mesh->m_owner_actor->m_aabb.CalculateVertices4D(m_actors[actor_mesh_count].aabb);
                m_actors[actor_mesh_count].draw_data.x = float(glb_mesh3->getIndexCount(0));
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
            m_stream_offsets[visible_mios++] = ((GetInstanceBufferOffset() + visible_instances[i][0]->m_mio2->m_indirect_draw_id ) * 256);
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
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_mesh_id_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_mesh_count * sizeof(KCL::uint32), NULL, GL_DYNAMIC_COPY); //TODO: usage
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, id_count * sizeof(KCL::uint32), m_mesh_ids.data());

    // Update the dynamic mesh buffer (actors)
    if (actor_mesh_count)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_mesh_buffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, m_actor_data_offset, actor_mesh_count * sizeof(BoundingVolume), m_actors.data());
    }

    // Reset the draw commands for instanced meshes
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_indirect_draw_buffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER,
        m_instanced_draw_commands_offset,
        m_instanced_indirect_draw_data.size() * sizeof(DrawElementsIndirectCommand),
        m_instanced_indirect_draw_data.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Bind the buffers
    if (m_is_draw_counter_enabled)
    {
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_draw_counter_buffer);
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_mesh_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_mesh_id_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_indirect_draw_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, USE_MATRIX_STREAM ? m_instanced_matrices_stream_buffer : m_instanced_matrices_buffer);

    // Bind the hiZ depth texture
    OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_depth_texture);
    glBindSampler(0, m_depth_sampler);

    OpenGLStateManager::GlUseProgram(m_occlusion_cull_shader->m_p);
    OpenGLStateManager::Commit();

    // Setup the shader
    glUniformMatrix4fv(m_occlusion_cull_shader->m_uniform_locations[uniforms::vp], 1, GL_FALSE, camera->GetViewProjection().v);
    glUniform1f(m_occlusion_cull_shader->m_uniform_locations[uniforms::near_far_ratio], -camera->GetProjection().v33);
    glUniform1ui(m_id_count_pos, id_count);
    glUniform1ui(m_instance_id_offset_pos, instance_id_offset);
    if (m_frame_counter_pos1 > -1)
    {
        glUniform1f(m_frame_counter_pos1, m_frame_counter);
    }

    // Execute the occlusion cull
    m_dispatch_count = (id_count + m_work_group_size - 1) / m_work_group_size;
    glDispatchComputeProc(m_dispatch_count, 1, 1);

    if (USE_MATRIX_STREAM)
    {
        glMemoryBarrierProc(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    }
    else
    {
        glMemoryBarrierProc(GL_COMMAND_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    }

    // Clean up
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindSampler(0, 0);

    if (m_is_draw_counter_enabled)
    {
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0);
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);

    /////////////////////////
    if (m_is_draw_counter_enabled)
    {
        glMemoryBarrierProc(GL_ATOMIC_COUNTER_BARRIER_BIT);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_draw_counter_buffer);
        KCL::uint32 *ptr = (KCL::uint32*) glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(KCL::uint32), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        if (ptr)
        {
            m_draw_count = *ptr;
            *ptr = 0;
            glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
        }
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    }
    /////////////////////////

    if (USE_MATRIX_STREAM && visible_mios)
    {
        GLB::OpenGLStateManager::GlUseProgram(m_stream_compact_shader->m_p);

        glUniform1iv(m_stream_offsets_pos, m_stream_offsets.size(), m_stream_offsets.data());
        glUniform1f(m_frame_counter_pos2, m_frame_counter);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_instanced_matrices_stream_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_instanced_matrices_buffer);

        glDispatchComputeProc(visible_mios, 1, 1);

        glMemoryBarrierProc(GL_SHADER_STORAGE_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
    }
}

void OcclusionCull::UpdateMeshData(const KCL::SceneHandler *scene)
{
    std::vector<KCL::Matrix4x4> mesh_matrices;
    std::vector<BoundingVolume> bounding_volumes;
    std::vector<DrawElementsIndirectCommand> draw_commands;

    m_static_mesh_count = 0;
    KCL::uint32 id_counter = 0;

    for (KCL::uint32 i = 0; i < scene->m_rooms.size(); i++)
    {
        //m_static_mesh_count += scene->m_rooms[i]->m_meshes.size();
        for (KCL::uint32 j = 0; j < scene->m_rooms[i]->m_meshes.size(); j++)
        {
            KCL::Mesh *kcl_mesh = scene->m_rooms[i]->m_meshes[j];
            GLB::Mesh3 *glb_mesh3 = (GLB::Mesh3*)kcl_mesh->m_mesh;

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
            DrawElementsIndirectCommand draw_command;
            draw_command.count = glb_mesh3->getIndexCount(0);
            draw_command.instanceCount = 1;
            draw_command.firstIndex = *((KCL::uint32*)(&glb_mesh3->m_ebo[0].m_offset)) / 2; // Convert the byte offset to unsigned short index

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
                GLB::Mesh3 *glb_mesh3 = (GLB::Mesh3*)kcl_mesh->m_mesh;

                DrawElementsIndirectCommand draw_command;
                draw_command.count = glb_mesh3->getIndexCount(0);
                draw_command.instanceCount = 1;
                draw_command.firstIndex = *((KCL::uint32*)(&glb_mesh3->m_ebo[0].m_offset)) / 2; // Convert the byte offset to unsigned short index

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
        DrawElementsIndirectCommand &draw_command = m_instanced_indirect_draw_data[i];
        draw_command.count = mesh3->getIndexCount(0);
        draw_command.instanceCount = 0;
        draw_command.firstIndex = *((KCL::uint32*)(&mesh3->m_ebo[0].m_offset)) / 2; // Convert the byte offset to unsigned short index
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

    {
        KCL::uint32 mesh_buffer_size = mesh_matrices.size() * sizeof(KCL::Matrix4x4) + m_mesh_count * sizeof(BoundingVolume);
        KCL::uint32 static_mesh_data_offset = mesh_matrices.size() * sizeof(KCL::Matrix4x4);

        if (!m_mesh_buffer)
        {
            glGenBuffers(1, &m_mesh_buffer);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_mesh_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, mesh_buffer_size, NULL, GL_STATIC_DRAW);

        if (mesh_matrices.size())
        {
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, static_mesh_data_offset, mesh_matrices.data());
        }
        if (bounding_volumes.size())
        {
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, static_mesh_data_offset, bounding_volumes.size() * sizeof(BoundingVolume), bounding_volumes.data());
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        m_actor_data_offset = static_mesh_data_offset + m_static_mesh_count * sizeof(BoundingVolume);
        m_mesh_ids.resize(m_mesh_count);
        m_actors.resize(actor_mesh_count);
    }

    {
        KCL::uint32 draw_command_buffer_size = (m_mesh_count + mio_count) * sizeof(DrawElementsIndirectCommand);
        m_instanced_draw_commands_offset = m_mesh_count * sizeof(DrawElementsIndirectCommand);

        if (!m_indirect_draw_buffer)
        {
            glGenBuffers(1, &m_indirect_draw_buffer);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_indirect_draw_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, draw_command_buffer_size, NULL, GL_DYNAMIC_COPY); // TODO: usage
        if (draw_commands.size())
        {
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, draw_commands.size() * sizeof(DrawElementsIndirectCommand), draw_commands.data());
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    {
        if (!m_mesh_id_buffer)
        {
            glGenBuffers(1, &m_mesh_id_buffer);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_mesh_id_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, m_mesh_count * sizeof(KCL::uint32), NULL, GL_DYNAMIC_COPY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    // Init the instance matrixes with zeros
    KCL::uint32 instance_matrix_buffer_size = scene->m_mios2.size() * sizeof(Mesh3::InstanceData) * m_max_instance_count;
    std::vector<float> zero_floats(instance_matrix_buffer_size / sizeof(float), 0.0f);

    {
        if (!m_instanced_matrices_buffer)
        {
            glGenBuffers(1, &m_instanced_matrices_buffer);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_instanced_matrices_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, instance_matrix_buffer_size, zero_floats.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    if (USE_MATRIX_STREAM)
    {
        if (!m_instanced_matrices_stream_buffer)
        {
            glGenBuffers(1, &m_instanced_matrices_stream_buffer);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_instanced_matrices_stream_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, instance_matrix_buffer_size, zero_floats.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    m_stream_offsets.resize(m_scene->m_mios2.size());
}

KCL::KCL_Status OcclusionCull::Warmup(WarmUpHelper *warm_up)
{
    NGLOG_INFO("Warm up occlusion cull...");
    GLB_Scene4 *scene = warm_up->GetScene();

    // Animation time 31840ms is the closest to the average draw count
    scene->m_animation_time = 31840;
    scene->Animate();

    KCL::KCL_Status status = LoadShaders(64);
    if (status != KCL::KCL_TESTERROR_NOERROR)
    {
        return status;
    }

    CreateHiZ(scene->m_active_camera, scene->m_occluders);

    double best_time = INT_MAX;
    KCL::uint32 best_wg_size = 0;
    KCL::uint32 sizes[] = { 8, 16, 32, 64, 128, 256, 512, 1024};
    for (KCL::uint32 i = 0; i < COUNT_OF(sizes); i++)
    {
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

        NGLOG_INFO("Workgroup size: %s", wg_size);

        // First try with 5 iterations
        KCL::uint32 iterations = 5;
        KCL::uint64 dt = 0;
        double avg_time = 0.0;
        warm_up->BeginTimer();
        for (KCL::uint32 j = 0; j < iterations; j++)
        {
            ExecuteOcclusionCull(scene->m_active_camera, scene->m_visible_meshes[0], scene->m_visible_instances);
        }
        dt = warm_up->EndTimer();
        avg_time = double(dt) / double(iterations);

        NGLOG_INFO("  result after %s interations: sum: %sms, avg time: %sms", iterations, float(dt), float(avg_time));

        if (dt < 50)
        {
            // Warm up until 200ms but maximalize the max iteration count
            iterations = avg_time > 0.01 ? KCL::uint32(200.0 / avg_time) : 200;
            iterations = KCL::Max(iterations, 5u);
            iterations = KCL::Min(iterations, 200u);

            NGLOG_INFO("  warmup %s iterations...", iterations);
            warm_up->BeginTimer();
            for (KCL::uint32 j = 0; j < iterations; j++)
            {
                ExecuteOcclusionCull(scene->m_active_camera, scene->m_visible_meshes[0], scene->m_visible_instances);
            }
            dt = warm_up->EndTimer();
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


#if 0
void OcclusionCull::DebugHiZMap(KCL::Camera2 *camera)
{
    if (!m_debug_shader)
    {
        KCL::KCL_Status status;
        GLBShaderBuilder sb;
        m_debug_shader = sb.ShaderFile("hi_z_debug.shader").Build(status);
    }

    glBindVertexArray(m_quad_vao);

    OpenGLStateManager::GlUseProgram(m_debug_shader->m_p);
    GLint layer_pos = glGetUniformLocation(m_debug_shader->m_p, "layer");
    OpenGLStateManager::GlDisable(GL_BLEND);
    OpenGLStateManager::GlDisable(GL_DEPTH_TEST);

    /*
    GLint debug_coords_pos = glGetUniformLocation(m_debug_shader->m_p, "debug1");
    glUniform4fv(debug_coords_pos, 1, debug_ssbo.debug1.v);

    debug_coords_pos = glGetUniformLocation(m_debug_shader->m_p, "debug2");
    glUniform4fv(debug_coords_pos, 1, debug_ssbo.debug2.v);
    */

    OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
    glUniform1i(m_debug_shader->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
    glBindTexture(GL_TEXTURE_2D, m_depth_texture);
    glBindSampler(0, 0);

    glUniform4fv(m_debug_shader->m_uniform_locations[GLB::uniforms::depth_parameters], 1, camera->m_depth_linearize_factors.v);

    OpenGLStateManager::Commit();
    for (int i = 0; i < m_depth_levels; i++)
    {
        glViewport(108 * i, 8, 100, 50);
        glUniform1f(layer_pos, i);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindSampler(0, 0);
    glBindVertexArray(0);
    OpenGLStateManager::GlUseProgram(0);
}
#endif