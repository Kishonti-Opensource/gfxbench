/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxbench_corporate.h"
#include "engine.h"
#include "battery_test.h"


CREATE_FACTORY(gl_manhattan, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_manhattan_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_manhattan31, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_manhattan31_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_manhattan31_battery, GFXBenchCorporateA<BatteryTestA<Engine2> >)

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

#endif
