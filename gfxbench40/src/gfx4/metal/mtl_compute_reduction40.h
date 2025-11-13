/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_COMPUTE_REDUCTION_H
#define GLB_COMPUTE_REDUCTION_H

#include <kcl_base.h>
#include <string>
#include <map>
#include "krl_scene.h"

#include "mtl_pipeline.h"
#include "metal/mtl_types_31.h"

namespace MetalRender
{

class ComputeReduction40
{
    static const KCL::uint32 DOWNSAMPLE = 4;

    static const KCL::uint32 PASS1_WORKGROUP_SIZE_X = 16;
    static const KCL::uint32 PASS1_WORKGROUP_SIZE_Y = 8;

    static const KCL::uint32 PASS1_DISPATCH_X = 8;
    static const KCL::uint32 PASS1_DISPATCH_Y = 8;

    static const KCL::uint32 PASS2_WORKGROUP_SIZE = 64;

public:
    static const KCL::uint32 ADAPTATION_ENABLED = 0; // Default: adaptation is enabled
    static const KCL::uint32 ADAPTATION_DISABLED = 1;  // Use the current frame's luminance value
    static const KCL::uint32 ADAPTATION_PREDEFINED = 2; // Use the predefines luminance values

    ComputeReduction40(id <MTLDevice> device);
    ~ComputeReduction40();
    
    KCL::KCL_Status Init(KCL::uint32 width, KCL::uint32 height, KRL_Scene *scene);
    void SetInputTexture(id <MTLTexture> texture);
    void Execute(id <MTLCommandBuffer> command_buffer, const UBOFrame &ubo_frame);
    void SetAdaptationMode(KCL::uint32 mode);
    
    id <MTLBuffer> GetLuminanceBuffer();

private:
    bool m_initialized;
    
    float m_step_u, m_step_v;
    int sample_per_thread_x, sample_per_thread_y;

    KRL_Scene *m_scene;

    KCL::uint32 m_sample_count;

    KCL::uint32 m_input_width;
    KCL::uint32 m_input_height;

    id <MTLTexture> m_input_texture;
    id <MTLSamplerState> m_input_sampler;
   
    Pipeline *m_pass1_shader;
    Pipeline *m_pass2_shader;

    id <MTLBuffer> m_pass1_output_buffer;
    id <MTLBuffer> m_pass2_output_buffer;

    KCL::int32 m_predefined_luminance_location;   

    KCL::uint32 m_adaptation_mode;

    std::map<KCL::uint32, float> m_luminance_map;
    
    void MapLuminanceValue(id <MTLCommandBuffer> command_buffer);
    void SaveLuminanceValues();
    void LoadLuminanceValues();
    static std::string GetLuminanceFilename(KRL_Scene *scene);

    void DebugBeginTimer();
    void DebugFinishTimer(id <MTLCommandBuffer> command_buffer);
    float DebugGetAvgLuminance(id <MTLTexture> texture, KCL::uint32 width, KCL::uint32 height);
    
    id <MTLDevice> m_device ;
};

}

#endif