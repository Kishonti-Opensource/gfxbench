/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef BLOOM_UPSAMPLE_BLUR_H
#define BLOOM_UPSAMPLE_BLUR_H

#include "kcl_base.h"
#include <vector>
#include "gfxb_bloom_downsample_blur.h"

namespace GFXB
{

class Shapes;

class BloomUpsampleBlur : public BloomDownsampleBlur
{
public:
	void Init( Shapes *shapes, KCL::uint32 input_texture0, KCL::uint32 input_texture1, KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, NGL_format texture_format, KCL::uint32 lod_levels, const char* name );
	KCL::uint32 RenderBlurPass( KCL::uint32 command_buffer, KCL::uint32 lod_level );

protected:
	void SetKernelSize(KCL::uint32 blur_kernel_size);
	KCL::uint32 m_input_texture0, m_input_texture1;
};

}//!namespace GFXB

#endif
