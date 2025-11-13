/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxbench_corporate.h"
#include "engine.h"
#include "battery_test.h"

#if !defined USE_METAL
#include "qualitymatch.h"
#if defined USE_ANY_GL
#elif defined HAVE_DX
#include "d3d11/gl_alu.h"
#include "d3d11/gl_blending.h"
#include "d3d11/gl_driver.h"
#include "d3d11/gl_fill.h"
#endif

CREATE_FACTORY(gl_manhattan, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_manhattan_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_manhattan31, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_manhattan31_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_manhattan31_battery, GFXBenchCorporateA<BatteryTestA<Engine2> >)
CREATE_FACTORY(gl_trex, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_trex_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_egypt, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_egypt_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_trex_battery, GFXBenchCorporateA<BatteryTestA<Engine2> >)
CREATE_FACTORY(gl_trex_qmatch, GFXBenchCorporateA<QualityMatchA<Engine2> >)
CREATE_FACTORY(gl_trex_qmatch_highp, GFXBenchCorporateA<QualityMatchA<Engine2> >)


#if defined USE_ANY_GL
#include "opengl/gl_alu2.h"
#include "opengl/gl_driver2.h"
#include "opengl/gl_fill2.h"

CREATE_FACTORY(gl_alu2, GFXBenchCorporateA<ALUTest2>)
CREATE_FACTORY(gl_alu2_off, GFXBenchCorporateA<ALUTest2>)
CREATE_FACTORY(gl_driver2, GFXBenchCorporateA<DriverOverheadTest2>)
CREATE_FACTORY(gl_driver2_off, GFXBenchCorporateA<DriverOverheadTest2>)
CREATE_FACTORY(gl_fill2, GFXBenchCorporateA<CompressedFillTest2>)
CREATE_FACTORY(gl_fill2_off, GFXBenchCorporateA<CompressedFillTest2>)

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

//#include "opengl/gui_test.h"
//CREATE_FACTORY(gl_gui_test, GFXBenchCorporateA<GUIB::GUI_Test>)
#endif

#else

CREATE_FACTORY(metal_manhattan31, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_manhattan31_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_manhattan31_battery, GFXBenchCorporateA<BatteryTestA<Engine2> >)
CREATE_FACTORY(metal_manhattan, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_manhattan_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_trex, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_trex_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_trex_battery, GFXBenchCorporateA<BatteryTestA<Engine2> >)

CREATE_FACTORY(metal_driver2, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_driver2_off, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_alu2, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_alu2_off, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_fill2, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_fill2_off, GFXBenchCorporateA<MetalWrapper>)

#endif
