/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxbench_corporate.h"
#include "engine.h"

#if defined HAVE_DX
#include "d3d11/dx_alu.h"
#include "d3d11/dx_blending.h"
#include "d3d11/dx_driver.h"
#include "d3d11/dx_fill.h"
#elif defined USE_ANY_GL
#include "opengl/gl_alu.h"
#include "opengl/gl_blending.h"
#include "opengl/gl_driver.h"
#include "opengl/gl_fill.h"
#endif

#include "qualitymatch.h"
#include "battery_test.h"

#if defined USE_METAL
CREATE_FACTORY(metal_manhattan, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_manhattan_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_trex, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_trex_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_alu, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_alu_off, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_driver, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_driver_off, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_blending, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_blending_off, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_fill, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_fill_off, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_trex_battery, GFXBenchCorporateA<BatteryTestA<Engine2> >)
CREATE_FACTORY(metal_trex_qmatch, GFXBenchCorporateA<QualityMatchA<Engine2> >)
CREATE_FACTORY(metal_trex_qmatch_highp, GFXBenchCorporateA<QualityMatchA<Engine2> >)
#elif defined HAVE_DX
CREATE_FACTORY(dx_manhattan, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(dx_manhattan_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(dx_trex, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(dx_trex_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(dx_egypt, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(dx_egypt_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(dx_fill, GFXBenchCorporateA<CompressedFillTest>)
CREATE_FACTORY(dx_fill_off, GFXBenchCorporateA<CompressedFillTest>)
CREATE_FACTORY(dx_blending, GFXBenchCorporateA<UITest>)
CREATE_FACTORY(dx_blending_off, GFXBenchCorporateA<UITest>)
CREATE_FACTORY(dx_alu, GFXBenchCorporateA<ALUTest>)
CREATE_FACTORY(dx_alu_off, GFXBenchCorporateA<ALUTest>)
CREATE_FACTORY(dx_driver, GFXBenchCorporateA<CPUOverheadTest>)
CREATE_FACTORY(dx_driver_off, GFXBenchCorporateA<CPUOverheadTest>)
CREATE_FACTORY(dx_trex_battery, GFXBenchCorporateA<BatteryTestA<Engine2> >)
CREATE_FACTORY(dx_trex_qmatch, GFXBenchCorporateA<QualityMatchA<Engine2> >)
CREATE_FACTORY(dx_trex_qmatch_highp, GFXBenchCorporateA<QualityMatchA<Engine2> >)
#elif defined USE_ANY_GL
CREATE_FACTORY(gl_manhattan, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_manhattan_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_trex, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_trex_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_egypt, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_egypt_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_fill, GFXBenchCorporateA<CompressedFillTest>)
CREATE_FACTORY(gl_fill_off, GFXBenchCorporateA<CompressedFillTest>)
CREATE_FACTORY(gl_blending, GFXBenchCorporateA<UITest>)
CREATE_FACTORY(gl_blending_off, GFXBenchCorporateA<UITest>)
CREATE_FACTORY(gl_alu, GFXBenchCorporateA<ALUTest>)
CREATE_FACTORY(gl_alu_off, GFXBenchCorporateA<ALUTest>)
CREATE_FACTORY(gl_driver, GFXBenchCorporateA<CPUOverheadTest>)
CREATE_FACTORY(gl_driver_off, GFXBenchCorporateA<CPUOverheadTest>)
CREATE_FACTORY(gl_trex_battery, GFXBenchCorporateA<BatteryTestA<Engine2> >)
CREATE_FACTORY(gl_trex_qmatch, GFXBenchCorporateA<QualityMatchA<Engine2> >)
CREATE_FACTORY(gl_trex_qmatch_highp, GFXBenchCorporateA<QualityMatchA<Engine2> >)
#endif

