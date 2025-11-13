/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __METAL_LOW_LEVEL_H__
#define __METAL_LOW_LEVEL_H__


#include "alu_base.h"
#include "driver_base.h"
#include "blending_base.h"
#include "fill_base.h"

#include "kcl_base.h"
#include "kcl_math3d.h"

#include "../gfxbench/global_test_environment.h"



ALUTest_Base*         CreateALUTestMetal(const GlobalTestEnvironment* const gte) ;
CPUOverheadTest_Base* CreateCPUOverheadTestMetal(const GlobalTestEnvironment* const gte) ;
UITest_Base*          CreateUITestMetal(const GlobalTestEnvironment* const gte) ;
CompressedFillTest_Base* CreateCompressedFillTestMetal(const GlobalTestEnvironment* const gte) ;



#endif  // __METAL_LOW_LEVEL_H__ 