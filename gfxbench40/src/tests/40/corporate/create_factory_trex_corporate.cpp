/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxbench_corporate.h"
#include "engine.h"

CREATE_FACTORY(gl_trex, GFXBenchCorporateA<Engine2>)
CREATE_FACTORY(gl_trex_off, GFXBenchCorporateA<Engine2>)

#include "qualitymatch.h"
#include "battery_test.h"
CREATE_FACTORY(gl_trex_battery, GFXBenchCorporateA<BatteryTestA<Engine2> >)
CREATE_FACTORY(gl_trex_qmatch, GFXBenchCorporateA<QualityMatchA<Engine2> >)
CREATE_FACTORY(gl_trex_qmatch_highp, GFXBenchCorporateA<QualityMatchA<Engine2> >)
