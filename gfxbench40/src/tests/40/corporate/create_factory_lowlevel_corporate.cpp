/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxbench_corporate.h"

#if defined USE_ANY_GL

#include "opengl/gl_alu.h"
#include "opengl/gl_blending.h"
#include "opengl/gl_driver.h"
#include "opengl/gl_fill.h"

CREATE_FACTORY(gl_fill, GFXBenchCorporateA<CompressedFillTest>)
CREATE_FACTORY(gl_fill_off, GFXBenchCorporateA<CompressedFillTest>)
CREATE_FACTORY(gl_blending, GFXBenchCorporateA<UITest>)
CREATE_FACTORY(gl_blending_off, GFXBenchCorporateA<UITest>)
CREATE_FACTORY(gl_alu, GFXBenchCorporateA<ALUTest>)
CREATE_FACTORY(gl_alu_off, GFXBenchCorporateA<ALUTest>)
CREATE_FACTORY(gl_driver, GFXBenchCorporateA<CPUOverheadTest>)
CREATE_FACTORY(gl_driver_off, GFXBenchCorporateA<CPUOverheadTest>)

#endif


#ifdef USE_METAL

CREATE_FACTORY(metal_driver2, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_driver2_off, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_alu2, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_alu2_off, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_fill2, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_fill2_off, GFXBenchCorporateA<MetalWrapper>)

#endif
