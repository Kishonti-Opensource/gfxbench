/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxbench_corporate.h"
#include "engine.h"

#if USE_ANY_GL
#include "low_level_tests/opengl/gl_tess.h"
#include "low_level_tests/opengl/gl_precision.h"
//#include "low_level_tests/gl_vol.h"

CREATE_FACTORY(gl_4, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_4_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_tess, GFXBenchCorporateA<TessTest>)
CREATE_FACTORY(gl_tess_off, GFXBenchCorporateA<TessTest>)
CREATE_FACTORY(gl_precision, GFXBenchCorporateA<PrecisionTest>)
//CREATE_FACTORY(gl_vol, GFXBenchCorporateA<VolTest>)
#endif


#if USE_METAL

CREATE_FACTORY(metal_4, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_4_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_tess, GFXBenchCorporateA<MetalWrapper>)
CREATE_FACTORY(metal_tess_off, GFXBenchCorporateA<MetalWrapper>)

CREATE_FACTORY(metal_manhattan, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_manhattan_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_manhattan31, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_manhattan31_off, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_trex, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(metal_trex_off, GFXBenchCorporateA<Engine2>)

#include "qualitymatch.h"
#include "battery_test.h"
CREATE_FACTORY(metal_trex_battery, GFXBenchCorporateA<BatteryTestA<Engine2> >)
CREATE_FACTORY(metal_manhattan31_battery, GFXBenchCorporateA<BatteryTestA<Engine2> >)

#endif

