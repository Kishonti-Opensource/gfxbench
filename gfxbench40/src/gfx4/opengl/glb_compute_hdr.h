/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_COMPUTE_HDR_H
#define GLB_COMPUTE_HDR_H

#include <kcl_base.h>

class GLB_Scene_ES2_;

namespace GFXB4
{
	class ComputeReduction;
}

namespace GLB
{
    class GLBShader2;
    class GLBFilter;
    class FragmentBlur;

    class BrightPass
    {
    public:
        BrightPass() {}
        virtual ~BrightPass() {}

        virtual void Init(KCL::uint32 width, KCL::uint32 height, KCL::uint32 bloom_texture_gl_type, KCL::uint32 quad_vao, KCL::uint32 quad_vbo) = 0;
        virtual void Execute(KCL::uint32 m_input_texture, KCL::uint32 m_input_sampler, KCL::uint32 m_output_texture) = 0;
    };

    class ComputeHDR
    {
    public:
        static const KCL::uint32 DOWNSAMPLE = 2;
        static const KCL::uint32 BLUR_TEXTURE_COUNT = 4;

        ComputeHDR();
        virtual ~ComputeHDR();

        void Init(KCL::uint32 width, KCL::uint32 height, KCL::uint32 bloom_texture_gl_type, bool use_compute_bright_pass, KCL::uint32 quad_vao, KCL::uint32 quad_vbo, GLB_Scene_ES2_ *scene);
        void SetInputTexture(KCL::uint32 in_texture);

        void Execute();

        KCL::uint32 GetBloomTexture() const;
        KCL::uint32 GetBloomSampler() const;
        KCL::uint32 GetLuminanceBuffer() const;
		GFXB4::ComputeReduction *GetComputeReduction() const;

    private:
        KCL::uint32 m_width, m_height;

        // textures
        KCL::uint32 m_input_texture;
        KCL::uint32 m_bloom_texture;

        // samplers
        KCL::uint32 m_input_sampler;
        KCL::uint32 m_bloom_sampler;

        GLB::BrightPass *m_bright_pass;
        GFXB4::ComputeReduction *m_reduction;
        GLB::FragmentBlur *m_fragment_blur;

        void CompileShader();
    };

    class ComputeBrightPass : public BrightPass
    {
    public:
        ComputeBrightPass();
        virtual ~ComputeBrightPass();
        virtual void Init(KCL::uint32 width, KCL::uint32 height, KCL::uint32 bloom_texture_gl_type, KCL::uint32 quad_vao, KCL::uint32 quad_vbo);
        virtual void Execute(KCL::uint32 input_texture, KCL::uint32 input_sampler, KCL::uint32 output_texture);

    private:
        KCL::uint32 m_bloom_texture_gl_type;

        // work group sizes
        KCL::uint32 m_work_group_size_x;
        KCL::uint32 m_work_group_size_y;
        KCL::uint32 m_dispatch_count_x;
        KCL::uint32 m_dispatch_count_y;

        // compute shader
        GLB::GLBShader2 *m_bright_pass;

        void SetWorkGroupSize();
    };

    class FragmentBrightPass : public BrightPass
    {
    public:
        FragmentBrightPass();
        virtual ~FragmentBrightPass();
        virtual void Init(KCL::uint32 width, KCL::uint32 height, KCL::uint32 bloom_texture_gl_type, KCL::uint32 quad_vao, KCL::uint32 quad_vbo);
        virtual void Execute(KCL::uint32 input_texture, KCL::uint32 input_sampler, KCL::uint32 output_texture);

    private:
        GLB::GLBFilter *m_filter;
        KCL::uint32 m_output_texture;
    };

}

#endif