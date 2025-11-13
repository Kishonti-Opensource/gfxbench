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

class GLB_Scene_ES2_;

namespace GLB
{
	class GLBShader2;
}

namespace GFXB4
{

class ComputeReduction
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

    ComputeReduction();
    ~ComputeReduction();
    
    KCL::KCL_Status Init(KCL::uint32 width, KCL::uint32 height, GLB_Scene_ES2_ *scene);
    void SetInputTexture(KCL::uint32 texture);
    void Execute();
    void SetAdaptationMode(KCL::uint32 mode);
    KCL::uint32 GetAdaptationMode() const;
    
    KCL::uint32 GetLuminanceBuffer();

private:
    bool m_initialized;

    GLB_Scene_ES2_ *m_scene;

    KCL::uint32 m_sample_count;

    KCL::uint32 m_input_width;
    KCL::uint32 m_input_height;

    KCL::uint32 m_input_texture;
    KCL::uint32 m_input_sampler;
   
    GLB::GLBShader2 *m_pass1_shader;
    GLB::GLBShader2 *m_pass2_shader[3]; // Separated shaders by adaptation mode

    KCL::uint32 m_pass1_output_buffer;
    KCL::uint32 m_pass2_output_buffer;

    KCL::int32 m_predefined_luminance_location;   

    KCL::uint32 m_adaptation_mode;

    std::map<KCL::uint32, float> m_luminance_map;
    
    KCL::KCL_Status InitAdaptationShader(KCL::uint32 mode);
    void MapLuminanceValue();
    void SaveLuminanceValues();
    void LoadLuminanceValues();
    static std::string GetLuminanceFilename(GLB_Scene_ES2_ *scene);

    void DebugBeginTimer();
    void DebugFinishTimer();
    float DebugGetAvgLuminance(KCL::uint32 texture, KCL::uint32 width, KCL::uint32 height);
};

}

#endif