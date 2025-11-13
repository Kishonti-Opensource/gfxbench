/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if 0
#ifndef BILATERAL_FRAGMENT_BLUR_H
#define BILATERAL_FRAGMENT_BLUR_H

#include "kcl_base.h"
#include "gfxb_fragment_blur.h"
#include <vector>

namespace GFXB
{

class Shapes;

//the depth tex size, should match the blurred tex size!!!!
class BilateralFragmentBlur : public FragmentBlur
{
public:

	void Init(Shapes *shapes, KCL::uint32 input_texture, KCL::uint32 depth_texture, KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, NGL_format texture_format, KCL::uint32 lod_levels, const char* name);
	KCL::uint32 RenderVerticalPass( KCL::Vector4D depth_parameters, KCL::uint32 lod_level = 0u );
	KCL::uint32 RenderHorizontalPass( KCL::Vector4D depth_parameters, KCL::uint32 lod_level = 0u );
	void SetKernelSize( KCL::uint32 blur_kernel_size );

protected:

	KCL::uint32 m_depth_texture;
};

}//!namespace GFXB

#endif

#endif